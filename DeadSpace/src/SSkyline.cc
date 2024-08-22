/*
 * SSkyline.cc
 *
 *  Created on: Jan 5, 2016
 *      Author: sidlausk
 */

#include "SSkyline.h"

#include <bitset>

SSkyline::SSkyline(const vector<Box> &input):
  n_(input.size()), num_corners_(1<<NUM_DIMS) {
  corner_points_.reserve(num_corners_);

  for (uint32_t c = 0; c <= num_corners_ - 1; ++c) {
    corner_points_[c] = new Point[n_];
  }

  for (uint32_t i = 0; i < n_; ++i) {
    for (uint32_t c = 0; c <= num_corners_ - 1; ++c) {
      bitset<NUM_DIMS> bs(c);
      for (uint32_t d = 0; d < NUM_DIMS; ++d) {
        corner_points_[c][i][d] = bs[d] == 0 ? input[i].lo[d] : input[i].hi[d];
      }

//      if ( c == 0 )
//        assert( corner_points_[c][i] == input[i].lo );
//      if ( c == num_corners_ - 1)
//        assert( corner_points_[c][i] == input[i].second );
    }
  }
}

SSkyline::~SSkyline() {
  for (uint32_t c = 0; c <= num_corners_ - 1; ++c) {
    delete[] corner_points_[c];
  }
}

