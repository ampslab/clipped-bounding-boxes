/*
 * SSkyline.h
 *
 *  Created on: Jan 5, 2016
 *      Author: sidlausk
 *
 *  Simple Skyline
 */

#ifndef SKYLINE_SSKYLINE_H_
#define SKYLINE_SSKYLINE_H_

#include <stdint.h>

#include <vector>
#include <cstring>
#include <bitset>
#include <algorithm>

#include "ASCIDumpParser.h"
#include "SpatialObjects.h"

using namespace std;

#define DOM_LEFT_    0
#define DOM_RIGHT_   1
#define DOM_INCOMP_  2

#ifndef NDEBUG
  #define DP_MIN_DSPACE 0.0 // take all possible points into account
#else
  #define DP_MIN_DSPACE 0.025 // i.e., at least 2.5% of node's volume
#endif

static const uint32_t SHIFTS[] = { 1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1
    << 5, 1 << 6, 1 << 7, 1 << 8, 1 << 9, 1 << 10, 1 << 11, 1 << 12, 1 << 13, 1
    << 14, 1 << 15, 1 << 16, 1 << 17, 1 << 18, 1 << 19, 1 << 20, 1 << 21, 1
    << 22, 1 << 23, 1 << 24, 1 << 25, 1 << 26, 1 << 27, 1 << 28, 1 << 29, 1
    << 30 };

static const uint32_t ALL_ONES[] = {
    (1 << 0) - 1, (1 << 1) - 1, (1 << 2) - 1, (1 << 3) - 1, (1 << 4) - 1,
    (1 << 5) - 1, (1 << 6) - 1, (1 << 7) - 1, (1 << 8) - 1, (1 << 9) - 1,
    (1 << 10) - 1, (1 << 11) - 1, (1 << 12) - 1, (1 << 13) - 1, (1 << 14) - 1,
    (1 << 15) - 1, (1 << 16) - 1, (1 << 17) - 1, (1 << 18) - 1, (1 << 19) - 1,
    (1 << 20) - 1, (1 << 21) - 1, (1 << 22) - 1, (1 << 23) - 1, (1 << 24) - 1,
    (1 << 25) - 1, (1 << 26) - 1, (1 << 27) - 1, (1 << 28) - 1, (1 << 29) - 1,
    (1 << 30) - 1 };

class SSkyline {
private:
  const uint32_t n_;
  const uint32_t num_corners_;
  vector<Point*> corner_points_;

  /*
   * Dominance test returning result as a bitmap.
   * Note: this version assumes distinct value condition.
   */
  inline uint32_t DT(const Point &cur_value, const Point &sky_value) {
    uint32_t lattice = 0;
    for (uint32_t dim = 0; dim < NUM_DIMS; dim++)
      if ( sky_value[dim] < cur_value[dim] )
        lattice |= SHIFTS[dim];

    return lattice;
  }

  /*
   * 2-way dominance test with NO assumption for distinct value condition and
   * taking into account the targeted corner.
   *
   */
  static inline int DominanceTest(const Point &p1, const Point &p2,
		  const uint32_t corner) {

    const uint32_t eq_mask = eqMask( p1, p2 );
  //  printf( "eq: %s\n", uint2bin(eq_mask, dims) );

    if ( eq_mask == ALL_ONES[NUM_DIMS] )
      return DOM_INCOMP_; // equal!

    const uint32_t gt_mask1 = gtMask( p1, p2 );
  //  printf( "gt: %s\n", uint2bin(gt_mask1, dims) );

    if ( (eq_mask | gt_mask1) == (eq_mask | corner) )
      return DOM_LEFT_;

    const uint32_t gt_mask2 = gtMask( p2, p1 );
  //  printf( "gt: %s\n", uint2bin(gt_mask2, dims) );
    if ( (eq_mask | gt_mask2) == (eq_mask | corner) )
      return DOM_RIGHT_;

    return DOM_INCOMP_;
  }

  static inline uint32_t eqMask(const Point &left, const Point &right) {
    uint32_t mask = 0;
    for (uint32_t dim = 0; dim < NUM_DIMS; dim++)
      if ( left[dim] == right[dim] )
        mask |= SHIFTS[dim];

    return mask;
  }

  static inline uint32_t gtMask(const Point &left, const Point &right) {
	uint32_t mask = 0;
	for (uint32_t dim = 0; dim < NUM_DIMS; dim++)
      if ( left[dim] > right[dim] )
        mask |= SHIFTS[dim];

    return mask;
  }

  struct ComparatorX {
    bool operator() (const Point &a, const Point &b) { return a[0] < b[0]; }
  } compX;

public:

  static vector<ClipPoint> skyline( vector<ClipPoint> &data ) {
	  const uint32_t corner = data[0].corner_code;
      int i, head = 0, tail = data.size() - 1;

      // simple (but cache-efficient) skyline algorithm follows:
      while ( head < tail ) {
        i = head + 1;
        while ( i <= tail ) {
          const int dom_mask = DominanceTest( data[head].dpoint, data[i].dpoint, corner );
          if ( dom_mask == DOM_LEFT_ ) {
            data[i] = data[tail--];
            data.pop_back();
            assert( tail == data.size() - 1 );
          } else if ( dom_mask == DOM_RIGHT_ ) {
              data[head] = data[i];
              data[i] = data[tail--];
              i = head + 1;
              data.pop_back();
              assert(tail == data.size() - 1);
            } else
              ++i;
          }
        head++;
      }
      return data;
  }

  /*
   * Returns TRUE if point p dominates no points in dpoints
   * (except for points for i and j indexes).
   */
  static inline bool dominatesNone( const vector<ClipPoint> &dpoints,
		  const uint32_t i, const uint32_t j, const Point &p ) {

    assert( dpoints.size() > 0 );
    const uint32_t kCorner = dpoints[0].corner_code;

    for (uint32_t k = 0; k < dpoints.size(); ++k ) {
      if (k == i || k == j) continue;
      const int dom = DominanceTest( dpoints[k].dpoint, p, kCorner );
      if ( dom == DOM_LEFT_ ) {
        // If p dominates point k (in dpoints).
        return false;
      }
    }
    return true;
  }

  static inline Point computeStaircasePoint(const Point &sky1,
		  const Point &sky2,const uint32_t corner) {

	  Point sta;
	  for (uint32_t d = 0; d < NUM_DIMS; ++d )
		  if ( ( corner & (SHIFTS[d]) ) == 0 )
			  sta[d] = fmin( sky1[d], sky2[d] );
		  else
			  sta[d] = fmax( sky1[d], sky2[d] );

	  return sta;
  }

  static inline bool dpContains(const vector<ClipPoint> &c, const Point &p) {
	  for (uint32_t i = 0; i < c.size(); ++i) {
		if ( c[i].dpoint == p )
			return true;
	}
	  return false;
  }

  static vector<ClipPoint> staircase( vector<ClipPoint> &sky, const double kNodeVol ) {
	  vector<ClipPoint> staircase;

	  if (sky.size() == 1) {
		  assert( sky[0].ccoord == sky[0].dpoint );
		  sky.clear( );
		  return staircase;
	  }

	  const uint32_t kCCode = sky[0].corner_code;
	  const uint32_t kOppCorner = ~kCCode; // & (NUM_DIMS - 1);
	  const Point &kCorner = sky[0].ccoord;
	  const uint32_t kSkySize = sky.size();

	  for ( uint32_t i = 0; i < kSkySize - 1; ++i ) {
	    for ( uint32_t j = i + 1; j < kSkySize; ++j ) {
	    	Point sta_cand = computeStaircasePoint(sky[i].dpoint, sky[j].dpoint, kOppCorner);

	        if ( !dominatesNone( sky, i, j, sta_cand ) )
	        	continue;

	        // With current logic, it can happen that a pair of skyline points
	        // generate more than one valid staircase points. Generally, the
	        // extra staircase sky_points are also captured by other pairs.
	        // As such, we might have unnecessary duplicates here that we
	        // better eliminate.
	        //
	        // Check if such staircase point hasn't been added before:
	        if ( dpContains(staircase, sta_cand) )
	        	continue;

	        // Then store this potential candidate
	        staircase.push_back( ClipPoint(kCCode, kCorner, sta_cand) );
	    }
	  }
	  assert( !staircase.empty() );

	  return staircase;
  }

  SSkyline(const vector<Box> &input);
  virtual ~SSkyline();

  void run(vector<vector<Point> > &skies) {
    skies.clear();
    skies.resize( num_corners_ );
    for (uint32_t c = 0; c <= num_corners_ - 1; ++c) {
      int i, head = 0, tail = n_ - 1;
      Point *const D = corner_points_[c];

      // simple (but cache-efficient) skyline algorithm follows:
      while ( head < tail ) {
        i = head + 1;
        while ( i <= tail ) {
          const int dom_mask = DT( D[head], D[i] );
          if ( dom_mask == DOM_LEFT_ ) {
            D[i] = D[tail--];
          } else if ( dom_mask == DOM_RIGHT_ ) {
              D[head] = D[i];
              D[i] = D[tail--];
              i = head + 1;
            } else
              ++i;
          }
        head++;
      }
      // sskyline

      skies[c].reserve( tail + 1 );
      for (int j = 0; j <= tail; ++j) {
        skies[c].push_back( D[j] );
      }
//      bitset<NUM_DIMS> bs(c);
//      printf( "Skyline%s: %lu\n", bs.to_string().c_str(), skylines[c].size() );

    } // for each corner
  }

//  vector<vector<POINT> > computeStaircasePoints(vector<vector<POINT> > &skies) {
//    vector<vector<POINT> > stairs(num_corners_);
//    for (uint32_t i = 0; i < skies.size(); ++i) {
//      std::sort(skies[i].begin(), skies[i].end(), compX);
//      const vector<POINT> &sky = skies[i];
//
//      for (uint32_t p = 1; p < sky.size(); ++p) {
//        POINT nonsky;
//        for (int d = 0; d < NUM_DIMS; ++d) {
//
//        }
//        stairs[i].push_back( POINT() );
//      }
//    }
//
//    return stairs;
//  }
};

#endif /* SKYLINE_SSKYLINE_H_ */
