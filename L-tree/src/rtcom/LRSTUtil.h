/*
 * LRSTUtil.h
 *
 *  Created on: Jun 9, 2016
 *      Author: sidlausk
 */

/**:   L-tree related utility routines
       ==============================                                   **/

#ifndef RTCOM_LRSTUTIL_H_
#define RTCOM_LRSTUTIL_H_

#include <assert.h>

#include "RSTTypes.h"
#include "RSTBase.h"

// DPoint flags
#define DP_NULL     0
#define DP_ALLOCED  1
#define DP_VALID    2

#define DP_MIN_DSPACE 0.025 // i.e., at least 2.5% of node's volume

int comp_by_score( const void * el1, const void * el2 );

int comp_by_vol(const void * elem1, const void * elem2, void * arg);

void initEmptyDPoints( DPoint *const dpoints, const Rint n, const Rint dims);

void initDPoint( DPoint *const dpoint, const Rint c,
                   const typcoord *const ccoord,
                   const typinterval *const cand_mbr, const Rint dims);

void initDPoint2( DPoint *const dpoint, const Rint c,
                    const typcoord *const ccoord,
                    const typcoord *const cand_coord, const Rint dims);

void freeDPoint( DPoint *const dpoint );

void freeDPoints( DPoint *const dpoints, const Rint n );

void nullifyDPoints( DPoint *const dpoints, const Rint n );

void memcpyDPs( DPoint *const dst, DPoint *const src, const Rint n );

void swapDPs(  DPoint *i, DPoint *j );

Rint computeScores( DPoint *const dpoints,
					const Rint num,
					const Rfloat kNodeVol );

Rint eliminateZeroVolMBRs( DPoint *const dpoints, const Rint num,
    const Rfloat node_vol );

Rfloat computeOverlap( const typinterval *const mbr1,
    const typinterval *const mbr2, typinterval *const buf, const Rint dims );

void printDPoint(const DPoint *const dp, const char *const prefix);

void printDPoints(const DPoint *const dps, const Rint n,
    const char *const prefix);

/* Inlined functions */
static inline void computeCorner(const Rint corner, const typinterval *const box,
    Rfloat*const buf, const Rint dims) {
  Rint d;
  for ( d = 0; d < dims; ++d ) {
    if ( ( corner & (1 << d) ) == 0 )
      buf[d] = box[d].l;
    else
      buf[d] = box[d].h;
  }
}

static inline void makeMBR(const Rfloat *const p1, const Rfloat *const p2,
    const Rint dims, typinterval *const mbr) {
  Rint d;
  for (d = 0; d < dims; ++d) {
    mbr[d].l = fmin( p1[d], p2[d] );
    mbr[d].h = fmax( p1[d], p2[d] );
  }
}

static inline void makeMBRfromDP(const DPoint *const dp, typinterval *const buf) {
  Rint d;
  for (d = 0; d < dp->dims; ++d) {
    buf[d].l = fmin( dp->ccoord[d], dp->dpoint[d] );
    buf[d].h = fmax( dp->ccoord[d], dp->dpoint[d] );
  }
}

static inline Rfloat volume(const typinterval *const box, const Rint dims) {
  Rint d;
  Rfloat vol = 1.0;
  for ( d = 0; d < dims; ++d ) {
    vol *= box[d].h - box[d].l;
    assert( box[d].h >= box[d].l );
  }
  return vol;
}

static inline void storeDeadlyPoints(  DPointInfo *const dp_entry,
    const DPoint *const dpoints, const Rint num_dpoints) {

  if ( num_dpoints == 0 ) return;

  const Rint Dims = dpoints[0].dims;
  const size_t DPSize = sizeof(Rint) + SIZEcoord * Dims;

  assert( dpoints != NULL );
  assert( dp_entry->entLen == 0 );
  assert( dp_entry->dpoints == NULL );
  dp_entry->entLen = num_dpoints;
  dp_entry->dpoints = malloc( num_dpoints * DPSize );

  char *write_slot = dp_entry->dpoints;
  Rint i, d;
  for ( i = 0; i < num_dpoints; ++i ) {
    memcpy( write_slot, &dpoints[i].corner, sizeof(Rint) );
    write_slot += sizeof(Rint);
    for (d = 0; d < Dims; ++d) {
      memcpy( write_slot, &dpoints[i].dpoint[d], SIZEcoord );
      write_slot += SIZEcoord;
    }
  }
}

static inline void loadDeadlyPoint( const char *const from, typcoord *const to_dmbr,
    Rint *const to_corner, const Rint dims ) {

  *to_corner = *((Rint*)from);
  memcpy( to_dmbr, from + sizeof(Rint), SIZEcoord*dims );
}

static inline Rfloat volumePs(const Rfloat *const p1, const Rfloat *const p2,
    const Rint dims) {
  Rint d;
  Rfloat vol = 1.0;
  for ( d = 0; d < dims; ++d )
    vol *= fabs( p1[d] - p2[d] );
  return vol;
}

#endif /* RTCOM_LRSTUTIL_H_ */
