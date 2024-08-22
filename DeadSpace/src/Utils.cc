/*
 * Utils.cc
 *
 *  Created on: Jan 25, 2016
 *      Author: sidlausk
 */

#include "Utils.h"
#include "SpatialObjects.h"	// for midpoint() calculations

#include <cstring>
#include <random>	// std::default_random_engine, std::shuffle
#include <cmath> // sin() and cos()
#include <set>

#include "NodeVisualizer.h"


Utils::Utils() {
  // TODO Auto-generated constructor stub

}

Utils::~Utils() {
  // TODO Auto-generated destructor stub
}

// Sorts points so that they follow clock-wise order for a given corner:
typedef struct CWFunctor {
//  const bitset<NUM_DIMS> sort_mask;
  const uint32_t sort_mask;
  CWFunctor(const uint32_t corner): sort_mask(corner) { }

  bool operator()(const Point &a, const Point &b) const {
    // THIS WORKS ONLY WITH 2D case
    assert( NUM_DIMS == 2 );

    switch (sort_mask) {
      case 0: // corner = 00
        return a[0] > b[0];

      case 1: // corner = 01
        return a[1] > b[1];

      case 2: // corner = 10
        return a[1] < b[1];

      case 3: // corner = 11
        return a[0] < b[0];

      default:
        assert( false );
    }

//    if (sort_mask[0] == sort_mask[1])
//      return a[1] > b[1];
//    else
//      return a[1] < b[1];

//    uint32_t d;
//    for (d = 0; d < NUM_DIMS && a[d] == b[d]; ++d)
//      ;
//    if (d == NUM_DIMS) return false;
//
//    if (d == 0)
//      return a[0] < b[0];
//    else {
//      if (sort_mask[d-1] == sort_mask[d])
//        return a[d] > b[d];
//      else
//        return a[d] < b[d];
//    }
    assert(false);
    return false;
  }
} CWFunctor;

/*
 * Converts CBB to a set of lines using k clip points.
 *
 * Obviously k must be less than the total number of clip points.
 */
std::vector<Line> Utils::cbb2lines( const Box &b,
    const std::vector<ClipPoint> &dpoints, const uint32_t topk ) {

  if ( NUM_DIMS == 2 )
    return cbb2lines2D(b, dpoints, topk);
  if ( NUM_DIMS == 3 )
    return cbb2lines3D(b, dpoints, topk);

  // not supported for other dimensions..
}

std::vector<Line> Utils::cbb2lines2D( const Box &b,
    const std::vector<ClipPoint> &dpoints, const uint32_t topk ) {

  assert( topk <= dpoints.size() );

  const int kCW[NUM_CORNERS] = {0, 2, 3, 1}; // corners in clock-wise order

  std::vector<Line> cbb;

  // For tracking the very first and current last CBB points:
  Point first_cbb_point; // the very first CBB point
  Point prevc_cbb_point; // the current last CBB point

  // Start from corner 00 and go clock-wise
  for (int c_idx = 0; c_idx < NUM_CORNERS; ++c_idx) {
    const uint32_t corner = kCW[c_idx];
    // Note 0 is code of the very first corner and 1---of last corner code:
    const int next_corner = c_idx + 1 <  NUM_CORNERS ? kCW[c_idx + 1] : 0;
    const int prev_corner = c_idx - 1 >= 0 ? kCW[c_idx - 1] : 1;
    const int oppo_corner = ~corner & (NUM_CORNERS - 1); // opposite corner

    vector<Point> corner_dpoints;
    for (uint32_t p = 0; p < topk; ++p)
      if (dpoints[p].corner_code == corner)
        corner_dpoints.push_back( dpoints[p].dpoint );

    if ( corner_dpoints.empty() ) {
      /* Corner is not clipped */

      if (c_idx == 0)
        // the MBB corner will be the very first starting CBB point
        first_cbb_point = b.getCorner(corner);
      else
        // if it's not the 1st corner, connect with the previous last CBB point
        cbb.push_back( Line( b.getCorner(corner), prevc_cbb_point ) );

      if (c_idx == NUM_CORNERS - 1)
        // if it's the last corner, connect with the very first CBB point
        cbb.push_back( Line( b.getCorner(corner), first_cbb_point ) );

      // New last CBB point (which is simply a current corner):
      prevc_cbb_point = b.getCorner( corner );

    } else if ( corner_dpoints.size() == 1 ) {
      /* Corner is clipped using a single clipping point */
      Box deadmbb( b.getCorner(corner), corner_dpoints[0] );

      if (c_idx == 0)
        // store the very first starting point (to connect with the last later)
        first_cbb_point = deadmbb.getCorner( prev_corner );
      else
        // if it's not the 1st corner, connect with the previous last CBB point
        cbb.push_back( Line(deadmbb.getCorner(prev_corner), prevc_cbb_point) );

      if (c_idx == NUM_CORNERS - 1)
        // if it's the last corner, connect with the very first CBB point
        cbb.push_back( Line( deadmbb.getCorner(next_corner), first_cbb_point) );

      std::vector<Line> lines = deadmbb.getEdges( oppo_corner );
      assert( lines.size() == 2 );
      cbb.insert( cbb.end(), lines.begin(), lines.end() );

      // Save the new last CBB point:
      prevc_cbb_point = deadmbb.getCorner( next_corner );

    } else {
      /* Corner is clipped using multiple clipping point */

      // sort the clip points so that they are accessed in CW order:
      std::sort( corner_dpoints.begin(), corner_dpoints.end(), CWFunctor(corner) );

      if (c_idx == 0) {
        assert( corner == 0 && prev_corner == 1 && oppo_corner == 3 );
        Box deadmbb( corner_dpoints.front(), b.getCorner(corner) );
        first_cbb_point = deadmbb.getCorner( prev_corner );

        // Extra line needed to connect the 1st clip point to the (outer) MBB:
        cbb.push_back( Line(first_cbb_point, corner_dpoints.front()) );
      } else {
        // connect with the previous last CBB point
        Box dead_first( corner_dpoints.front(), b.getCorner(corner) );
        cbb.push_back( Line(dead_first.getCorner(prev_corner), prevc_cbb_point) );

        // Extra line needed to connect the 1st clip point to the (outer) MBB:
        cbb.push_back(
            Line(dead_first.getCorner(prev_corner), corner_dpoints.front())
        );
      }
      if (c_idx == NUM_CORNERS - 1) {
        assert( corner == 1 && prev_corner == 3 && oppo_corner == 2 );
        Box deadmbb( corner_dpoints.back(), b.getCorner(corner) );
        cbb.push_back( Line(first_cbb_point, deadmbb.getCorner(next_corner)) );
      }

      for (uint32_t j = 1; j < corner_dpoints.size(); ++j) {
        Box innermbb( corner_dpoints[j-1], corner_dpoints[j] );
        std::vector<Line> lines = innermbb.getEdges(corner);
        assert( lines.size() == 2);
        cbb.insert( cbb.end(), lines.begin(), lines.end() );
      }

      // And save the new last CBB point:
      Box dead_last( corner_dpoints.back(), b.getCorner(corner) );
      prevc_cbb_point = dead_last.getCorner( next_corner );

      // Special case: the last clip point of the corner needs also to
      // connected to the outer MBB:
      cbb.push_back( Line(corner_dpoints.back(), prevc_cbb_point) );

    } // END of * Corner is clipped using multiple clipping point *
  } // END OF NUM_CORNERS

  return cbb;
}

std::vector<Line> Utils::box2lines3D( const Box &b ) {
  assert( NUM_DIMS == 3 );

  std::vector<Line> lines;
  Point v0, v1;

  v0 = b.lo;
  v1 = v0;
  v1[1] = b.hi[1];
  lines.push_back( std::pair<Point, Point>(v0, v1) );
  v1 = v0;
  v1[2] = b.hi[2];
  lines.push_back( std::pair<Point, Point>(v0, v1) );
  v1 = v0;
  v1[0] = b.hi[0];
  lines.push_back( std::pair<Point, Point>(v0, v1) );

  v0 = b.hi;
  v1 = v0;
  v1[1] = b.lo[1];
  lines.push_back( std::pair<Point, Point>(v0, v1) );
  v1 = v0;
  v1[2] = b.lo[2];
  lines.push_back( std::pair<Point, Point>(v0, v1) );
  v1 = v0;
  v1[0] = b.lo[0];
  lines.push_back( std::pair<Point, Point>(v0, v1) );

  v0 = b.lo;
  v0[1] = b.hi[1];
  v1 = b.hi;
  v1[0] = b.lo[0];
  lines.push_back( std::pair<Point, Point>(v0, v1) );
  v1 = b.hi;
  v1[2] = b.lo[2];
  lines.push_back( std::pair<Point, Point>(v0, v1) );

  v0 = b.lo;
  v0[2] = b.hi[2];
  v1 = b.hi;
  v1[0] = b.lo[0];
  lines.push_back( std::pair<Point, Point>(v0, v1) );
  v1 = b.hi;
  v1[1] = b.lo[1];
  lines.push_back( std::pair<Point, Point>(v0, v1) );

  v0 = b.lo;
  v0[0] = b.hi[0];
  v1 = b.hi;
  v1[2] = b.lo[2];
  lines.push_back( std::pair<Point, Point>(v0, v1) );
  v1 = b.hi;
  v1[1] = b.lo[1];
  lines.push_back( std::pair<Point, Point>(v0, v1) );

  assert( lines.size() == 12 );

  return lines;
}

std::vector<Line> Utils::box2linesExceptCorner3D( const Box &b, const Point &corner ) {

  assert( NUM_DIMS == 3 );

  std::vector<Line> lines = box2lines3D( b );

  std::vector<Line>::iterator it;
  for ( it = lines.begin(); it != lines.end(); ) {
    if ( it->first == corner || it->second == corner )
      it = lines.erase( it );
    else
      it++;
  }

  return lines;
}

std::vector<Line> Utils::cbb2lines3D( const Box &b,
    const std::vector<ClipPoint> &dpoints, const uint32_t topk ) {
  assert( NUM_DIMS == 3 );
  assert( topk <= dpoints.size() );
//  int myorder[] = { 1, 3, 7, 0, 2, 3, 5, 6 }; // order for node #5380 in RSS axo03
//  int myorder[] = { 0, 4, 5, 1, 2, 3 }; // order for node #0 in RSS test_data_3d

  std::vector<Line> cbb = box2lines3D( b );
  std::set<Point> clipped_corners;
  std::pair<std::set<Point>::iterator,bool> ret;

  std::vector<Line> new_lines;
  for (uint32_t k = 0; k < topk; ++k) {
    const ClipPoint &cp = dpoints[ k ];
//    const ClipPoint &cp = dpoints[ myorder[k] ];
    for (auto &l : cbb) {
      if ( l.first == cp.ccoord || l.second == cp.ccoord ) {

        if ( clipped_corners.find( cp.ccoord ) == clipped_corners.end() ) {

          vector<Line> lines_at_cp = box2linesExceptCorner3D( cp.toMBR(), cp.ccoord );
          new_lines.insert( new_lines.end(), lines_at_cp.begin(), lines_at_cp.end() );

          ret = clipped_corners.insert( cp.ccoord );
          assert( ret.second );

        }

        // Find which coordinate needs to be shifted:
        int to_shift = 0;
        for (uint32_t d = 0; d < NUM_DIMS; ++d) {
          if ( l.first[d] != l.second[d] )
            to_shift = d;
        }

        if ( l.first == cp.ccoord )
          l.first[to_shift] = cp.dpoint[to_shift];
        else
          l.second[to_shift] = cp.dpoint[to_shift];

        //      VisUtils::plotLine( l, "vis/test_/CBB_line", BLUE_COLOR );

        //      for (uint32_t i = 0; i < 3; ++i) {
        //        Point p = cp.dpoint;
        //        p[i] = cp.ccoord[i];
        //        cbb.push_back( std::pair<Point, Point>( cp.dpoint, p) );
        //      }
      }
    }
  }

  std::vector<Line>::iterator it;
  for (auto cc : clipped_corners)
    for ( it = cbb.begin(); it != cbb.end(); ) {
      if ( it->first == cc || it->second == cc )
        it = cbb.erase( it );
      else
        it++;
  }

  cbb.insert(cbb.end(), new_lines.begin(), new_lines.end() );

//  VisUtils::plotLines( cbb, "vis/test_/CBB", RED_COLOR );

  return cbb;
}

/*
 * Compute pixel-based data coverage in 2D, i.e., approximate area covered by
 * all node's entries.
 *
 * The accuracy depends on the macro PIXELS_PER_DIMENSIONS (usually at least
 * 100 pixels per dimension are dedicated so that accuracy is high).
 *
 * The computed area is stored in node.pixel_coverage_abs
 */
void Utils::computePixelCoverage2D( NodeInfo &node, bool verbose ) {
  if ( node.mbr.volume() == 0.0 ) {
    assert(false);
    return;
  }

  const std::vector<Box> &entries = node.entries;

  const Point diff = node.mbr.lo.distanceTo(node.mbr.hi);
  int32_t bounds[NUM_DIMS];
  double max_side = diff[0];
  for (uint32_t d = 1; d < NUM_DIMS; ++d) {
    max_side = std::max(max_side, diff[d]);
  }
//  printf("BOUNDS: ");
  uint64_t num_total_pixels = 1;
  for (uint32_t d = 0; d < NUM_DIMS; ++d) {
    bounds[d] = std::ceil( (diff[d] * PIXELS_PER_DIMENSIONS) / max_side );
    num_total_pixels *= bounds[d];
//    printf( "%u ", bounds[d] );
  }
//  printf("\n");

  assert( num_total_pixels > 0 );

  char *vol_space = new char[num_total_pixels];
  memset(vol_space, 0, num_total_pixels);

  for (uint32_t e = 0; e < entries.size(); ++e) {
    int32_t from[NUM_DIMS];
    int32_t to[NUM_DIMS];

    for (uint32_t d = 0; d < NUM_DIMS; ++d) {
      const double rel_lo = entries[e].lo[d] - node.mbr.lo[d];
      const double rel_hi = entries[e].hi[d] - node.mbr.lo[d];
      from[d] = std::floor( (rel_lo / diff[d]) * bounds[d] );
      to[d] = std::floor( (rel_hi / diff[d]) * bounds[d] );

      if ( from[d] == to[d] && from[d] != bounds[d] )
        to[d] += 1;

//      from[d] = std::min( from[d], bounds[d] - 1 );
//      to[d] = std::min( to[d], bounds[d] - 1 );

      if ( !( from[d] <= bounds[d] && to[d] <= bounds[d] && from[d] <= to[d] ) )
        printf( "ERROR: exceeding bounds[0,%u]: from %u to %d\n", bounds[d], from[d], to[d] );

//      printf(" [%u; %u) ", from[d], to[d]);
    }
//    printf("\n");

    for (int32_t d2 = from[1]; d2 < to[1]; ++d2) {
      for (int32_t d1 = from[0]; d1 < to[0]; ++d1) {
        const uint32_t addr = d2*bounds[0] + d1;
        assert( addr < num_total_pixels );
        vol_space[addr] = vol_space[addr] + 1;
        assert( vol_space[addr] < numeric_limits<char>::max() );
      }
    }

  }

  uint64_t num_covered_pixels = 0, num_overlapping_pixels = 0;
  for (int32_t d2 = bounds[1] - 1; d2 >= 0; --d2) {
    for (int32_t d1 = 0; d1 < bounds[0]; ++d1) {
      const uint32_t addr = d2*bounds[0] + d1;
      assert( addr < num_total_pixels );

      if ( vol_space[addr] > 0 )
        ++num_covered_pixels;

      if ( vol_space[addr] > 1 )
        ++num_overlapping_pixels;
    }
  }
  node.pixel_coverage_abs = node.mbr.volume() * num_covered_pixels / num_total_pixels;
  node.pixel_overlap_abs = node.mbr.volume() * num_overlapping_pixels / num_total_pixels;

//  if (verbose) {
//    if ( node.level > 0 ) {
//      printf( " '-> Total deadpsace: %f / %f = %.3f %%\n",
//          node.total_deadspace_abs, node.mbr.volume(),
//          node.total_deadspace_abs / node.mbr.volume() *100.0 );
//      printf( " '-> Total overlap: %f / %f = %.3f %%\n",
//          node.total_overlap_abs, node.mbr.volume(),
//          node.total_overlap_abs / node.mbr.volume() *100.0 );
//    }
//
//    // (Textual) visualization
//    for (int32_t d2 = bounds[1] - 1; d2 >= 0; --d2) {
//      for (int32_t d1 = 0; d1 < bounds[0]; ++d1) {
//        const uint32_t addr = d2*bounds[0] + d1;
//        assert( addr < num_total_pixels );
//        printf( "%u ", vol_space[addr] );
//      }
//      printf("\n");
//    }
//    // NodeVisualizer::vis( node );
//  }

  delete[] vol_space;
}


void Utils::computeExact2DArea( NodeInfo &node, bool verbose )
{
	node.exact_coverage_abs = Box::totalArea2d( node.entries );
}

Circle Utils::circleDefinedByUpToThreePoints( const PointSet& points )
{
	switch( points.size() )
	{
		case 3:
		{
			// three boundary points identify a unique circle
			// radius is distance from centre point to any of the original boundary points.
			Point centreOfCircle( points[ 0 ], points[ 1 ], points[ 2 ] );
			return Circle( 
				Point::euclidean_distance( points[ 0 ], centreOfCircle ), centreOfCircle 
			);
		}
		case 2:
		{
			// two points form diameter of circle
			// centre of circle is midpoint of line segment between them; radius is half the 
			// length of the line segment.
			const Point centreOfCircle = Point( points[ 0 ], points[ 1 ], 0.5f );
			const double radius = Point::euclidean_distance( points[ 0 ], centreOfCircle );
			return Circle( radius, centreOfCircle );
		}
		case 1:
		{
			// degenerate solution: circle is a single point with radius zero.
			return Circle( 0, points.back() );
		}
		case 0:
		{
			// degenerate solution: 
			// you can't enclose nothingness; it's a philosophical impossibility
			// but a negative radius ensures nothing is within that circle, not even a point 
			// (as long as doesn't compare to the square of the radius, that is!)
			return Circle( -1, Point{ 0, 0 } );
		}
		default:
		{
			// unless the points are all co-circular, this case is invalid 
			// and the Welzl algorithm will never invoke it.
			// Just return the result of the first three points and pray that none 
			// are coincident.
			assert( false );
			return circleDefinedByUpToThreePoints( 
				PointSet( points.begin(), points.begin() + 3 )
			);
		}
	}
}

Circle Utils::b_minidisk( PointSet& P, PointSet& R )
{
	if( P.size() == 0 || R.size() == 3 )
	{
		return circleDefinedByUpToThreePoints( R );
	}
	else if( R.size() == 0 && P.size() == 1 ) 
	{
		// Welzl should have included this base case to save a horrendous 
		// number of unnecessary (q) recursive calls.
		// Can't stop here at |P|=2 'though in case the points are coincident.
		return circleDefinedByUpToThreePoints( P );
	}
	else if( R.size() == 0 && P.size() == 2 && P[ 0 ] != P[ 1 ] )
	{
		return circleDefinedByUpToThreePoints( P );
	}
	else
	{
		const Point& p = P.back();
		P.pop_back();
		Circle c = b_minidisk( P, R );
		if( Point::euclidean_distance_squared( p, c.second ) > c.first * c.first 
			|| c.first < 0 // since we're comparing to the square of the radius.
		) 
		{
			// p is not contained in nor on boundary of Circle c
			R.push_back( p );
			c = b_minidisk( P, R );
			R.pop_back();
		}
		P.push_back( p );
		return c;
	}
}


// Implementing algorithm from https://dx.doi.org/10.1007%2FBFb0038202
// Expected linear time performance if points is randomly shuffled
//
// should be pass-by-value because we are shuffling the points; however, 
// this is currently only invoked on a throw-away PointSet anyway, so using 
// pass-by-reference for efficiency. Should be changed if used elsewhere.
Circle Utils::welzl91Algorithm( PointSet& P )
{
	static auto rand_engine = std::default_random_engine{};
	std::shuffle( P.begin(), P.end(), rand_engine );
	
	PointSet R;
	R.reserve( 3 );
	return b_minidisk( P, R );
}

Circle Utils::computeRadiusOfMinEnclosingCircle( const std::vector< Box >& rectangles )
{
	// Copy and randomly shuffle all the corner points of all the rectangles.
	PointSet corners;
	populateBoxCorners( corners, rectangles );
	
	// Find min enclosing circle.
	const Circle min_enclosing_circle = welzl91Algorithm( corners );
	return min_enclosing_circle;
}

/**
 * @pre Assumes that the exact coverage has already been calculated.
 */
void Utils::computeSphericalDeadspace2D( NodeInfo &node, bool verbose ) 
{

  if ( node.mbr.volume() <= 0.0 ) {
    assert(false);
    return;
  }

	// compute minimum bounding circle
	const Circle circle = computeRadiusOfMinEnclosingCircle( node.entries );
	node.area_min_enclosing_circle_abs = pi * circle.first * circle.first;
	node.spherical_dead_space = node.area_min_enclosing_circle_abs - node.exact_coverage_abs;
	approximateCircleWithPolygon2D(circle, 36, node.triacontakaihexagon );
}

/*
 * Approximates circle with n-sided polygon.
 *
 * Exploits Equation of the Circle: http://stackoverflow.com/a/839931/246275
 */
void Utils::approximateCircleWithPolygon2D( const Circle &c,
    const int n_edges, std::vector<Point> &poly ) {

  // Compute the angle between two consecutive polygon edges
  // originating from the circle center:
  const double angle = 2 * pi / n_edges;

  for (int i = 1; i <= n_edges; ++i ) {
    const Point p = {
        c.second[0] + c.first * std::cos(angle * i),
        c.second[1] + c.first * std::sin(angle * i)
    };
    poly.push_back( p );
  }

  assert( poly.size() == n_edges );
}

void Utils::populateBoxCorners( 
	std::vector< Point >& corners, const std::vector< Box > boxes )
{ 
	corners.reserve( 4 * boxes.size() );
  std::for_each( boxes.begin(), boxes.end(), 
  	[ &corners ]( const Box& b )
  	{
			if( b.volume() > 0 )
			{
				corners.push_back( Point( b.lo ) );
				corners.push_back( Point( b.hi ) );
				corners.push_back( Point{ b.lo[ 0 ], b.hi[ 1 ] } );
				corners.push_back( Point{ b.hi[ 0 ], b.lo[ 1 ] } );
			}
			else // degenerate box (i.e., point): just add once.
			{
				corners.push_back( Point( b.lo ) );
			}
		}
  );
  
  // eliminate all the duplicates
  std::sort( corners.begin(), corners.end() );
  corners.resize( std::distance( 
  	corners.begin(), std::unique( corners.begin(), corners.end() ) 
  ) );
}

/**
 * @pre Assumes that the exact coverage has already been calculated.
 */
void Utils::computeConvexHullAndFriends2D( NodeInfo &node, bool verbose ) 
{

  if ( node.mbr.volume() <= 0.0 ) {
    assert(false);
    return;
  }
  
  std::vector< Point > corners;
	populateBoxCorners( corners, node.entries );
	
	node.ch_size = Point::convexHull( corners );
	corners.resize( node.ch_size );
	node.convex_hull_area = Point::areaOfPolygon( corners );
	node.ch_vertices = corners;
	auto rotated_mbr = Point::minimumRotatedMBR( corners );
	node.area_min_rotated_mbr_abs = rotated_mbr.first;
	node.rotated_mbr = rotated_mbr.second;
	auto four_corners = Point::fourCorners( corners );
	node.area_four_corners = four_corners.first;
	node.four_corners = four_corners.second;
	auto five_corners = Point::fiveCorners( corners );
	node.area_five_corners = five_corners.first;
	node.five_corners = five_corners.second;
}

void Utils::computeExactClippedVol2D( NodeInfo &node, bool verbose ) {
  vector<Box> clipped_mbbs;

  uint32_t e = 0;
  for (e = 0; e < node.clip_points.size(); ++e) {
    clipped_mbbs.push_back( Box(node.clip_points[e].toMBR()) );

    const double clipped_vol = Box::totalArea2d( clipped_mbbs );
    node.accum_clipped_abs[e] = ( clipped_vol );
  }

  for (; e < node.max_cpoints; ++e)
    if ( e > 0 )
      node.accum_clipped_abs[e] = node.accum_clipped_abs[e - 1];
    else
      assert( node.accum_clipped_abs[e] == 0.0 );

}

/*
 * Compute pixel-based (absolute and approximate) area of clipped 2D boxes.
 *
 * It has to be run only after node has been 'clipped' (i.e., after computeCBB()
 * is called)
 *
 * The accuracy depends on the macro PIXELS_PER_DIMENSIONS (usually at least
 * 100 pixels per dimension are dedicated so that accuracy is high).
 *
 * The accumulative clipped area (by each clip) point is stored in vector
 * node.accum_clipped_abs.
 */
void Utils::computePixelClippedDeadspace2D( NodeInfo &node, bool verbose ) {
  if ( node.mbr.volume() == 0.0 ) {
    assert(false);
    return;
  }

  const Point diff = node.mbr.lo.distanceTo(node.mbr.hi);
  int32_t bounds[NUM_DIMS];
  double max_side = diff[0];
  for (uint32_t d = 1; d < NUM_DIMS; ++d) {
    max_side = std::max(max_side, diff[d]);
  }
//  printf("BOUNDS: ");
  uint64_t num_total_pixels = 1;
  for (uint32_t d = 0; d < NUM_DIMS; ++d) {
    bounds[d] = std::ceil( (diff[d] * PIXELS_PER_DIMENSIONS) / max_side );
    num_total_pixels *= bounds[d];
//    printf( "%u ", bounds[d] );
  }
//  printf("\n");

  assert( num_total_pixels > 0 );

  char *vol_space = new char[num_total_pixels];
  memset(vol_space, 0, num_total_pixels);

  uint32_t e = 0;
  for (e = 0; e < node.clip_points.size(); ++e) {
    int32_t from[NUM_DIMS];
    int32_t to[NUM_DIMS];

    const Box deadmbr = node.clip_points[e].toMBR( );
    for (uint32_t d = 0; d < NUM_DIMS; ++d) {
      const double rel_lo = deadmbr.lo[d] - node.mbr.lo[d];
      const double rel_hi = deadmbr.hi[d] - node.mbr.lo[d];
      from[d] = std::floor( (rel_lo / diff[d]) * bounds[d] );
      to[d] = std::floor( (rel_hi / diff[d]) * bounds[d] );

      if ( from[d] == to[d] && from[d] != bounds[d] )
        to[d] += 1;

//      from[d] = std::min( from[d], bounds[d] - 1 );
//      to[d] = std::min( to[d], bounds[d] - 1 );

      if ( !( from[d] <= bounds[d] && to[d] <= bounds[d] && from[d] <= to[d] ) )
        printf( "ERROR: exceeding bounds[0,%u]: from %u to %d\n", bounds[d], from[d], to[d] );

//      printf(" [%u; %u) ", from[d], to[d]);
    }
//    printf("\n");

    for (int32_t d2 = from[1]; d2 < to[1]; ++d2) {
      for (int32_t d1 = from[0]; d1 < to[0]; ++d1) {
        const uint32_t addr = d2*bounds[0] + d1;
        assert( addr < num_total_pixels );
        vol_space[addr] = vol_space[addr] + 1;
        assert( vol_space[addr] < numeric_limits<char>::max() );
      }
    }

    uint64_t num_clipped_pixels = 0, num_overlapping_pixels = 0;
    for (int32_t d2 = bounds[1] - 1; d2 >= 0; --d2) {
      for (int32_t d1 = 0; d1 < bounds[0]; ++d1) {
        const uint32_t addr = d2*bounds[0] + d1;
        assert( addr < num_total_pixels );

        if ( vol_space[addr] != 0 )
          ++num_clipped_pixels;
        if ( vol_space[addr] > 1 )
          ++num_overlapping_pixels;
      }
    }

    assert( e < node.accum_clipped_abs.size() );
    node.accum_clipped_abs[e] = ( node.mbr.volume() * num_clipped_pixels / num_total_pixels );
    node.accum_abs_chamfered_overlap[e] =  node.mbr.volume() * num_overlapping_pixels / num_total_pixels;

  }  // END OF for each deadly point

  for (; e < node.max_cpoints; ++e)
	  if ( e > 0 )
		  node.accum_clipped_abs[e] = node.accum_clipped_abs[e - 1];
	  else
		  assert( node.accum_clipped_abs[e] == 0.0 );


//  if (verbose) {
//    if ( node.level > 0 ) {
//  	  printf( " '-> Total pruned deadpsace: %f / %f / %f = %.3f %%\n",
//  			  node.accum_abs_chamfered.back(), node.total_deadspace_abs,
//  			  node.mbr.volume(),
//  			  node.accum_abs_chamfered.back() / node.mbr.volume() *100.0 );
//  	  printf( " '-> Total pruned overlap: %f / %f = %.3f %%\n",
//  			  node.accum_abs_chamfered_overlap.back(), node.mbr.volume(),
//  			  node.accum_abs_chamfered_overlap.back() / node.mbr.volume() *100.0 );
//    }
//
//    // (Textual) visualization
//    for (int32_t d2 = bounds[1] - 1; d2 >= 0; --d2) {
//      for (int32_t d1 = 0; d1 < bounds[0]; ++d1) {
//        const uint32_t addr = d2*bounds[0] + d1;
//        assert( addr < num_total_pixels );
//        printf( "%u ", vol_space[addr] );
//      }
//      printf("\n");
//    }
//    // NodeVisualizer::vis( node );
//  }

  delete[] vol_space;
}

/*
 * Compute pixel-based data coverage in 3D, i.e., approximate volume covered by
 * all node's entries.
 *
 * The accuracy depends on the macro PIXELS_PER_DIMENSIONS (usually at least
 * 100 voxels per dimension are dedicated so that accuracy is high).
 *
 * The computed volume is stored in node.pixel_coverage_abs
 */
void Utils::computePixelCoverage3D( NodeInfo &node, bool verbose ) {
  if ( node.mbr.volume() == 0.0 ) {
    assert(false);
    return;
  }

  const std::vector<Box> &entries = node.entries;

  const Point diff = node.mbr.lo.distanceTo(node.mbr.hi);
  Point cell_size;
  int32_t grid_rez[3];
  double max_side = diff[0];
  for (uint32_t d = 1; d < 3; ++d) {
    max_side = std::max(max_side, diff[d]);
  }
//  printf("BOUNDS: ");
  uint64_t num_total_pixels = 1;
  for (uint32_t d = 0; d < 3; ++d) {
    grid_rez[d] = std::ceil( (diff[d] * PIXELS_PER_DIMENSIONS) / max_side );
    num_total_pixels *= grid_rez[d];
    cell_size[d] = diff[d] / grid_rez[d];
//    printf( "%u ", bounds[d] );
  }
//  printf("\n");

  assert( num_total_pixels > 0 );

  char *vol_space = new char[num_total_pixels];
  memset(vol_space, 0, num_total_pixels);

  for (uint32_t e = 0; e < entries.size(); ++e) {
    int32_t from[3];
    int32_t to[3];

    for (uint32_t d = 0; d < 3; ++d) {
      const double rel_lo = entries[e].lo[d] - node.mbr.lo[d];
      const double rel_hi = entries[e].hi[d] - node.mbr.lo[d];
      from[d] = std::floor( (rel_lo / diff[d]) * grid_rez[d] );
      to[d] = std::floor( (rel_hi / diff[d]) * grid_rez[d] );

      if ( from[d] == to[d] && from[d] != grid_rez[d] )
        to[d] += 1;

      if ( !( from[d] <= grid_rez[d] && to[d] <= grid_rez[d] && from[d] <= to[d] ) )
        printf( "ERROR: exceeding bounds[0,%u]: from %u to %d\n", grid_rez[d], from[d], to[d] );

//      printf(" [%u; %u) ", from[d], to[d]);
    }
//    printf("\n");

    for (int32_t z = from[2]; z < to[2]; ++z) {
      for (int32_t y = from[1]; y < to[1]; ++y) {
        for (int32_t x = from[0]; x < to[0]; ++x) {
          const uint32_t addr = (z * grid_rez[1]*grid_rez[0]) + y * grid_rez[0] + x;
          assert( addr < num_total_pixels );
          vol_space[addr] = vol_space[addr] + 1;
        }
      }
    }

  }

  uint64_t num_covered_pixels = 0, num_overlapping_pixels = 0;
  for (int32_t z = 0; z < grid_rez[2]; ++z) {
    for (int32_t y = 0; y < grid_rez[1]; ++y) {
      for (int32_t x = 0; x < grid_rez[0]; ++x) {
        const uint32_t addr = (z * grid_rez[1]*grid_rez[0]) + y * grid_rez[0] + x;
        assert( addr < num_total_pixels );

        if ( vol_space[addr] > 0 ) {
          ++num_covered_pixels;

          // for voxel visualization of actual spatial data:
          if (NUM_DIMS == 3) {
            double x1 = node.mbr.lo[0] + x * cell_size[0];
            double x2 = node.mbr.lo[0] + (x+1) * cell_size[0];
            double y1 = node.mbr.lo[1] + y * cell_size[1];
            double y2 = node.mbr.lo[1] + (y+1) * cell_size[1];
            double z1 = node.mbr.lo[2] + z * cell_size[2];
            double z2 = node.mbr.lo[2] + (z+1) * cell_size[2];

            const Box voxel( Point(std::initializer_list<double>{x1, y1, z1}),
                             Point(std::initializer_list<double>{x2, y2, z2}) );
            node.data_pixels.push_back( voxel );

          }
        }
        if ( vol_space[addr] > 1 )
          ++num_overlapping_pixels;
      }
    }
  }

  node.pixel_coverage_abs = node.mbr.volume() * num_covered_pixels / num_total_pixels;
  node.pixel_overlap_abs = node.mbr.volume() * num_overlapping_pixels / num_total_pixels;

//  if (verbose) {
//    if ( node.level > 0 ) {
//        printf( " '-> Total deadpsace: %f / %f = %.3f %%\n",
//            node.total_deadspace_abs, node.mbr.volume(),
//          node.total_deadspace_abs / node.mbr.volume() *100.0 );
//        printf( " '-> Total overlap: %f / %f = %.3f %%\n",
//            node.total_overlap_abs, node.mbr.volume(),
//          node.total_overlap_abs / node.mbr.volume() *100.0 );
//    }
//
//    // (Textual) visualization
//    for (int32_t y = bounds[1] - 1; y >= 0; --y) {
//      for (int32_t x = 0; x < bounds[0]; ++x) {
//        const uint32_t addr = y*bounds[0] + x;
//        assert( addr < num_total_pixels );
//        printf( "%02u ", vol_space[addr] );
//      }
//      printf("\n");
//    }
//    // NodeVisualizer::vis( node );
//  }

  delete[] vol_space;
}

/*
 * Compute pixel-based (absolute and approximate) volume of clipped 3D boxes.
 *
 * It has to be run only after node has been 'clipped' (i.e., after computeCBB()
 * is called)
 *
 * The accuracy depends on the macro PIXELS_PER_DIMENSIONS (usually at least
 * 100 voxels per dimension are dedicated so that accuracy is high).
 *
 * The accumulative clipped volume (by each clip) point is stored in vector
 * node.accum_clipped_abs.
 */
void Utils::computePixelClippedDeadspace3D( NodeInfo &node, bool verbose ) {
  if ( node.mbr.volume() == 0.0 ) {
    assert(false);
    return;
  }

  const Point diff = node.mbr.lo.distanceTo(node.mbr.hi);
  int32_t bounds[3];
  double max_side = diff[0];
  for (uint32_t d = 1; d < 3; ++d) {
    max_side = std::max(max_side, diff[d]);
  }
//  printf("BOUNDS: ");
  uint64_t num_total_pixels = 1;
  for (uint32_t d = 0; d < 3; ++d) {
    bounds[d] = std::ceil( (diff[d] * PIXELS_PER_DIMENSIONS) / max_side );
    num_total_pixels *= bounds[d];
//    printf( "%u ", bounds[d] );
  }
//  printf("\n");

  assert( num_total_pixels > 0 );

  char *vol_space = new char[num_total_pixels];
  memset(vol_space, 0, num_total_pixels);

  uint32_t e = 0;
//  int myorder[] = { 1, 3, 7, 0, 2, 3, 5, 6 }; // order for node #5380 in RSS axo03
  for (e = 0; e < node.clip_points.size(); ++e) {
    int32_t from[NUM_DIMS];
    int32_t to[NUM_DIMS];

    const ClipPoint &cp = node.clip_points[e];
//    const ClipPoint &cp = node.clip_points[myorder[e]];

    const Box deadmbr = cp.toMBR( );
    for (uint32_t d = 0; d < NUM_DIMS; ++d) {
      const double rel_lo = deadmbr.lo[d] - node.mbr.lo[d];
      const double rel_hi = deadmbr.hi[d] - node.mbr.lo[d];
      from[d] = std::floor( (rel_lo / diff[d]) * bounds[d] );
      to[d] = std::floor( (rel_hi / diff[d]) * bounds[d] );

      if ( from[d] == to[d] && from[d] != bounds[d] )
        to[d] += 1;

      if ( !( from[d] <= bounds[d] && to[d] <= bounds[d] && from[d] <= to[d] ) )
        printf( "ERROR: exceeding bounds[0,%u]: from %u to %d\n", bounds[d], from[d], to[d] );

//      printf(" [%u; %u) ", from[d], to[d]);
    }
//    printf("\n");

    for (int32_t z = from[2]; z < to[2]; ++z) {
      for (int32_t y = from[1]; y < to[1]; ++y) {
        for (int32_t x = from[0]; x < to[0]; ++x) {
          const uint32_t addr = (z * bounds[1]*bounds[0]) + y * bounds[0] + x;
          assert( addr < num_total_pixels );
          vol_space[addr] = vol_space[addr] + 1;
        }
      }
    }

    uint64_t num_dead_pixels = 0, num_overlapping_pixels = 0;
    for (int32_t z = 0; z < bounds[2]; ++z) {
      for (int32_t y = 0; y < bounds[1]; ++y) {
        for (int32_t x = 0; x < bounds[0]; ++x) {
          const uint32_t addr = (z * bounds[1]*bounds[0]) + y * bounds[0] + x;
          assert( addr < num_total_pixels );

          if ( vol_space[addr] != 0 )
            ++num_dead_pixels;
          if ( vol_space[addr] > 1 )
            ++num_overlapping_pixels;
        }
      }
    }
    assert( e < node.accum_clipped_abs.size() );
    node.accum_clipped_abs[e] = node.mbr.volume() * num_dead_pixels / num_total_pixels;
    node.accum_abs_chamfered_overlap[e] = node.mbr.volume() * num_overlapping_pixels / num_total_pixels;

  } // END OF for each deadly point

  for (; e < node.max_cpoints; ++e)
	  if ( e > 0 ) {
		  node.accum_clipped_abs[e] = node.accum_clipped_abs[e - 1];
		  node.accum_abs_chamfered_overlap[e] = node.accum_abs_chamfered_overlap[e - 1];
	  } else {
		  assert( node.accum_clipped_abs[e] == 0.0 );
		  assert( node.accum_abs_chamfered_overlap[e] == 0.0 );
	  }

//  if ( verbose ) {
//    if ( node.level > 0 ) {
//      printf(" '-> Total pruned deadpsace: %f / %f / %f = %.3f %%\n",
//          node.accum_abs_chamfered.back(), node.total_deadspace_abs,
//          node.mbr.volume(),
//          node.accum_abs_chamfered.back() / node.mbr.volume() * 100.0);
//      printf(" '-> Total pruned overlap: %f / %f = %.3f %%\n",
//          node.accum_abs_chamfered_overlap.back(), node.mbr.volume(),
//          node.accum_abs_chamfered_overlap.back() / node.mbr.volume() * 100.0);
//    }
//
//    // (Textual) visualization
//    for (int32_t y = bounds[1] - 1; y >= 0; --y) {
//      for (int32_t x = 0; x < bounds[0]; ++x) {
//        const uint32_t addr = y * bounds[0] + x;
//        assert(addr < num_total_pixels);
//        printf("%02u ", vol_space[addr]);
//      }
//      printf("\n");
//    }
//    // NodeVisualizer::vis( node );
//  }

  delete[] vol_space;
}

