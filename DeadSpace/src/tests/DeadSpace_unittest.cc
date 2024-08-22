/*
 * DeadSpace_unittest.cc
 *
 *  Created on: Apr 13, 2016
 *      Author: sidlausk
 */

// Copyright 2005, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// A sample program demonstrating using Google C++ testing framework.
//
// Author: wan@google.com (Zhanyong Wan)


// In this example, we use a more advanced feature of Google Test called
// test fixture.
//
// A test fixture is a place to hold objects and functions shared by
// all tests in a test case.  Using a test fixture avoids duplicating
// the test code necessary to initialize and cleanup those common
// objects for each test.  It is also useful for defining sub-routines
// that your tests need to invoke a lot.
//
// <TechnicalDetails>
//
// The tests share the test fixture in the sense of code sharing, not
// data sharing.  Each test is given its own fresh copy of the
// fixture.  You cannot expect the data modified by one test to be
// passed on to another test, which is a bad idea.
//
// The reason for this design is that tests should be independent and
// repeatable.  In particular, a test should not fail as the result of
// another test's failure.  If one test depends on info produced by
// another test, then the two tests should really be one big test.
//
// The macros for indicating the success/failure of a test
// (EXPECT_TRUE, FAIL, etc) need to know what the current test is
// (when Google Test prints the test result, it tells you which test
// each failure belongs to).  Technically, these macros invoke a
// member function of the Test class.  Therefore, you cannot use them
// in a global function.  That's why you should put test sub-routines
// in a test fixture.
//
// </TechnicalDetails>

#include <limits.h>
#include <unordered_map>

#include "gtest/gtest.h"

#include "ASCIDumpParser.h"
#include "Utils.h"
#include "SSkyline.h"
#include "NodeVisualizer.h"

// To use a test fixture, derive a class from testing::Test.
class DeadSpaceTest : public testing::Test {
 protected:  // You should make the members protected s.t. they can be
             // accessed from sub-classes.

  // virtual void SetUp() will be called before each test is run.  You
  // should define it if you need to initialize the varaibles.
  // Otherwise, this can be skipped.
  virtual void SetUp() {
    const std::string input_filepath = "tests/RRSTree_test_data.RSF.ASC";
    parser = new ASCIDumpParser( input_filepath, 0 );
  }

  // virtual void TearDown() will be called after each test is run.
  // You should define it if there is cleanup work to do.  Otherwise,
  // you don't have to provide it.
  //
   virtual void TearDown() {
     delete parser;
   }

  // A helper function that some test uses.
//  static int Double(int n) {
//    return 2*n;
//  }

  // Declare the variables your tests want to use:
  ASCIDumpParser *parser;
};

// When you have a test fixture, you define a test using TEST_F
// instead of TEST.

TEST_F(DeadSpaceTest, UniverseSize) {
  // Expected values:
  Point p1, p2;
  p1[0] = 0.0; p1[1] = 0.0;
  p2[0] = 10.0; p2[1] = 9.0;
  const Box universe( p1, p2 );

  NodeInfo node;
  while ( parser->getNextNode(node) ) {
    node.mbr = Box::computeMBR( node.entries );
    EXPECT_EQ( node.mbr, universe );

    break;
  }
}

TEST_F(DeadSpaceTest, TotalDeadSpaceVolume) {
  // Expected values:
//  const double abs_volume[3] = { 17.5, 46.0, 5.5 };

  NodeInfo node;
  int i = 0;
  while ( parser->getNextNode(node) ) {

    node.mbr = Box::computeMBR( node.entries );

    Utils::computeDeadspace( node, false );
//    EXPECT_EQ( node.total_deadspace_abs, abs_volume[i] ); FIXME
    ++i;
  }
}

TEST_F(DeadSpaceTest, SSkyline) {
  // Expected values:
  const uint32_t num_corners = 4;
  uint32_t sky_sizes[num_corners] = { 1, 1, 2, 2};

  std::vector< vector<Point> > correct_skies;

  // Corner 00:
  std::vector<Point> sky00;
  double coords[NUM_DIMS] = { 0, 0 };
  sky00.push_back( Point(coords) );
  correct_skies.push_back( sky00 );

  // Corner 01:
  std::vector<Point> sky01;
  coords[0] = 10; coords[1] = 0;
  sky01.push_back( Point(coords) );
  correct_skies.push_back( sky01 );

  // Corner 10:
  std::vector<Point> sky10;
  coords[0] = 0; coords[1] = 6;
  sky10.push_back( Point(coords) );
  coords[0] = 2; coords[1] = 9;
  sky10.push_back( Point(coords) );
  correct_skies.push_back( sky10 );

  // Corner 11:
  std::vector<Point> sky11;
  coords[0] = 10; coords[1] = 6;
  sky11.push_back( Point(coords) );
  coords[0] = 7; coords[1] = 9;
  sky11.push_back( Point(coords) );
  correct_skies.push_back( sky11 );

  std::vector<Point> not_sky;
  coords[0] = 1; coords[1] = 4;
  not_sky.push_back( Point(coords) );
  coords[0] = 3; coords[1] = 3;
  not_sky.push_back( Point(coords) );
  coords[0] = 3; coords[1] = 2;
  not_sky.push_back( Point(coords) );
  coords[0] = 1; coords[1] = 4;
  not_sky.push_back( Point(coords) );
  coords[0] = 7; coords[1] = 6;
  not_sky.push_back( Point(coords) );

  NodeInfo node;
  while ( parser->getNextNode(node) ) {
    node.mbr = Box::computeMBR( node.entries );
    Utils::computeDeadspace( node, false );

    for (uint32_t c = 0; c < NUM_CORNERS; ++c) {
		bitset<NUM_DIMS> bs(c);
		const Point ccoord = node.mbr.getCorner(c);
		vector<ClipPoint> dcands; // deadly candidates
		for (uint32_t i = 0; i < node.entries.size(); ++i) {
			dcands.push_back( ClipPoint(c, ccoord, node.entries[i].getCorner(c) ) );
		}

		SSkyline::skyline( dcands );
		assert(dcands.size() > 0 && dcands.size() <= node.entries.size() );

      EXPECT_EQ( dcands.size(), sky_sizes[c] );

      for (uint32_t s1 = 0; s1 < correct_skies[c].size(); ++s1) {
        bool found = std::find( dcands.begin(), dcands.end(), correct_skies[c][s1]) != dcands.end();
        EXPECT_TRUE( found );
      }

      for (uint32_t s1 = 0; s1 < not_sky.size(); ++s1) {
        bool found = std::find( dcands.begin(), dcands.end(), not_sky[s1]) != dcands.end();
        EXPECT_FALSE( found );
      }
    }

    break; // At the moment: tests only root node
  }
}
