/*
 * ASCIDumpParser.h
 *
 *  Created on: Jan 7, 2016
 *      Author: sidlausk
 */

#ifndef ASCIDUMPPARSER_H_
#define ASCIDUMPPARSER_H_

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cassert>
#include <limits>
#include <map>

#include "SpatialObjects.h"

/*
 * Used to be called "deadly" points.
 */
typedef struct ClipPoint {
  uint32_t corner_code;
  double score; // absolute volume
  Point ccoord;
  Point dpoint;
  ClipPoint(): corner_code(0), score(-1.0), ccoord(Point()), dpoint(Point()) {

  }
  ClipPoint(const uint32_t cc, const Point &corner, const Point &dp):
  	  corner_code(cc), score(corner.volumeTo(dp)), ccoord(corner), dpoint(dp) {

  }

  bool operator<(const ClipPoint& rhs) const { return score > rhs.score; }
  bool operator==(const Point &p) {
	  for (uint32_t d = 0; d < NUM_DIMS; ++d)
		if ( dpoint[d] != p[d] ) return false;
	  return true;
  }

  const Box toMBR() const {
	  return Box(ccoord, dpoint);
  }

} DeadSpaceInfo;

typedef struct NodeInfo {
  std::string name;
  uint32_t id;
  uint32_t level;
  uint32_t size;
  uint32_t max_cpoints; // max #clip-points per node
  uint32_t ch_size;     // #points in convex hull
  double exact_coverage_abs; // absolute area occupied by entries
  double pixel_coverage_abs; // pixel-based absolute area occupied by entries
  double pixel_overlap_abs; // pixel-based absolute overlapping volume among node's entries
  double area_min_enclosing_circle_abs; // absolute area of min. enclosing circle of entries
  double area_min_rotated_mbr_abs; // absolute area of smallest rectangle enclosing entries
  double area_four_corners; // absolute area of smallest 4-cornered polygon enclosing entries
  double area_five_corners; // absolute area of smallest 5-cornered polygon enclosing entries
  double spherical_dead_space; // absolute area of min enclosing circle not covering entries
  double convex_hull_area; // absolute area of the convex hull of the entries
  double total_chamfered_percent; // skyline or staircase removed dead-space (%)
  Box mbr;
  std::vector<Box> entries;
  std::vector<Box> data_pixels; // for visualizing pixel-based data coverage

  std::vector<std::vector<ClipPoint> > skylines; // for each corner
  std::vector<std::vector<ClipPoint> > stairs; // for each corner
  std::vector<std::vector<std::pair<Point, Point> > > cbbs; // vector of line sets representing CBBs

  std::vector<Point> ch_vertices; // CH vertices
  std::vector<std::vector<Point> > ch_planes; // vector of line sets representing CBBs
  std::vector<std::pair<Point, Point> > ch_edges; // vector of lines representing CH
  std::vector<Box> ch_pixels; // raster of convex hull

  std::vector<Point> rotated_mbr; // for each corner of the rotated MBR
  std::vector<Point> four_corners; // the min-area 4-cornered circumscribing convex polygon
  std::vector<Point> five_corners; // the min-area 5-cornered circumscribing convex polygon
  std::vector<Point> triacontakaihexagon; //  36-sided polygon (used to approximate a circle
                                          // for visualization purposes)

  std::vector<ClipPoint> clip_points;
  std::vector<double> accum_clipped_abs; // prefix sum of chamfered dead-space with each dpoint (absolute)
  std::vector<double> accum_abs_chamfered_overlap; // as above just for overlap among chamfered dead-space (absolute)

  std::string path_from_root;

  NodeInfo(const uint32_t max_clip_points = NUM_CORNERS):
	  max_cpoints(max_clip_points) {
	  reset();
  }

  void reset() {
    name.clear();
    id = std::numeric_limits<uint32_t>::max();
    level = std::numeric_limits<uint32_t>::max();
    size = std::numeric_limits<uint32_t>::max();
    ch_size = 0;
    exact_coverage_abs = 0.0;
    pixel_coverage_abs = 0.0;
    pixel_overlap_abs = 0.0;
    area_min_enclosing_circle_abs = 0.0;
    area_min_rotated_mbr_abs = 0.0;
    area_four_corners = 0.0;
    area_five_corners = 0.0;
    spherical_dead_space = 0.0;
    convex_hull_area = 0.0;
    total_chamfered_percent = 0.0;
    entries.clear();
    data_pixels.clear();
    skylines.clear();
    skylines.resize( NUM_CORNERS );
    stairs.clear();
    stairs.resize( NUM_CORNERS );
    clip_points.clear();
    cbbs.clear();
    ch_vertices.clear();
    ch_planes.clear();
    ch_edges.clear();
    ch_pixels.clear();
    rotated_mbr.clear();
    four_corners.clear();
    five_corners.clear();
    triacontakaihexagon.clear();
    accum_clipped_abs.clear();
    accum_clipped_abs.resize( max_cpoints, 0.0 );
    accum_abs_chamfered_overlap.clear();
    accum_abs_chamfered_overlap.resize( max_cpoints, 0.0 );
    path_from_root = "";
  }
} NodeInfo;

class ASCIDumpParser {
private:
  int num_dims_;
  int nodes_parsed_;
  long line_counter_;
  uint32_t llevel_;

  std::string data_name_;
  std::string index_name_;
  std::ifstream ifs_;

public:
  ASCIDumpParser(std::string inputfile, const uint32_t llevel = 0);
  virtual ~ASCIDumpParser();

  bool getNextNode( NodeInfo &node );

  // setters & getters
  int num_dims() const { return num_dims_; }
  long line_counter() const { return line_counter_; }
  int nodes_parsed() const { return nodes_parsed_; }
  std::string data_name() const { return data_name_; }
  std::string index_name() const { return index_name_; }

};

#endif /* ASCIDUMPPARSER_H_ */
