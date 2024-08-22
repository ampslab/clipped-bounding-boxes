/*
 * Utils.h
 *
 *  Created on: Jan 25, 2016
 *      Author: sidlausk
 */

#ifndef UTILS_H_
#define UTILS_H_

#include "SpatialObjects.h"
#include "ASCIDumpParser.h"

// these should really be in a namespace!
using Circle = std::pair< double, Point >;
using PointSet = std::vector< Point >;
using Line = std::pair<Point, Point>;

class Utils {

  static void computePixelCoverage2D( NodeInfo &node, bool verbose );
  static void computeExact2DArea( NodeInfo &node, bool verbose );
  static void computeExactClippedVol2D( NodeInfo &node, bool verbose );
  static void computePixelClippedDeadspace2D( NodeInfo &node, bool verbose );
  static void computePixelCoverage3D( NodeInfo &node, bool verbose );
  static void computePixelClippedDeadspace3D( NodeInfo &node, bool verbose );
  static void computePixelCHDeadspace3D( NodeInfo &node, bool verbose );
  static void computeSphericalDeadspace2D( NodeInfo &node, bool verbose );
  static void approximateCircleWithPolygon2D( const Circle &c, const int n_edges,
      std::vector<Point> &poly );
  static void computeConvexHullAndFriends2D( NodeInfo &node, bool verbose );
  
  static void populateBoxCorners( 
  	std::vector< Point >& corners, const std::vector< Box > boxes );
  
  // static methods for computing minimum enclosing spheres.
  static Circle circleDefinedByUpToThreePoints( const PointSet& points );
  static Circle b_minidisk( PointSet& P, PointSet& R );
  static Circle welzl91Algorithm( PointSet& points );
  static Circle computeRadiusOfMinEnclosingCircle( const std::vector< Box >& rectangles );

  static std::vector<Line> cbb2lines2D( const Box &b, const
      std::vector<ClipPoint> &dpoints, const uint32_t k );
  static std::vector<Line> cbb2lines3D( const Box &b, const
      std::vector<ClipPoint> &dpoints, const uint32_t k );

  static std::vector<Line> box2lines3D( const Box &b );
  static std::vector<Line> box2linesExceptCorner3D( const Box &b, const Point &corner );

public:
  Utils();
  virtual ~Utils();

  /*
   * Runs all deadspace computations (for currently supported bounding methods).
   *
   * Note: only for 2D and 3D datasets (i.e., truly spatial data)!
   */
  static void computeDeadspace( NodeInfo &node, bool verbose = false ) {
    if (NUM_DIMS == 2) {
      computePixelCoverage2D( node, verbose );
      computeExact2DArea( node, verbose );

  		computeSphericalDeadspace2D( node, verbose );
  		computeConvexHullAndFriends2D( node, verbose );

      computePixelClippedDeadspace2D( node, verbose ); // pixel-based dead space
      computeExactClippedVol2D( node, verbose );
    } else if (NUM_DIMS == 3) {
      computePixelCoverage3D( node, verbose );
      computePixelClippedDeadspace3D( node, verbose );
    } else {
      node.pixel_coverage_abs = 0.0;
      node.pixel_overlap_abs = 0.0;
    }
  }
  
  static std::vector<Line> cbb2lines( const Box &b, const
      std::vector<ClipPoint> &dpoints, const uint32_t k );

  constexpr static double pi = 3.141592653589793238462643383279502884;
};

#endif /* UTILS_H_ */
