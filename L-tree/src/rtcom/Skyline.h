/*
 * SSkyline.h
 *
 *  Created on: May 17, 2016
 *      Author: sidlausk
 *
 */

/**:   Skyline computation related routines
       ====================================                             **/

#include <assert.h>

#include "RSTTypes.h"
#include "RSTBase.h"

#ifndef LRRST_SSKYLINE_H_
#define LRRST_SSKYLINE_H_

#define DOM_LEFT    0
#define DOM_RIGHT   1
#define DOM_INCOMP  2

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

static const Rint SHIFTS[] = { 1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1
    << 5, 1 << 6, 1 << 7, 1 << 8, 1 << 9, 1 << 10, 1 << 11, 1 << 12, 1 << 13, 1
    << 14, 1 << 15, 1 << 16, 1 << 17, 1 << 18, 1 << 19, 1 << 20, 1 << 21, 1
    << 22, 1 << 23, 1 << 24, 1 << 25, 1 << 26, 1 << 27, 1 << 28, 1 << 29, 1
    << 30 };

static const Rint ALL_ONES[] = {
    (1 << 0) - 1, (1 << 1) - 1, (1 << 2) - 1, (1 << 3) - 1, (1 << 4) - 1,
    (1 << 5) - 1, (1 << 6) - 1, (1 << 7) - 1, (1 << 8) - 1, (1 << 9) - 1,
    (1 << 10) - 1, (1 << 11) - 1, (1 << 12) - 1, (1 << 13) - 1, (1 << 14) - 1,
    (1 << 15) - 1, (1 << 16) - 1, (1 << 17) - 1, (1 << 18) - 1, (1 << 19) - 1,
    (1 << 20) - 1, (1 << 21) - 1, (1 << 22) - 1, (1 << 23) - 1, (1 << 24) - 1,
    (1 << 25) - 1, (1 << 26) - 1, (1 << 27) - 1, (1 << 28) - 1, (1 << 29) - 1,
    (1 << 30) - 1 };

/*
 * Simple (but cache-efficient) skyline algorithm.
 */
int skylineNEW(const Rint cbit, DPoint *const points, const Rint num,
    const Rint dims);

int staircaseNEW( const Rint cbit, DPoint *const points, const Rint entries,
    const Rfloat ccoord[], const Rint dims, const Rfloat node_vol);

static inline uint32_t ltMask(const Rfloat *const left,
    const Rfloat *const right, const Rint dims) {
  Rint mask = 0, dim;
  for ( dim = 0; dim < dims; dim++ )
    if ( left[dim] < right[dim] )
      mask |= SHIFTS[dim];

  return mask;
}

static inline uint32_t gtMask(const Rfloat *const left,
    const Rfloat *const right, const Rint dims) {
  Rint mask = 0, dim;
  for ( dim = 0; dim < dims; dim++ )
    if ( left[dim] > right[dim] )
      mask |= SHIFTS[dim];

  return mask;
}

static inline uint32_t eqMask(const Rfloat *const left,
    const Rfloat *const right, const Rint dims) {
  Rint mask = 0, dim;
  for ( dim = 0; dim < dims; dim++ )
    if ( left[dim] == right[dim] )
      mask |= SHIFTS[dim];

  return mask;
}

/*
 * Returns a bit-mask with bits set where sky_value has a strictly smaller value
 * than cur_value.
 *
 * All-ones bitmask implies that cur_value is dominated by sky_value.
 *
 * All-zeros bit mask *does not* imply that sky_value is dominated by cur_value
 * because we consider extended skyline here (i.e., a strict dominance is
 * required). A second call to DomTest with substituted values are needed to
 * verify that's really the case.
 *
 */
static inline uint32_t DomTest(const Rfloat *const cur_value,
    const Rfloat *const sky_value, const Rint dims) {
  Rint mask = 0, dim;
  for ( dim = 0; dim < dims; dim++ )
    if ( sky_value[dim] < cur_value[dim] )
      mask |= SHIFTS[dim];

  return mask;
}

/*
 * 2-way dominance test with NO assumption for distinct value condition and
 * taking into account the targeted corner.
 *
 */
static inline int DominanceTest(const Rfloat *const t1,
    const Rfloat *const t2, const Rint dims, const Rint corner) {

  const uint32_t eq_mask = eqMask( t1, t2, dims );
//  printf( "eq: %s\n", uint2bin(eq_mask, dims) );

  if ( eq_mask == ALL_ONES[dims] )
    return DOM_INCOMP; // equal!

  const uint32_t gt_mask1 = gtMask( t1, t2, dims );
//  printf( "gt: %s\n", uint2bin(gt_mask1, dims) );

  if ( (eq_mask | gt_mask1) == (eq_mask | corner) )
    return DOM_LEFT;

  const uint32_t gt_mask2 = gtMask( t2, t1, dims );
//  printf( "gt: %s\n", uint2bin(gt_mask2, dims) );
  if ( (eq_mask | gt_mask2) == (eq_mask | corner) )
    return DOM_RIGHT;

  return DOM_INCOMP;
}

static inline void computeStaircasePoint(const Rfloat *const sky1,
    const Rfloat *const sky2, const Rint dims, const Rint corner,
    Rfloat *const buf) {

  Rint d;
  for ( d = 0; d < dims; ++d )
    if ( ( corner & (SHIFTS[d]) ) == 0 )
      buf[d] = fmin( sky1[d], sky2[d] );
    else
      buf[d] = fmax( sky1[d], sky2[d] );

}

/*
   * Returns TRUE if point p dominates no points in dpoints
   * (except for points for i and j indexes). TODO:
 */
static inline boolean dominatesAny( const DPoint *const dpoints, const Rint n,
    const Rint i, const Rint j, const Rfloat *const p ) {

  assert( n > 0 );
  const Rint kDims = dpoints[0].dims;
  const Rint kCorner = dpoints[0].corner;
  Rint k;
  for ( k = 0; k < n; ++k ) {
    if (k == i || k == j) continue;
    const int dom = DominanceTest( dpoints[k].dpoint, p, kDims, kCorner );
    if ( dom == DOM_LEFT ) {
      // If p dominates point k (in dpoints).
      return FALSE;
    }
  }
  return TRUE;
}

static inline boolean dpContains( const DPoint *const dpoints, const Rint n,
    const Rfloat *const stair_cand ) {

  if ( n == 0 ) return FALSE;
  const Rint kDims = dpoints[0].dims;
  Rint k;
  for ( k = 0; k < n; ++k )
    if ( ALL_ONES[kDims] == eqMask( dpoints[k].dpoint, stair_cand, kDims ) )
      return TRUE;
  return FALSE;
}

void printP(const Rfloat *const p, const Rint dims, const char *const prefix);

void printPs(const Rfloat *const p, const Rint num, const Rint dims,
    const char *const prefix);

void printR(const typinterval *const r, const Rint dims, const char *const prefix);

void printRs( char *const rects, const Rint num, const Rint dims,
    const char *const prefix );

const char* uint2bin(uint32_t x, const Rint dims);


#endif /* LRRST_SSKYLINE_H_ */
