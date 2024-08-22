/*
 * Skyline.c
 *
 *  Created on: Jun 24, 2016
 *      Author: sidlausk
 */

#include "Skyline.h"
#include "LRSTUtil.h"

#include <assert.h>

int skylineNEW(const Rint cbit, DPoint *const points, const Rint num,
    const Rint dims) {

  Rint i, head = 0, tail = num - 1;
  DPoint *const D = points;

  // simple (but cache-efficient) skyline algorithm follows:
  while ( head < tail ) {
    i = head + 1;
    while ( i <= tail ) {
      const int dom = DominanceTest( D[head].dpoint, D[i].dpoint, dims, cbit );
      if ( dom == DOM_LEFT ) {
        if ( tail != i )
          memcpyDPs( &D[i], &D[tail], 1 );
        tail--;
      } else if (dom == DOM_RIGHT) {
        memcpyDPs( &D[head], &D[i], 1 );
        if ( tail != i )
          memcpyDPs( &D[i], &D[tail], 1 );
        tail--;
        i = head + 1;
      } else
        ++i;
    }
    head++;
  }

  return tail + 1;
}

int staircaseNEW( const Rint cbit, DPoint *const sky_points, const Rint kSkySize,
    const Rfloat ccoord[], const Rint kDims, const Rfloat kNodeVol ) {

  assert(kSkySize > 0);

  if (kSkySize < 2) // a single sky point is equivalent to the same stair point
    return 0;

//  printP( ccoord, dims, "COR: " );

  const size_t kCorners = pow( 2, kDims );
  const size_t kMaxNumSta = kSkySize * kCorners; // FIXME: how to set that ???
  const Rint kOppCorner = ~cbit;
  DPoint sta_candidates[kMaxNumSta]; // corner candidates
  initEmptyDPoints( sta_candidates, kMaxNumSta, kDims );
  Rint sta_count = 0;

  Rint i, j;

  // Keep track of the deadliest candidate:
  Rint deadliest_idx = 0;
  Rfloat deadliest_vol = 0.0;


  for ( i = 0; i < kSkySize - 1; ++i ) {
    for ( j = i + 1; j < kSkySize; ++j ) {

      Rfloat stair_cand[kDims];
      computeStaircasePoint(sky_points[i].dpoint, sky_points[j].dpoint, kDims, kOppCorner, stair_cand);
      // Ignore if the candidate eliminates less than X% deadspace in a node:
      const Rfloat cc_vol = volumePs( ccoord, stair_cand, kDims );
      if ( cc_vol / kNodeVol < DP_MIN_DSPACE )
        continue;

      if ( dominatesAny( sky_points, kSkySize, i, j, stair_cand ) ) {
        assert( cc_vol < kNodeVol ); // valid must have a non-zero volume ?

        // It can happen that a pair of skyline points generate more than one
        // valid staircase points. Generally, the extra staircase sky_points are
        // also captured by other pairs. As such, we might have unnecessary
        // duplicates here that we better eliminate.
        //
        // Check if such staircase point hasn't been added before:
        if ( !dpContains( sta_candidates, kSkySize, stair_cand ) ) {
          // Then store this potential candidate:
          assert( sta_count < kMaxNumSta );
          initDPoint2( &sta_candidates[sta_count], cbit, ccoord, stair_cand, kDims);
          sta_candidates[sta_count].score = cc_vol;

          if ( cc_vol > deadliest_vol ) {
            // Keep track of the deadliest:
            deadliest_vol = cc_vol;
            deadliest_idx = sta_count;
          }
          ++sta_count;
//          assert( num_sta < MaxNumSta );
//          printDPoints( sta_candidates, num_sta, "MID-WAY:\n");
        }
      } // end of is valid
    } // end of j
  } // end of i

  // Calculate scores based on the deadliest candidate:
  if ( deadliest_idx != 0 )
	  swapDPs( sta_candidates + 0, sta_candidates + deadliest_idx );
  sta_count = computeScores( sta_candidates, sta_count, kNodeVol );

//  printDPoints( sta_candidates, num_sta, "BEFORE SORTING:\n");
  qsort( sta_candidates, sta_count, sizeof(DPoint), comp_by_score );
//  printDPoints( sta_candidates, num_sta, "AFTER  SORTING:\n");

  // Deep-copy stair candidates over sky candidates (at most kSkySize):
  memcpyDPs( sky_points, sta_candidates, MIN(sta_count, kSkySize) );

  // Clean-up
  freeDPoints( sta_candidates, kMaxNumSta );

  return MIN(sta_count, kSkySize);
}

void printP(const Rfloat *const p, const Rint dims, const char *const prefix) {
  Rint d;
  printf( "%s<", prefix );
  for (d = 0; d < dims; ++d)
    printf( " %.2f", p[d] );
  printf( " >\n" );
}

void printPs(const Rfloat *const pnts, const Rint num, const Rint dims,
    const char *const prefix) {
  Rint i;
  const Rfloat* p = pnts;
  printf( "%s", prefix );

  for (i = 0; i < num; ++i) {
    printP( p, dims, "  " );
    p += dims;
  }
}

void printR(const typinterval *const r, const Rint dims, const char *const prefix) {
  Rint d;
  printf( "%s<", prefix );
  Rfloat vol = 1.0;
  for (d = 0; d < dims; ++d) {
    printf( "(%.2f, %.2f)", r[d].l, r[d].h );
    vol *= fabs( r[d].h - r[d].l );
  }
  printf( "> vol=%.1f\n", vol );
}

void printRs( char *const rects, const Rint num, const Rint dims,
    const char *const prefix ) {
  Rint i;
  char* r = rects;
  printf( "%s", prefix );
  for (i = 0; i < num; ++i) {
    printf( "%d", *((Rint*)r) );
    r += sizeof(Rint);
    printR( (typinterval*)r, dims, "" );
    r += (sizeof(Rfloat) * 2 * dims);
  }
}

const char* uint2bin(uint32_t x, const Rint dims) {
    static char b[32];
    memset( b, '\0', 32 );

    uint32_t z;
    char *p = b;
    // Print order: dim_2 dim_1 dim_0:
//    for ( z = 1 << (dims-1); z > 0; z >>= 1 ) {
//      *p++ = (x & z) ? '1' : '0';
//    }
    // Print order: dim_0 dim_1 dim_2
    for ( z = 1; z < 1 << dims; z <<= 1 ) {
      *p++ = (x & z) ? '1' : '0';
    }

    return b;
}
