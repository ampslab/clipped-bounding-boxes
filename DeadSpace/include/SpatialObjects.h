/*
 * SpatialObjects.h
 *
 *  Created on: Jan 18, 2016
 *      Author: sidlausk
 */

#ifndef SPATIALOBJECTS_H_
#define SPATIALOBJECTS_H_

#include <cstdio>
#include <string>
#include <vector>
#include <cassert>
#include <bitset>
#include <cmath>

#include <algorithm>	// std::transform
#include <numeric>		// std::accumulate
#include <functional>	// std::bind

#define NUM_DIMS 3
#define NUM_CORNERS (1 << NUM_DIMS) // #sub-spaces

#ifndef NDEBUG
  #define PIXELS_PER_DIMENSIONS 20U
#else
  #define PIXELS_PER_DIMENSIONS 100U
#endif


class Point; // forward declare in order to define PointSet.
using PointPair = std::pair< Point, Point >; // for line segments.
using PointSet = std::vector< Point >;

class Point {
private:
  double coord[NUM_DIMS];
	
	/**
	 * Uses y-coordinate (i.e., coord[ 1 ]) as comparator, 
	 * with the x-coordinate (i.e., coord[ 0 ]) for tie-breaking.
	 */
	static bool smallerY( const Point& p, const Point& q )
	{
		if( q.coord[ 1 ] < p.coord[ 1 ] ) { return false; }
		else if ( p.coord[ 1 ] < q.coord[ 1 ] ) { return true; }
		else { return p.coord[ 0 ] < q.coord[ 0 ]; }
	}
	
	/**
	 * Helper method for Graham's Scan: checks whether [p1,p2,p3] makes 
	 * a counter-clockwise turn.
	 */
	static bool ccw( const Point& p1, const Point& p2, const Point& p3 )
	{
		return ( p2.coord[ 0 ] - p1.coord[ 0 ] ) * ( p3.coord[ 1 ] - p1.coord[ 1 ] ) 
			- ( p2.coord[ 1 ] - p1.coord[ 1 ] ) * ( p3.coord[ 0 ] - p1.coord[ 0 ] ) <= 0;
	}
	
	static double crossProduct( const Point& p, const Point& q )
	{
		return p.coord[ 0 ] * q.coord[ 1 ] - q.coord[ 0 ] * p.coord[ 1 ];
	}
	
	static inline double dotProduct( const Point& p, const Point& q )
	{
		return std::inner_product( p.coord, p.coord + NUM_DIMS, q.coord, 0.0 );
	}
	
	static double vectorMagnitude( const Point& p )
	{
		return std::sqrt( dotProduct( p, p ) );
	}
	
	static double vectorMagnitude( const Point& ref, const Point& p )
	{
		return std::sqrt( 
			std::inner_product( p.coord, p.coord + NUM_DIMS, ref.coord, 0.0, 
				std::plus< double >(), 
				[]( double pval, double refval ){ return ( pval - refval ) * ( pval - refval ); }
			)
		);
	}
	
	static bool smallerPolarAngle( const Point& ref, const Point& p, const Point& q )
	{
		// get unit vectors and compare those
		// first check for coincident points.
		if( p[ 0 ] == q[ 0 ] && p[ 1 ] == q[ 1 ] ) { return false; }
		else if( p[ 0 ] == ref[ 0 ] && p[ 1 ] == ref[ 1 ] ) { return true; }
		else if( q[ 0 ] == ref[ 0 ] && q[ 1 ] == ref[ 1 ] ) { return false; }
		else
		{
			const double pMagnitude = vectorMagnitude( ref, p );
			const double qMagnitude = vectorMagnitude( ref, q );
			const double pNormalisedXVal = 
				( p.coord[ 0 ] - ref.coord[ 0 ] ) / pMagnitude + ref.coord[ 0 ];
			const double qNormalisedXVal = 
				( q.coord[ 0 ] - ref.coord[ 0 ] ) / qMagnitude + ref.coord[ 0 ];
			
			if( pNormalisedXVal == qNormalisedXVal ) 
			{
				return p[ 0 ] > q[ 0 ]; // if same direction, put larger one first
			}
			else
			{
				return pNormalisedXVal > qNormalisedXVal;
			}
		}
	}
	
  
  /**
   * Calculates the area of a minimum enclosing rotated bounding box, 
   * given a set of points and a line segment [p,q] co-oriented with
   * one of the edges of the bounding box.
   *
   * The line segment is interpreted as a vector, and we computes the farthest 
   * points in the point set in the direction of and opposite to that vector: 
   * this gives the "height" of the rectangle. The width is computed identically
   * by using instead the normal vector. By first normalising the vectors to unit 
   * length, we can compute distances quickly as dot products (i.e., the size of
   * the projections).
   */
  template < bool returnCorners >
  static double areaOfRotatedMBR( const PointSet& points, const Point& p, const Point& q,
  	Point& cornerA, Point& cornerB, Point& cornerC, Point& cornerD )
  {
  	// Compute this edge's unit vector and normal vector.
		const Point vectorPQ{ q[ 0 ] - p[ 0 ], q[ 1 ] - p[ 1 ] };
		double vectorPQMagnitude = vectorMagnitude( vectorPQ );
		const Point unitVectorPQ{ 
			vectorPQ[ 0 ] / vectorPQMagnitude, 
			vectorPQ[ 1 ] / vectorPQMagnitude 
		};
		const Point normUnitVectorPQ{ unitVectorPQ[ 1 ], -1.0 * unitVectorPQ[ 0 ] };
		
		// Init the min/max distances with respect to each vector
		double min = std::numeric_limits< double >::max();
		double max = std::numeric_limits< double >::max() * -1;
		double minNorm = std::numeric_limits< double >::max();
		double maxNorm = std::numeric_limits< double >::max() * -1;
		
		// Iterate through the point set, updating the min/max distances 
		// with respect to the vector and its norm.
		std::for_each( points.begin(), points.end(),
			[ &min, &max, &minNorm, &maxNorm, &unitVectorPQ, &normUnitVectorPQ, &p ]
			( const Point& r )
			{
				// transform r into the same vector space.
				const Point pr{ r[ 0 ] - p[ 0 ], r[ 1 ]- p[ 1 ] };
				
				// Calculate size of projections/distance
				// (dot product of p with the unit vector in the direction of P->Q)
				const double projection = dotProduct( pr, unitVectorPQ );
				const double projectionNorm = dotProduct( pr, normUnitVectorPQ );
				
				// Update global min/max values
				min = std::min( min, projection );
				max = std::max( max, projection );
				minNorm = std::min( minNorm, projectionNorm );
				maxNorm = std::max( maxNorm, projectionNorm );
			}
		);
		
		// Finally, return the product of the extents (i.e., area) 
		// and, if appropriate, the corners of the rotated MBR.
		if( returnCorners )
		{
			cornerA[ 0 ] = p[ 0 ] + unitVectorPQ[ 0 ] * min + normUnitVectorPQ[ 0 ] * minNorm;
			cornerA[ 1 ] = p[ 1 ] + unitVectorPQ[ 1 ] * min + normUnitVectorPQ[ 1 ] * minNorm;
			cornerB[ 0 ] = p[ 0 ] + unitVectorPQ[ 0 ] * min + normUnitVectorPQ[ 0 ] * maxNorm;
			cornerB[ 1 ] = p[ 1 ] + unitVectorPQ[ 1 ] * min + normUnitVectorPQ[ 1 ] * maxNorm;
			cornerC[ 0 ] = p[ 0 ] + unitVectorPQ[ 0 ] * max + normUnitVectorPQ[ 0 ] * maxNorm;
			cornerC[ 1 ] = p[ 1 ] + unitVectorPQ[ 1 ] * max + normUnitVectorPQ[ 1 ] * maxNorm;
			cornerD[ 0 ] = p[ 0 ] + unitVectorPQ[ 0 ] * max + normUnitVectorPQ[ 0 ] * minNorm;
			cornerD[ 1 ] = p[ 1 ] + unitVectorPQ[ 1 ] * max + normUnitVectorPQ[ 1 ] * minNorm;
		}
		return ( max - min ) * ( maxNorm - minNorm );
  }
  
	
public:

  Point() {
    for (uint32_t d = 0; d < NUM_DIMS; ++d) coord[d] = 0;
  }

  Point(double *coords) {
    for (uint32_t d = 0; d < NUM_DIMS; ++d) coord[d] = coords[d];
  }

  Point( std::initializer_list< double > initial_values ) {
  	assert( initial_values.size() <= NUM_DIMS );
    std::copy( initial_values.begin(), initial_values.end(), coord );
  }
  
  // copy constructor.
  Point( const Point& original )
  {
  	std::copy( original.coord, original.coord + NUM_DIMS, coord );
  }
  // move constructor
  Point( Point&& other )
  {
  	swap( *this, other );
  }
  
  /**
   * Creates a new point in between p and q at a given percentage of the 
   * distance away from p. (E.g., if percentage = 0.5, this is the midpoint 
   * of the line [p, q]).
   */
	Point( const Point& p, const Point& q, const float percentage )
  {
  	std::transform( p.coord, p.coord + NUM_DIMS, q.coord, this->coord,
  		[ percentage ]( const double pval, const double qval )
  		{
				return ( pval + qval ) * percentage;
  		}
  	);
  }
  
  /**
   * Creates a new point at the centre of three points 
   * (i.e., the center of the minimum enclosing circle).
   *
   * This is the same as finding the circumcircle of triangle (a,b,c), 
   * which we do by intersecting the perpendicular bisectors of
   * line segments [a,b] and [a,c].
   */
	Point( Point a, Point b, Point c )
  {
  	assert( NUM_DIMS == 2 ); // this is only valid for 2d points.
  	
  	// check for simple special case where slope = 0
  	// without this check, will get divide by zero errors.
  	bool vert_solved = false;
  	if( a[ 0 ] == c[ 0 ] )
  	{
  		swap( b, c ); // simplify logic by assuming horiz match is on b
  	}
  	if( a[ 0 ] == b[ 0 ] )
  	{
  		coord[ 1 ] = ( a[ 1 ] + b[ 1 ] ) / 2.0;
  		vert_solved = true;
  	}
  	
  	// check for vertical special case (slope = inf)
  	// without this check, answer will be inf/nan.
  	bool horiz_solved = false;
  	if( a[ 1 ] == b[ 1 ] )
  	{
  		if( vert_solved ) // a and b are coincident.
  		{
  			*this = Point( a, c, 0.5 );
  			return;
  		}
  		swap( b, c ); // simplify logic by assuming vert match is on c
  	}
  	if( a[ 1 ] == c[ 1 ] )
  	{
  		coord[ 0 ] = ( a[ 0 ] + c[ 0 ] ) / 2.0;
  		horiz_solved = true;
  	}
  	if( horiz_solved && vert_solved ) { return; } // That was easy! :D
  	
  	
		// calculate midpoints of line segments between point A and B and point A and C 
		// and also the perpendicular bisectors of those line segments
		const double line_seg_ab_xval = ( b.coord[ 0 ] + a.coord[ 0 ] ) / 2;
		const double line_seg_ac_xval = ( c.coord[ 0 ] + a.coord[ 0 ] ) / 2;
		const double line_seg_ab_yval = ( b.coord[ 1 ] + a.coord[ 1 ] ) / 2;
		const double line_seg_ac_yval = ( c.coord[ 1 ] + a.coord[ 1 ] ) / 2;

		const double perp_bisect_ab_xval = ( b.coord[ 1 ] - a.coord[ 1 ] );
		const double perp_bisect_ab_yval = ( a.coord[ 0 ] - b.coord[ 0 ] );
		//const double ab_bisector_slope = perp_bisect_ab_yval / perp_bisect_ab_xval; <- unused
		const double perp_bisect_ac_xval = ( c.coord[ 1 ] - a.coord[ 1 ] );
		const double perp_bisect_ac_yval = ( a.coord[ 0 ] - c.coord[ 0 ] );
		const double ac_bisector_slope = perp_bisect_ac_yval / perp_bisect_ac_xval;
		
		if( horiz_solved )
		{
			// if horiz solved, just find where ab's perpendicular bisector intersects x-val.
			const double distance = ( coord[ 0 ] - line_seg_ab_xval ) / perp_bisect_ab_xval;
			coord[ 1 ] = line_seg_ab_yval + distance * perp_bisect_ab_yval;
		}
		else if( vert_solved )
		{
			// as above, but with ac's perpendicular bisector intersecting y-val
			const double distance = ( coord[ 1 ] - line_seg_ac_yval ) / perp_bisect_ac_yval;
			coord[ 0 ] = line_seg_ac_xval + distance * perp_bisect_ac_xval;
		}
		else
		{
			// find distance from centre point to midpoint of line between first two points
			// as first solution to system of equations: 
			// AB-mid.x + distance * AB-bisect-vector = AC-mid.x + distance' * AC-bisect-vec
			// AB-mid.y + distance * AB-bisect-vector = AC-mid.y + distance' * AC-bisect-vec
			// by rearranging the latter to solve for distance' and plugging that into the 
			// first equation, which is then solved for distance.
			const double distance = 
			( 
				ac_bisector_slope * ( line_seg_ab_xval - line_seg_ac_xval ) 
					+ line_seg_ac_yval - line_seg_ab_yval 
			) 
			/ ( perp_bisect_ab_yval - ac_bisector_slope * perp_bisect_ab_xval );
		
			// finally, advance the calculated distance in the direction of the 
			// perpendicular bisector from the midpoint of the line segment AB to recover 
			// the centerpoint of the minimum enclosing circle. The radius of the circle 
			// is the distance between that centerpoint and any of the boundary points.
			coord[ 0 ] = line_seg_ab_xval + distance * perp_bisect_ab_xval;
			coord[ 1 ] = line_seg_ab_yval + distance * perp_bisect_ab_yval;
		}
  }

  inline bool operator<(const Point &rhs) const {
    for (uint32_t d = 0; d < NUM_DIMS; ++d) {
      if ( coord[d] == rhs[d] ) {
        continue;
      }
      return coord[d] < rhs[d];
    }
    return false; // points are equal
  }
  inline bool operator==(const Point &rhs) const {
    for (uint32_t d = 0; d < NUM_DIMS; ++d)
      if (coord[d] != rhs[d])
        return false;

    return true;
  }
  inline bool operator!=( const Point &rhs ) const {
    return !operator==( rhs );
  }

  inline double& operator[](const size_t idx) {
    assert(idx >= 0 && idx < NUM_DIMS);
    return coord[idx];
  };

  inline const double& operator[](const size_t idx) const {
    assert(idx >= 0 && idx < NUM_DIMS);
    return coord[idx];
  }
  
  static void swap( Point& first, Point& second )
  {
  	std::swap( first.coord, second.coord );
  }
  Point& operator=(const Point& other)
	{
			Point temp( other );
			swap( *this, temp );

			return *this;
	}

  // The absolute difference in each dimension to point p
  Point distanceTo(const Point &p) const {
    Point diff;
    for (int d = 0; d < NUM_DIMS; d++) {
      if ( p[d] >= coord[d] )
        diff[d] = p[d] - coord[d];
      else
        diff[d] = coord[d] - p[d];
    }
    return diff;
  }

  double manhattan_distance(const Point &p) {
    Point diff = distanceTo(p);
    double dist = std::fabs( diff[0] );
    for (int d = 1; d < NUM_DIMS; d++) {
      dist += std::fabs( diff[d] );
    }

    return dist;
  }
  double euclidean_distance(const Point &p) {
  	Point diff = distanceTo(p);
    double dist = std::fabs( diff[0] );
    for (int d = 1; d < NUM_DIMS; d++) {
      dist += std::fabs( diff[d] * diff[d] ); // why fabs of a square?
    }
    return std::sqrt( dist );
  }
  static double euclidean_distance_squared( const Point& p, const Point& q) {
    return std::inner_product( p.coord, p.coord + NUM_DIMS, q.coord, 0.0, 
    	std::plus< double >(), 
    	[]( double pval, double qval )
    	{
    		return ( pval - qval ) * ( pval - qval );
    	}
    );
  }
  static double euclidean_distance( const Point& p, const Point& q )
  {
  	return std::sqrt( euclidean_distance_squared( p, q ) );
  }

  // minimum distance to another point
  double minDistanceTo(const Point &p) const {
    double min_dist = std::fabs(p[0] - coord[0]);
    for (int d = 1; d < NUM_DIMS; ++d)
      min_dist = std::min(min_dist, std::fabs(p[d] - coord[d]));

    return min_dist;
  }

  // The volume of minimum bounding box of this and point p
  double volumeTo(const Point &p) const {
    double vol = 1;
    const Point diff = distanceTo(p);
    for (int d = 0; d < NUM_DIMS; d++)
      vol *= diff[d];

    return vol;
  }
  
  /**
   * Computes the area of the polygon described by a counter-clockwise ordering 
   * of its extreme points.
   *
   * Using method: http://stackoverflow.com/a/451482/2769271
   * wraps around the input vector, which I account for with the 
   * initial value to std::inner_product().
   */
  static double areaOfPolygon( const PointSet& ps )
  {
  	return std::inner_product( ps.begin(), ps.end() - 1, ps.begin() + 1, 
   			crossProduct( *( ps.end() - 1 ), *( ps.begin() ) ),
   			std::plus< double >(),
   			crossProduct
   		) / 2;
  }
  
  
  /**
   * Reorders a set of 2d points such that those lying on the convex hull,
   * in counter-clockwise order, appear at the beginning of the vector.
   * @return The number of points on the convex hull.
   *
   * Computed using the [Graham Scan](https://en.wikipedia.org/wiki/Graham_scan)
   * algorithm.
   */
  static size_t convexHull( PointSet& ps ) 
  {
  	using namespace std::placeholders;
  	
  	// find the reference point at the bottom which is guaranteed to be on the hull
  	// and order points by their polar angle with respect to that reference point.
   	Point reference = *( std::min_element( ps.begin(), ps.end(), smallerY ) ); // copy.
   	std::sort( ps.begin(), ps.end(), std::bind( smallerPolarAngle, reference, _1, _2 ) );
   	auto ps_end = std::unique( ps.begin(), ps.end() );
   	
   	// Main body of Graham's Scan, copying pseudocode from Wikipedia
   	// (Note! wikipedia uses a sentinel and arrays offset by 1. 
   	// we can assume second point is also on convex hull, which simplifies that, 
   	// because we tie-break our sort with vector magnitude.
   	uint32_t hull_pos = 1;
   	const uint32_t num_original_points = ( ps_end - ps.begin() );
   	for( uint32_t i = 2; i < num_original_points; ++i )
   	{
   		while( ccw( ps[ hull_pos - 1 ], ps[ hull_pos ], ps[ i ] ) )
   		{
   			if( hull_pos > 1 ) { --hull_pos; continue; }
   			else if( i == num_original_points - 1 ) { break; }
   			else { ++i; }
   		}
   		assert( i < num_original_points );
   		++hull_pos;
   		swap( ps[ hull_pos ], ps[ i ] );
   	}
   	return hull_pos + 1; // because of the offset-1 array.
  }
  
  /**
   * Computes the area of the smallest rectangle, not necessarily axis-aligned, 
   * that encloses the input set of points, which are assumed to be their own 
   * convex hull, ordered rotationally. 
   *
   * Exploits that the _minimum enclosing rotated bounding rectangle_ must 
   * have at least one edge overlapping an edge of the convex hull. The 
   * algorithms iterates through all edges of the convex hull (i.e., all 
   * consecutive pairs of points, including the wrap around pair (n-1,0)).
   * For each edge, it explicitly computes the area of the resultant bounding 
   * box. Thus, running time is Θ(n^2) for n = |convex_hull|, since 
   * each area calculation also iterates all points.
   * @see [Source on StackOverflow](http://gis.stackexchange.com/a/22934)
   */
  static std::pair< double, PointSet > minimumRotatedMBR( const PointSet& convex_hull )
  {
  	const size_t num_points = convex_hull.size();
  	std::vector< double > areas( num_points );
  	PointSet solution( 4 );
  	
  	using namespace std::placeholders;
  	auto areaOfRMBR_ThesePoints = std::bind( 
  		areaOfRotatedMBR< false >, convex_hull, _1, _2, 
  		solution[ 0 ], solution[ 1 ], solution[ 2 ], solution[ 3 ] //these aren't used yet. 
  	);
  	
  	// compute the area of each possible MBR, considering that every
  	// MBR must have an edge coincident with an edge of the convex hull
  	std::transform( convex_hull.begin(), convex_hull.end() - 1, 
  		convex_hull.begin() + 1, areas.begin(), areaOfRMBR_ThesePoints
  	);
  	auto defining_edge = std::min_element( areas.begin(), areas.end() - 2 );
  	
  	// finally, check the last edge (wrapping around the vector)
  	// and return the smallest area in the set
  	areas[ num_points - 1 ] = areaOfRMBR_ThesePoints( convex_hull[ num_points - 1 ], 
  		convex_hull[ 0 ] );
  	
  	if( *defining_edge <= areas[ num_points - 1 ] ) // last edge did not improve on best
  	{
  		const size_t offset = defining_edge - std::begin( areas ); 
  		
  		// recompute in order to actually store the points this time
  		areaOfRotatedMBR< true >( 
  			convex_hull, convex_hull[ offset ], convex_hull[ offset + 1 ], 
  			solution[ 0 ], solution[ 1 ], solution[ 2 ], solution[ 3 ]
  		);
  		return std::make_pair( *defining_edge, solution );
  	}
  	else // the wrap-around edge is the ideal defining edge.
  	{
  		// recompute in order to actually store the points this time
  		areaOfRotatedMBR< true >( 
  			convex_hull, convex_hull[ num_points - 1 ], convex_hull[ 0 ], 
  			solution[ 0 ], solution[ 1 ], solution[ 2 ], solution[ 3 ]
  		);
  		return std::make_pair( areas[ num_points - 1 ], solution );
  	}
  }
  
  /**
   * Computes the smallest (in terms of area) four-cornered convex 
   * polygon that encloses (i.e., "circumscribes") the input convex 
   * polygon. Returns the four corner points and the area that they enclose.
   *
   * A soft implementation of Aggarwal, Chang, and Yap (1985). "Minimum 
   * area circumscribing polygons." _The Visual Computer_ 1: 112--117. 
   * doi: 10.1007/BF01898354
   * In particular, we exploit the paper's decomposition of this problem 
   * into that of finding an optimal one-sided chain (P1) and that of 
   * finding an optimal (k-3)-sided flush chain (P2), for all i/j pairs of
   * edges, but more-or-less brute force each of these subproblems.
   * Hence we get a complexity of O(n^3) rather than O(n^2 lg n).
   */
  static std::pair< double, PointSet > fourCorners( const PointSet& convex_hull )
  {
  	PointSet result( 4 );
  	const size_t num_points = convex_hull.size();

  	
  	if( num_points <= 4 )
  	{
  		assert( num_points >= 3 ); // otherwise is it really a polygon?
  		// then solution == input!
  		return std::make_pair(
  			areaOfPolygon( convex_hull ), convex_hull
  		);
  	}
  	
  	// transform the input polygon from vertices into edges.
  	std::vector< PointPair > edges( 2 * num_points - 1 );
  	std::transform( convex_hull.begin(), convex_hull.end() - 1, 
  		convex_hull.begin() + 1, edges.begin(), 
  		[]( const Point& p, const Point& q )
  		{
  			return PointPair( p, q );
  		}
  	);
  	edges[ num_points - 1 ] = PointPair( convex_hull[ num_points -1 ], convex_hull[ 0 ] );
  	
  	// duplicate the edge list for easier "wrapping around" (i.e., mod P)
  	std::transform( convex_hull.begin(), convex_hull.end() - 1, 
  		convex_hull.begin() + 1, edges.begin() + num_points, 
  		[]( const Point& p, const Point& q )
  		{
  			return PointPair( p, q );
  		}
  	);
  	
  	// finds the intersection point of line1 and line2
  	auto intersect = []( const PointPair& line1, const PointPair& line2 )
  	{
  		const double line1_rise = ( line1.second[ 1 ] - line1.first[ 1 ] );
  		const double line1_run = ( line1.second[ 0 ] - line1.first[ 0 ] );
  		const double line2_rise = ( line2.second[ 1 ] - line2.first[ 1 ] );
  		const double line2_run = ( line2.second[ 0 ] - line2.first[ 0 ] );
  		
  		const double line1_y_intercept = line1.first[ 1 ] 
  			- line1_rise / line1_run * line1.first[ 0 ]; 
  		const double line2_y_intercept = line2.first[ 1 ] 
  			- line2_rise / line2_run * line2.first[ 0 ]; 
  		
  		if( ( line1_run == 0 && line2_run == 0 ) 
  			|| ( line1_rise / line1_run == line2_rise / line2_run ) )
  		{
  			// parallel lines. Return "infinite" intersection point.
				return Point{ std::numeric_limits< double >::max(), 
					std::numeric_limits< double >::max() };
  		}
  		if( line1_run == 0 ) // vertical line causes div-by-zero
  		{
				const double xval = line1.first[ 0 ];
				const double yval = line2_rise / line2_run * xval + line2_y_intercept;
				return Point{ xval, yval };
  		}
  		else if( line2_run == 0 )
  		{
				const double xval = line2.first[ 0 ];
				const double yval = line1_rise / line1_run * xval + line1_y_intercept;
				return Point{ xval, yval };
  		}
  		else
  		{
  			const double xval = ( line2_y_intercept - line1_y_intercept ) 
  				/ ( line1_rise / line1_run - line2_rise / line2_run );
  			const double yval = line1_rise / line1_run * xval + line1_y_intercept;
  			return Point{ xval, yval };
  		}
  	};
  	
  	// calculates the area contained by the flush circumscribing polygon that 
  	// starts at i, ends at j, and contains edge mid.
  	auto flush_added_area = [ &intersect ]
  	( const PointPair& i, const PointPair& j, const PointPair& mid, 
  		Point& new_i, Point& new_j )
  	{
  		const Point intersect_i_mid = intersect( i, mid );
  		const Point intersect_mid_j = intersect( mid, j );
  		
  		if( // lines are parallel
  			intersect_i_mid[ 0 ] == std::numeric_limits< double >::max() 
  			|| intersect_mid_j[ 0 ] == std::numeric_limits< double >::max() 
  			// || angle is too great thus intersection point is in wrong direction...
  			|| ( euclidean_distance_squared( intersect_i_mid, i.first ) 
  				< euclidean_distance_squared( intersect_i_mid, i.second ) )
  			|| ( euclidean_distance_squared( intersect_mid_j, j.second ) 
  				< euclidean_distance_squared( intersect_mid_j, j.first ) )
  		)
  		{
  			// could not close polygon. return impossibly high area
  			// together with arbitrary point pair.
  			return std::numeric_limits< double >::max();
  		}
  		else
  		{
  			new_i[ 0 ] = intersect_i_mid[ 0 ];
  			new_i[ 1 ] = intersect_i_mid[ 1 ];
  			new_j[ 0 ] = intersect_mid_j[ 0 ];
  			new_j[ 1 ] = intersect_mid_j[ 1 ];
  			
  			return areaOfPolygon( 
					PointSet{ i.first, intersect_i_mid, intersect_mid_j, j.second } 
				);
			}
  	};
  	
  	// calculates the area contained by the non-flush circumscribing polygon that 
  	// starts at i, ends at j, and passes through vertex mid.
  	auto nonflush_added_area = [ &intersect ]
  	( const PointPair& i, const PointPair& j, const Point& mid, 
  		Point& new_i, Point& new_j )
  	{
  		const Point head = intersect( i, j );
  		if( 
  			head[ 0 ] == std::numeric_limits< double >::max() 
  			|| ( euclidean_distance_squared( head, i.first ) 
  				< euclidean_distance_squared( head, i.second ) )
  			|| ( euclidean_distance_squared( head, j.second ) 
  				< euclidean_distance_squared( head, j.first ) )
  		)
  		{
  			// does not close polygon. return impossibly high score
  			// together with arbitrary point pair.
  			return std::numeric_limits< double >::max();
  		}
  		
  		
  		const double mid_dist = euclidean_distance( head, mid );
  		const Point mid_norm{ ( mid[ 0 ] - head[ 0 ] ) / mid_dist, 
  			( mid[ 1 ] - head[ 1 ] ) /mid_dist  };
  		
  		// double check that mid_dist is reasonable. 
  		// sometimes "near parallel" lines lead to divisions 
  		// that even "double" doesn't have sufficient precision for.
  		if( mid_dist > 1000 * euclidean_distance( i.first, j.first ) )
  		{
  			return std::numeric_limits< double >::max();
  		}
  		
  		// pick further point on line segment to avoid magnitudes close to 0.
  		const Point& i_ref = ( 
  			vectorMagnitude( i.first, head ) < vectorMagnitude( i.second, head ) 
  			? i.second : i.first 
  		);
  		const double i_mag = vectorMagnitude( i_ref, head );
  		const Point i_norm{ ( i_ref[ 0 ] - head[ 0 ] ) / i_mag, 
  			( i_ref[ 1 ] - head[ 1 ] ) / i_mag };
  		
  		const Point& j_ref = ( 
  			vectorMagnitude( j.first, head ) < vectorMagnitude( j.second, head ) 
  			? j.second : j.first 
  		);
  		const double j_mag = vectorMagnitude( j_ref, head );
  		const Point j_norm{ ( j_ref[ 0 ] - head[ 0 ] ) / j_mag, 
  			( j_ref[ 1 ] - head[ 1 ] ) / j_mag };
  		
  		const double angle_i_mid = std::acos( dotProduct( i_norm, mid_norm ) );
  		const double angle_j_mid = std::acos( dotProduct( mid_norm, j_norm ) );
  		const double angle_i_j = std::acos( dotProduct( i_norm, j_norm ) );
  		
  		const double dist_new_j = mid_dist * 2 * sin( angle_i_mid ) / sin( angle_i_j );
  		const double dist_new_i = mid_dist * 2 * sin( angle_j_mid ) / sin( angle_i_j );
  		
  		new_i[ 0 ] = head[ 0 ] + i_norm[ 0 ] * dist_new_i;
  		new_i[ 1 ] = head[ 1 ] + i_norm[ 1 ] * dist_new_i;
  		new_j[ 0 ] = head[ 0 ] + j_norm[ 0 ] * dist_new_j;
  		new_j[ 1 ] = head[ 1 ] + j_norm[ 1 ] * dist_new_j;
  		
  		return areaOfPolygon( 
  			PointSet{ i.first, new_i, new_j, j.second } 
  		);
  	};
  	
  	double global_best_area = std::numeric_limits< double >::max();
  	using namespace std::placeholders;
  	Point new_i, new_j;
   	for( size_t i = 0; i < num_points; ++i )
   	{
   		for( size_t j = i + 2; j < i + num_points - 2; ++j  )
   		{
   			const double ij_quadrilateral_area = areaOfPolygon( 
					PointSet{ edges[ i ].first, edges[ i ].second, edges[ j ].first, edges[ j ].second } 
				);
				
   			auto flush_ij = std::bind( 
   				flush_added_area, edges[ i ], edges[ j ], _1, std::ref( new_i ), std::ref( new_j ) 
   			);
   			std::vector< std::pair< double, PointPair > > added_areas;
   			for( auto mid = i + 1; mid < j; ++mid )
   			{
   				const double new_area = flush_ij( edges[ mid ] );
   				added_areas.push_back( std::make_pair( 
   					new_area, PointPair( Point( new_i ), Point( new_j ) ) 
   				) );
   			}
   			const auto min_flush = std::min_element( added_areas.begin(), added_areas.end() );
   			
   			auto flush_ji = std::bind( 
   				flush_added_area, edges[ j ], edges[ i ], _1, std::ref( new_i ), std::ref( new_j ) 
   			);
   			auto nonflush_ji = std::bind( 
   				nonflush_added_area, edges[ j ], edges[ i ], _1, std::ref( new_i ), std::ref( new_j ) 
   			);
   			
   			std::vector< std::pair< double, PointPair > > added_areas_bottom;
   			for( size_t mid = j + 1; mid < i + num_points - 1; ++mid )
   			{
   				const double flush = flush_ji( edges[ mid ] );
   				added_areas_bottom.push_back( std::make_pair( 
   					flush, PointPair( Point( new_i ), Point( new_j ) )
   				) );
   				const double nonflush = nonflush_ji( edges[ mid ].second );
   				added_areas_bottom.push_back( std::make_pair(
   					nonflush, PointPair( Point( new_i ), Point( new_j ) )
   				) );
   			}
   			const double last_ji_flush = flush_ji( edges[ i + num_points - 1 ] );
   			added_areas_bottom.push_back( std::make_pair(
   				last_ji_flush, PointPair( Point( new_i ), Point( new_j ) )
   			) );
   			const auto min_nonflush = std::min_element( 
   				added_areas_bottom.begin(), added_areas_bottom.end()
   			);
   			
   			const double total_area = min_flush->first + min_nonflush->first 
   				- ij_quadrilateral_area; // subtracting off the double-counted area between i/j.
   			
   			if( total_area < global_best_area )
   			{
   				global_best_area = total_area;
   				result = PointSet{ min_flush->second.first, min_flush->second.second,
   					min_nonflush->second.first, min_nonflush->second.second };
   			}
   		}
   	}
  	
  	return std::make_pair( global_best_area, result );
  }
  
  /**
   * Computes the smallest (in terms of area) five-cornered convex 
   * polygon that encloses (i.e., "circumscribes") the input convex 
   * polygon. Returns the five corner points and the area that they enclose.
   * @see fourCorners()
   * @todo Egads! Refactor! This is more-or-less a copy-paste of fourCorners(), 
   * which itself is in desperate need of a refactor!
   *
   * A soft implementation of Aggarwal, Chang, and Yap (1985). "Minimum 
   * area circumscribing polygons." _The Visual Computer_ 1: 112--117. 
   * doi: 10.1007/BF01898354
   * In particular, we exploit the paper's decomposition of this problem 
   * into that of finding an optimal one-sided chain (P1) and that of 
   * finding an optimal (k-3)-sided flush chain (P2), for all i/j pairs of
   * edges, but more-or-less brute force each of these subproblems.
   * Hence we get a complexity of O(n^3) rather than O(n^2 lg n).
   */
  static std::pair< double, PointSet > fiveCorners( const PointSet& convex_hull )
  {
  	PointSet result( 5 );
  	const size_t num_points = convex_hull.size();

  	
  	if( num_points <= 5 )
  	{
  		assert( num_points >= 3 ); // otherwise is it really a polygon?
  		// then solution == input!
  		return std::make_pair(
  			areaOfPolygon( convex_hull ), convex_hull
  		);
  	}
  	
  	// transform the input polygon from vertices into edges.
  	std::vector< PointPair > edges( 2 * num_points - 1 );
  	std::transform( convex_hull.begin(), convex_hull.end() - 1, 
  		convex_hull.begin() + 1, edges.begin(), 
  		[]( const Point& p, const Point& q )
  		{
  			return PointPair( p, q );
  		}
  	);
  	edges[ num_points - 1 ] = PointPair( convex_hull[ num_points -1 ], convex_hull[ 0 ] );
  	
  	// duplicate the edge list for easier "wrapping around" (i.e., mod P)
  	std::transform( convex_hull.begin(), convex_hull.end() - 1, 
  		convex_hull.begin() + 1, edges.begin() + num_points, 
  		[]( const Point& p, const Point& q )
  		{
  			return PointPair( p, q );
  		}
  	);
  	
  	// finds the intersection point of line1 and line2
  	auto intersect = []( const PointPair& line1, const PointPair& line2 )
  	{
  		const double line1_rise = ( line1.second[ 1 ] - line1.first[ 1 ] );
  		const double line1_run = ( line1.second[ 0 ] - line1.first[ 0 ] );
  		const double line2_rise = ( line2.second[ 1 ] - line2.first[ 1 ] );
  		const double line2_run = ( line2.second[ 0 ] - line2.first[ 0 ] );
  		
  		const double line1_y_intercept = line1.first[ 1 ] 
  			- line1_rise / line1_run * line1.first[ 0 ]; 
  		const double line2_y_intercept = line2.first[ 1 ] 
  			- line2_rise / line2_run * line2.first[ 0 ]; 
  		
  		if( ( line1_run == 0 && line2_run == 0 ) 
  			|| ( line1_rise / line1_run == line2_rise / line2_run ) )
  		{
  			// parallel lines. Return "infinite" intersection point.
				return Point{ std::numeric_limits< double >::max(), 
					std::numeric_limits< double >::max() };
  		}
  		if( line1_run == 0 ) // vertical line causes div-by-zero
  		{
				const double xval = line1.first[ 0 ];
				const double yval = line2_rise / line2_run * xval + line2_y_intercept;
				return Point{ xval, yval };
  		}
  		else if( line2_run == 0 )
  		{
				const double xval = line2.first[ 0 ];
				const double yval = line1_rise / line1_run * xval + line1_y_intercept;
				return Point{ xval, yval };
  		}
  		else
  		{
  			const double xval = ( line2_y_intercept - line1_y_intercept ) 
  				/ ( line1_rise / line1_run - line2_rise / line2_run );
  			const double yval = line1_rise / line1_run * xval + line1_y_intercept;
  			return Point{ xval, yval };
  		}
  	};
  	
  	// calculates the area contained by the flush circumscribing polygon that 
  	// starts at i, ends at j, and contains edge mid.
  	auto flush_added_area = [ &intersect ]
  	( const PointPair& i, const PointPair& j, const PointPair& mid, 
  		Point& new_i, Point& new_j )
  	{
  		const Point intersect_i_mid = intersect( i, mid );
  		const Point intersect_mid_j = intersect( mid, j );
  		
  		if( // lines are parallel
  			intersect_i_mid[ 0 ] == std::numeric_limits< double >::max() 
  			|| intersect_mid_j[ 0 ] == std::numeric_limits< double >::max() 
  			// || angle is too great thus intersection point is in wrong direction...
  			|| ( euclidean_distance_squared( intersect_i_mid, i.first ) 
  				< euclidean_distance_squared( intersect_i_mid, i.second ) )
  			|| ( euclidean_distance_squared( intersect_mid_j, j.second ) 
  				< euclidean_distance_squared( intersect_mid_j, j.first ) )
  		)
  		{
  			// could not close polygon. return impossibly high area
  			// together with arbitrary point pair.
  			return std::numeric_limits< double >::max();
  		}
  		else
  		{
  			new_i[ 0 ] = intersect_i_mid[ 0 ];
  			new_i[ 1 ] = intersect_i_mid[ 1 ];
  			new_j[ 0 ] = intersect_mid_j[ 0 ];
  			new_j[ 1 ] = intersect_mid_j[ 1 ];
  			
  			return areaOfPolygon( 
					PointSet{ i.first, intersect_i_mid, intersect_mid_j, j.second } 
				);
			}
  	};
  	
  	// calculates the area contained by the flush circumscribing polygon that 
  	// starts at i, ends at j, and contains two edges mid1 and mid2.
  	auto twoflush_added_area = [ &intersect ]
  	( const PointPair& i, const PointPair& j, const PointPair& mid1, 
  		const PointPair& mid2, Point& new_i, Point& new_j, Point& new_mid )
  	{
  		const Point intersect_i_mid = intersect( i, mid1 );
  		const Point intersect_mids = intersect( mid1, mid2 );
  		const Point intersect_mid_j = intersect( mid2, j );
  		
  		if( // lines are parallel
  			intersect_i_mid[ 0 ] == std::numeric_limits< double >::max() 
  			|| intersect_mid_j[ 0 ] == std::numeric_limits< double >::max() 
  			|| intersect_mids[ 0 ] == std::numeric_limits< double >::max() 
  			// || angle is too great thus intersection point is in wrong direction...
  			|| ( euclidean_distance_squared( intersect_i_mid, i.first ) 
  				< euclidean_distance_squared( intersect_i_mid, i.second ) )
  			|| ( euclidean_distance_squared( intersect_mid_j, j.second ) 
  				< euclidean_distance_squared( intersect_mid_j, j.first ) )
  			|| ( euclidean_distance_squared( intersect_mids, mid1.first ) 
  				< euclidean_distance_squared( intersect_mids, mid1.second ) )
  		)
  		{
  			// could not close polygon. return impossibly high area
  			// together with arbitrary point pair.
  			return std::numeric_limits< double >::max();
  		}
  		else
  		{
  			new_i[ 0 ] = intersect_i_mid[ 0 ];
  			new_i[ 1 ] = intersect_i_mid[ 1 ];
  			new_j[ 0 ] = intersect_mid_j[ 0 ];
  			new_j[ 1 ] = intersect_mid_j[ 1 ];
  			new_mid[ 0 ] = intersect_mids[ 0 ];
  			new_mid[ 1 ] = intersect_mids[ 1 ];
  			
  			return areaOfPolygon( 
					PointSet{ i.first, intersect_i_mid, intersect_mids, intersect_mid_j, j.second } 
				);
			}
  	};
  	
  	// calculates the area contained by the non-flush circumscribing polygon that 
  	// starts at i, ends at j, and passes through vertex mid.
  	auto nonflush_added_area = [ &intersect ]
  	( const PointPair& i, const PointPair& j, const Point& mid, 
  		Point& new_i, Point& new_j )
  	{
  		const Point head = intersect( i, j );
  		if( 
  			head[ 0 ] == std::numeric_limits< double >::max() 
  			|| ( euclidean_distance_squared( head, i.first ) 
  				< euclidean_distance_squared( head, i.second ) )
  			|| ( euclidean_distance_squared( head, j.second ) 
  				< euclidean_distance_squared( head, j.first ) )
  		)
  		{
  			// does not close polygon. return impossibly high score
  			// together with arbitrary point pair.
  			return std::numeric_limits< double >::max();
  		}
  		
  		
  		const double mid_dist = euclidean_distance( head, mid );
  		const Point mid_norm{ ( mid[ 0 ] - head[ 0 ] ) / mid_dist, 
  			( mid[ 1 ] - head[ 1 ] ) /mid_dist  };
  		
  		// double check that mid_dist is reasonable. 
  		// sometimes "near parallel" lines lead to divisions 
  		// that even "double" doesn't have sufficient precision for.
  		if( mid_dist > 1000 * euclidean_distance( i.first, j.first ) )
  		{
  			return std::numeric_limits< double >::max();
  		}
  		
  		// pick further point on line segment to avoid magnitudes close to 0.
  		const Point& i_ref = ( 
  			vectorMagnitude( i.first, head ) < vectorMagnitude( i.second, head ) 
  			? i.second : i.first 
  		);
  		const double i_mag = vectorMagnitude( i_ref, head );
  		const Point i_norm{ ( i_ref[ 0 ] - head[ 0 ] ) / i_mag, 
  			( i_ref[ 1 ] - head[ 1 ] ) / i_mag };
  		
  		const Point& j_ref = ( 
  			vectorMagnitude( j.first, head ) < vectorMagnitude( j.second, head ) 
  			? j.second : j.first 
  		);
  		const double j_mag = vectorMagnitude( j_ref, head );
  		const Point j_norm{ ( j_ref[ 0 ] - head[ 0 ] ) / j_mag, 
  			( j_ref[ 1 ] - head[ 1 ] ) / j_mag };
  		
  		const double angle_i_mid = std::acos( dotProduct( i_norm, mid_norm ) );
  		const double angle_j_mid = std::acos( dotProduct( mid_norm, j_norm ) );
  		const double angle_i_j = std::acos( dotProduct( i_norm, j_norm ) );
  		
  		const double dist_new_j = mid_dist * 2 * sin( angle_i_mid ) / sin( angle_i_j );
  		const double dist_new_i = mid_dist * 2 * sin( angle_j_mid ) / sin( angle_i_j );
  		
  		new_i[ 0 ] = head[ 0 ] + i_norm[ 0 ] * dist_new_i;
  		new_i[ 1 ] = head[ 1 ] + i_norm[ 1 ] * dist_new_i;
  		new_j[ 0 ] = head[ 0 ] + j_norm[ 0 ] * dist_new_j;
  		new_j[ 1 ] = head[ 1 ] + j_norm[ 1 ] * dist_new_j;
  		
  		return areaOfPolygon( 
  			PointSet{ i.first, new_i, new_j, j.second } 
  		);
  	};
  	
  	double global_best_area = std::numeric_limits< double >::max();
  	using namespace std::placeholders;
  	Point new_i, new_j, new_mid;
   	for( size_t i = 0; i < num_points; ++i )
   	{
   		for( size_t j = i + 3; j < i + num_points - 2; ++j  )
   		{
   			const double ij_quadrilateral_area = areaOfPolygon( 
					PointSet{ edges[ i ].first, edges[ i ].second, edges[ j ].first, edges[ j ].second } 
				);
				
   			auto flush_ij = std::bind( 
   				twoflush_added_area, edges[ i ], edges[ j ], _1, _2, 
   				std::ref( new_i ), std::ref( new_j ), std::ref( new_mid )
   			);
   			std::vector< std::pair< double, PointSet > > added_areas;
   			for( auto mid1 = i + 1; mid1 < j - 1; ++mid1 )
   			{
   				for( auto mid2 = mid1 + 1; mid2 < j; ++mid2 )
   				{
						const double new_area = flush_ij( edges[ mid1 ], edges[ mid2 ] );
						added_areas.push_back( std::make_pair( 
							new_area, PointSet{ Point( new_i ), Point( new_mid ), Point( new_j ) } 
						) );
					}
   			}
   			const auto min_flush = std::min_element( added_areas.begin(), added_areas.end() );
   			
   			auto flush_ji = std::bind( 
   				flush_added_area, edges[ j ], edges[ i ], _1, std::ref( new_i ), std::ref( new_j ) 
   			);
   			auto nonflush_ji = std::bind( 
   				nonflush_added_area, edges[ j ], edges[ i ], _1, std::ref( new_i ), std::ref( new_j ) 
   			);
   			
   			std::vector< std::pair< double, PointPair > > added_areas_bottom;
   			for( size_t mid = j + 1; mid < i + num_points - 1; ++mid )
   			{
   				const double flush = flush_ji( edges[ mid ] );
   				added_areas_bottom.push_back( std::make_pair( 
   					flush, PointPair( Point( new_i ), Point( new_j ) )
   				) );
   				const double nonflush = nonflush_ji( edges[ mid ].second );
   				added_areas_bottom.push_back( std::make_pair(
   					nonflush, PointPair( Point( new_i ), Point( new_j ) )
   				) );
   			}
   			const double last_ji_flush = flush_ji( edges[ i + num_points - 1 ] );
   			added_areas_bottom.push_back( std::make_pair(
   				last_ji_flush, PointPair( Point( new_i ), Point( new_j ) )
   			) );
   			const auto min_nonflush = std::min_element( 
   				added_areas_bottom.begin(), added_areas_bottom.end()
   			);
   			
   			const double total_area = min_flush->first + min_nonflush->first 
   				- ij_quadrilateral_area; // subtracting off the double-counted area between i/j.
   			
   			if( total_area < global_best_area )
   			{
   				global_best_area = total_area;
   				result = PointSet{ 
   					min_flush->second[ 0 ], min_flush->second[ 1 ], min_flush->second[ 2 ], 
   					min_nonflush->second.first, min_nonflush->second.second
   				};
   			}
   		}
   	}
  	
  	return std::make_pair( global_best_area, result );
  }
  

  void print(std::string prefix = "") const {
    printf( "%s [", prefix.c_str() );
    for (uint32_t i = 0; i < NUM_DIMS; ++i)
      printf( "%f ", coord[i] );
    printf( "]" );
  }
};

class Box {
  
public:
  Point lo;
  Point hi;

  Box(): lo(), hi() {  }
  Box( const Point &p1, const Point &p2 ): lo(p1), hi(p2) {
    for (uint32_t d = 0; d < NUM_DIMS; ++d)
      if (p2[d] < p1[d]) { lo[d] = p2[d]; hi[d]= p1[d]; }
  }

  inline bool operator==(const Box &rhs) const {
    return ( lo == rhs.lo && hi == rhs.hi);
  }

  /*
   * Strict contains! (I.e., not on edge.)
   */
  bool contains(const Point &p) const {
    for (uint32_t d = 0; d < NUM_DIMS; ++d) {
      if ( !(lo[d] <= p[d] && hi[d] >= p[d]) )
        return false;
    }
    return true;
  }

  inline bool overlap(const Box &b) const {
    for (uint32_t d = 0; d < NUM_DIMS; ++d)
      if ( hi[d] < b.lo[d] || b.hi[d] < lo[d] )
        return false;
    return true;
  }

  /*
   * Warning: touching is not overlapping here (which is not the case generally)
   */
  inline bool overlap_(const std::vector<Box> &boxes) const {
    for (uint32_t b = 0; b < boxes.size(); ++b) {
      bool overlap = true;
      for (uint32_t d = 0; d < NUM_DIMS; ++d) {
        if (hi[d] <= boxes[b].lo[d] || boxes[b].hi[d] <= lo[d]) {
          overlap = false;
          break;
        }
      }
      if (overlap)
        return true;
    }
    return false;
  }
  inline bool overlap_(const std::vector<Point> &points) const {
    for (uint32_t b = 0; b < points.size(); ++b) {
      bool overlap = true;
      for (uint32_t d = 0; d < NUM_DIMS; ++d) {
        if (hi[d] <= points[b][d] || points[b][d] <= lo[d]) {
          overlap = false;
          break;
        }
      }
      if (overlap)
        return true;
    }
    return false;
  }

  bool isOnEdge( const Point &p ) {
    for (uint32_t d = 0; d < NUM_DIMS; ++d) {
      if (p[d] == lo[d] || p[d] == hi[d]) {
        return true;
      }
    }
    return false;
  }

  Box combine(const Box &b) const {
    Box combined;
    for (uint32_t d = 0; d < NUM_DIMS; ++d) {
      if (lo[d] <=b.lo[d]) combined.lo[d] = lo[d];
      else combined.lo[d]  = b.lo[d];

      if (hi[d] >= b.hi[d]) combined.hi[d] = b.hi[d];
      else combined.hi[d] = b.hi[d];
    }

    return combined;
  }

  Box overlappingBox(const Box &b) const {
    Box overlapping;
    if ( !overlap(b) )
      return overlapping;

    int countOverlap = 0;
    overlapping = combine( b );

    for (uint32_t i = 0; i < NUM_DIMS; ++i) {
      if (lo[i] >= b.lo[i] && lo[i] <= b.hi[i]) { countOverlap++; overlapping.lo[i] = lo[i]; }
      if (hi[i] >= b.lo[i] && hi[i] <= b.hi[i]) { countOverlap++; overlapping.hi[i] = hi[i]; }
      if (b.lo[i] >= lo[i] && b.lo[i] <= hi[i]) { countOverlap++; overlapping.lo[i] = b.lo[i]; }
      if (b.hi[i] >= lo[i] && b.hi[i] <= hi[i]) { countOverlap++; overlapping.hi[i] = b.hi[i]; }
    }

    return overlapping;
  }

  double volume() const {
    return lo.volumeTo( hi );
  }

  Point getCorner(const uint32_t code) const {
    const std::bitset<NUM_DIMS> mask( code );
    Point corner;
    for (uint32_t d = 0; d < NUM_DIMS; ++d)
      if (mask[d] == 0)
        corner[d] = lo[d];
      else
        corner[d] = hi[d];
    return corner;
  }
  Point getCorner(const std::bitset<NUM_DIMS> mask) const {
    Point corner;
    for (uint32_t d = 0; d < NUM_DIMS; ++d)
      if (mask[d] == 0)
        corner[d] = lo[d];
      else
        corner[d] = hi[d];
    return corner;
  }

  Point getCenter() const {
    Point center;
    for (uint32_t d = 0; d < NUM_DIMS; ++d)
      center[d] = (lo[d] + hi[d]) / 2;
    return center;
  }

  /*
   * Returns two edges originating from a given corner ccode.
   */
  using Line = std::pair<Point, Point>;
  std::vector<Line> getEdges(const uint32_t ccode) const {
    std::vector<Line> lines;
    lines.reserve(2);
    switch (ccode) {
      case 0:
        lines.push_back( Line(getCorner(0), getCorner(2)) );
        lines.push_back( Line(getCorner(0), getCorner(1)) );
        break;
      case 1:
        lines.push_back( Line(getCorner(1), getCorner(0)) );
        lines.push_back( Line(getCorner(1), getCorner(3)) );
        break;
      case 2:
        lines.push_back( Line(getCorner(2), getCorner(0)) );
        lines.push_back( Line(getCorner(2), getCorner(3)) );
        break;
      case 3:
        lines.push_back( Line(getCorner(3), getCorner(2)) );
        lines.push_back( Line(getCorner(3), getCorner(1)) );
        break;
    }
    return lines;
  }

  
  class AreaCountingEvent {
  public:	
  	enum class EventType { insert, remove };
  	
  	double _xval;
  	EventType _type;
  	double _ymin, _ymax;
  	
  	// default/init-vals constructor
  	AreaCountingEvent( const double xval = 0, const EventType type = EventType::remove, 
  		const double ymin = 0, const double ymax = 0 ) 
  		: _xval( xval ), _type( type ), _ymin( ymin ), _ymax( ymax ) {}
  	
  	// copy constructor
  	AreaCountingEvent( const AreaCountingEvent& clone ) 
  		: _xval( clone._xval ), _type( clone._type ), _ymin( clone._ymin ), _ymax( clone._ymax ) {}
		
		// move constructor
		AreaCountingEvent( AreaCountingEvent&& other ) : AreaCountingEvent()
		{
			swap( *this, other );
		}
  	
  	~AreaCountingEvent() {}
  	
  	// for copy-swap.
		static void swap( AreaCountingEvent& first, AreaCountingEvent& second )
		{
			std::swap( first._xval, second._xval );
			std::swap( first._type, second._type );
			std::swap( first._ymin, second._ymin );
			std::swap( first._ymax, second._ymax );
		}
  	inline AreaCountingEvent& operator=( AreaCountingEvent other ) 
  	{
  		swap( *this, other );
  		return *this; 
  	}

  };
  
  /*
   * Few static methods..
   */
  static Box computeMBR(const std::vector<Box> &objs) {
    assert( !objs.empty() );
    Box mbr = objs[0];
    for (uint32_t i = 1; i < objs.size(); ++i) {
      for (uint32_t d = 0; d < NUM_DIMS; ++d) {
        mbr.lo[d] = std::min(mbr.lo[d], objs[i].lo[d]);
        mbr.hi[d] = std::max(mbr.hi[d], objs[i].hi[d]);
      }
    }
    return mbr;
  }
  
  /**
   * Computes the exact area covered by 2d boxes, without double-counting overlap.
   *
   * Uses a plane sweep algorithm along the x-axis that maintains a list of 
   * "active intervals" on the y-axis. Basically, the entire space is cut up into 
   * "columns" using the min-x and max-x coordinates of all the boxes. Then it 
   * suffices to add up the area covered in each column. To do that, we maintain 
   * a set of "active intervals" along the y-dimension, which is updated at each 
   * event (i.e., a top-right corner/max-x coordinate of a box removes an interval 
   * and a bottom-left/min-x coordinate of a box adds a new interval). Then we can 
   * simply scan the intervals to calculate the 1d coverage along the y-axis within 
   * the column and multiply that by the width of the column (distance since the 
   * previous x-value "event".
   */
  static double totalArea2d( const std::vector< Box >& boxes )
  {
  	
		// create queue of events for plane sweep algorithm.
		std::vector< Box::AreaCountingEvent > events;
		events.reserve( 2 * boxes.size() );
		std::for_each( boxes.begin(), boxes.end(), 
			[ &events ]( const Box& b )
			{
 				events.push_back( 
 					AreaCountingEvent( b.lo[ 0 ], AreaCountingEvent::EventType::insert, 
 						b.lo[ 1 ], b.hi[ 1 ] ) 
 				);
 				events.push_back( 
 					AreaCountingEvent( b.hi[ 0 ], AreaCountingEvent::EventType::remove, 
 						b.lo[ 1 ], b.hi[ 1 ] ) 
 				);
			}
		);
 		std::sort( events.begin(), events.end(), std::less< AreaCountingEvent >() );
 		
 		using IntervalBoundary 
 			= std::pair< double, bool >; //(  yval, closesInterval? ) <- sorts open/false first!
 		std::vector< IntervalBoundary > intervals; // set may be faster, but need duplicate vals.
 		double prev_xval = 0.0;
 		return std::accumulate( events.begin(), events.end(), 0.0, 
 			[ &prev_xval, &intervals ]( const double sum, const AreaCountingEvent &e )
 			{
 				const double column_width = ( e._xval - prev_xval );
 				prev_xval = e._xval;
 				
 				// add together the height of all the active boxes in this column.
 				double current_bottom;
 				uint32_t num_overlapping_boxes = 0;
 				double column_height = 
 					std::accumulate( intervals.begin(), intervals.end(), 0.0, 
 						[ &current_bottom, &num_overlapping_boxes ]( 
 						const double h, const IntervalBoundary& ib )
 						{
 							if( num_overlapping_boxes == 0 ) // empty interval range; starting new one.
 							{
 								++num_overlapping_boxes;
 								current_bottom = ib.first;
 								return h;
 							}
 							else
 							{
 								if( !ib.second ) // opened a new box inside our current one.
 								{
 									++num_overlapping_boxes;
 									return h;
 								}
 								else // closing a box
 								{
 									if( --num_overlapping_boxes == 0 ) // this box was the last open one.
 									{
 										return h + ( ib.first - current_bottom );
 									}
 									else
 									{
 										return h;
 									}
 								}
 							}
 						}
 					);
 				assert( num_overlapping_boxes == 0 );
 				
 				// Update list of intervals with new event.
 				// this is where using std::set instead of std::vector might be faster, 
 				// especially for the delete operation. Maybe std::multiset might be a good 
 				// compromise to maintain duplicate values while also maintaining O( lg n ) 
 				// insertion/deletion?
 				if( e._type == AreaCountingEvent::EventType::insert )
 				{
 					intervals.push_back( IntervalBoundary( e._ymin, false ) );
 					intervals.push_back( IntervalBoundary( e._ymax, true ) );
 					std::sort( intervals.begin(), intervals.end(), std::less< IntervalBoundary >() );
 				}
 				else // remove both sides of the interval.
 				{
 					auto it = std::find( 
 						intervals.begin(), intervals.end(), IntervalBoundary( e._ymin, false ) );	
 					if( it != intervals.end() )
 					{
 						intervals.erase( it );
 					}
 					it = std::find( 
 						intervals.begin(), intervals.end(), IntervalBoundary( e._ymax, true ) );	
 					if( it != intervals.end() )
 					{
 						intervals.erase( it );
 					}
 				}
 				
 				return sum + column_height * column_width;
 			}
 		);
  }
};
  	
// Operator overloads for storing in sorted containers.
inline bool operator==( const Box::AreaCountingEvent& lhs, 
	const Box::AreaCountingEvent& rhs ) 
{
	return lhs._xval == rhs._xval;
}
inline bool operator< ( const Box::AreaCountingEvent& lhs, 
	const Box::AreaCountingEvent& rhs )
{
	return lhs._xval < rhs._xval;
}
//std::ostream& operator<<( std::ostream& os, const Box::AreaCountingEvent& e ){
//  return os << "(" << e._xval << ": [" << e._ymin << "," << e._ymax << "]";
//}

#endif /* SPATIALOBJECTS_H_ */
