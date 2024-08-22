/*
 * LRSTUtil.c
 *
 *  Created on: Jun 9, 2016
 *      Author: sidlausk
 */

#include "LRSTUtil.h"

int comp_by_score( const void * el1, const void * el2 ) {
  const DPoint *const dp1 = (DPoint*)el1;
  const DPoint *const dp2 = (DPoint*)el2;
  assert( dp1->dims == dp2->dims );
  assert( dp1->score >= 0 );
  assert( dp2->score >= 0 );

  if ( dp1->score < dp2->score )
    return  1;
  if ( dp1->score > dp2->score )
    return -1;
  return 0;
}

void initEmptyDPoints( DPoint *const dpoints, const Rint n, const Rint dims) {

  DPoint empty = { dims, 0, DP_ALLOCED, -1.0, NULL, NULL };
  Rint i;
  for (i = 0; i < n; ++i) {
    dpoints[i] = empty;
    dpoints[i].ccoord = malloc( sizeof(typcoord) * dims );
    dpoints[i].dpoint = malloc( sizeof(typcoord) * dims );
  }
}

void initDPoint( DPoint *const dpoint, const Rint c,
                   const typcoord *const ccoord,
                   const typinterval *const cand_mbr, const Rint dims) {

  dpoint->dims = dims;
  dpoint->corner = c;
  memcpy( dpoint->ccoord, ccoord, sizeof(typcoord) * dims );
  computeCorner(c, cand_mbr, dpoint->dpoint, dims);
  dpoint->flag = DP_VALID;
}

void initDPoint2( DPoint *const dpoint, const Rint c,
                    const typcoord *const ccoord,
                    const typcoord *const cand_coord, const Rint dims) {

  dpoint->dims = dims;
  dpoint->corner = c;
  dpoint->score = -1.0;
  memcpy( dpoint->ccoord, ccoord, sizeof(typcoord) * dims );
  memcpy( dpoint->dpoint, cand_coord, sizeof(typcoord) * dims );
  dpoint->flag = DP_VALID;
}

void freeDPoints( DPoint *const dpoints, const Rint n ) {
  Rint i;
  for (i = 0; i < n; ++i)
    freeDPoint( &dpoints[i] );
}

void freeDPoint( DPoint *const dpoint ) {
  assert( dpoint->flag == DP_ALLOCED || dpoint->flag == DP_VALID );
  assert( dpoint->ccoord != NULL );
  assert( dpoint->dpoint != NULL );
  free( dpoint->ccoord );
  free( dpoint->dpoint );
  dpoint->flag = DP_NULL;
}

void memcpyDPs( DPoint *const dst, DPoint *const src, const Rint n ) {
  Rint i;

  for (i = 0; i < n; ++i) {
    assert( src[i].flag != DP_NULL );
    assert( dst[i].flag != DP_NULL );

    dst[i].dims = src[i].dims;
    dst[i].corner = src[i].corner;
    dst[i].score = src[i].score;
    memcpy( dst[i].ccoord, src[i].ccoord, SIZEcoord*src[i].dims );
    memcpy( dst[i].dpoint, src[i].dpoint, SIZEcoord*src[i].dims );
  }
}

void swapDPs(  DPoint *i, DPoint *j ) {
	DPoint temp = { 0, 0, DP_ALLOCED, -1.0, NULL, NULL };

	memcpy( &temp, i, sizeof(DPoint) );
	memcpy( i, j, sizeof(DPoint) );
	memcpy( j, &temp, sizeof(DPoint) );
}

Rfloat computeOverlap( const typinterval *const mbr1,
    const typinterval *const mbr2, typinterval *const buf, const Rint dims ) {

  Rint d;

  for (d = 0; d < dims; ++d) {
    assert( mbr1[d].l < mbr1[d].h );
    assert( mbr2[d].l < mbr2[d].h );

    if ( mbr1[d].h > mbr2[d].l || mbr2[d].h > mbr1[d].l) {
      buf[d].l = fmax(mbr1[d].l, mbr2[d].l);
      buf[d].h = fmin(mbr1[d].h, mbr2[d].h);
    } else {
      assert( 0 ); // as never called in non-overlapping cases (so far)
    }
  }
  return volume(buf, dims);
}

Rint computeScores( DPoint *const dpoints, const Rint num,
		const Rfloat kNodeVol ) {

  DPoint *const theone = dpoints  + 0;
  theone->score = volumePs( theone->ccoord, theone->dpoint, theone->dims );
//  printDPoints( dpoints, num, "ALL\n");

  typinterval deadliest_mbr[theone->dims];
  makeMBRfromDP( theone, deadliest_mbr );

  typinterval overlap_mbr[theone->dims], cand_mbr[theone->dims];
  Rint next = 1, tail = num - 1;
  while (next <= tail) {
//    printDPoint( &dpoints[next], "CANDIDATE: ");
    makeMBRfromDP( &dpoints[next], cand_mbr );
    Rfloat overlap = computeOverlap( deadliest_mbr, cand_mbr, overlap_mbr, theone->dims );
    assert( overlap > 0 );
    dpoints[next].score -= overlap;
    assert( dpoints[next].score >= 0 );

    // Remove this candidate if it became < DP_MIN_DSPACE
    if ( dpoints[next].score / kNodeVol < DP_MIN_DSPACE) {
    	if (next != tail)
    		swapDPs( &dpoints[next], &dpoints[tail] );
    	--tail;
    } else ++next;

  }

  return tail + 1;
}

Rint eliminateZeroVolMBRs( DPoint *const dpoints, const Rint num,
    const Rfloat node_vol ) {

  if (num < 3)
    return 0;

  Rint tail = num;
  Rint i = 0, i_deadliest = 0;
  while ( i < tail ) {
    dpoints[i].score = volumePs( dpoints[i].ccoord, dpoints[i].dpoint, dpoints[i].dims );

    if ( dpoints[i].score / node_vol < DP_MIN_DSPACE ) {
      if ( i != --tail ) {
        memcpyDPs( &dpoints[i], &dpoints[tail], 1 );
      }
    } else {
      if (dpoints[i_deadliest].score < dpoints[i].score)
        i_deadliest = i;
      ++i;
    }
  }

  // Calculate scores based on the deadliest candidate:
  if ( i_deadliest != 0 )
	  swapDPs( dpoints + 0, dpoints + i_deadliest );
  tail = computeScores( dpoints, tail, node_vol );

  return tail;
}

void printDPoint(const DPoint *const dp, const char *const prefix) {
  Rint d;
  printf("%s(%d <", prefix, dp->corner);

  for (d = 0; d < dp->dims; ++d) {
    printf( "%.2f", dp->ccoord[d] );
    if (d+1 < dp->dims)
      printf(",");
  }
  printf("> <");
  for (d = 0; d < dp->dims; ++d) {
    printf( "%.2f", dp->dpoint[d] );
    if (d+1 < dp->dims)
      printf(",");
  }
  printf("> ");
  printf(" sco=%f", dp->score);

  printf(")\n");
}

void printDPoints(const DPoint *const dps, const Rint n,
    const char *const prefix) {

  Rint i;
  printf( "%s", prefix );
  for (i = 0; i < n; ++i) {
    printf("  dp%d ", i);
    printDPoint( dps + i, "" );
  }
}
