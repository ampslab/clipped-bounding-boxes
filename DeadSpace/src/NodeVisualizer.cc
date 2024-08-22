/*
 * NodeVisualizer.cc
 *
 *  Created on: Jan 18, 2016
 *      Author: sidlausk
 */

#include "NodeVisualizer.h"

#include <algorithm>

std::string NodeVisualizer::CBB_TYPE = "sta";

NodeVisualizer::NodeVisualizer() {
  // TODO Auto-generated constructor stub

}

NodeVisualizer::~NodeVisualizer() {
  // TODO Auto-generated destructor stub
}

Point DPoint2Point(const ClipPoint &dp) {
	  return dp.dpoint;
}

/*
 * Visualize everything we know about this node. Output is stored in vtk files
 * suffixed with a given suffix.
 */
void NodeVisualizer::vis(const NodeInfo &node, string prefix ) {
  const string name_prefix = node.name + "@L" + to_string(node.level);
  const string outfolder = "vis/" + prefix + "/";
  VisUtils::reset(outfolder, "");
  VisUtils::plotBoxes( node.entries, outfolder + name_prefix + "_OBJ["+
      to_string(node.entries.size()) + "]", false, Color(WHITE_COLOR) );
  VisUtils::plotBoxes( node.entries, outfolder + name_prefix + "_OBJ["+
      to_string(node.entries.size()) + "]_trans", true, Color(WHITE_COLOR) );

  VisUtils::plotBoxes(node.data_pixels, outfolder + name_prefix + "_OBJ_voxels",
      false, Color(WHITE_COLOR) );

  Color bound_color = node.id == 0 ? Color(GREEN_COLOR) : Color(BLACK_COLOR);

  // Visualize MBB:
  VisUtils::plotBox( node.mbr, outfolder + name_prefix + "_MBB_trans", true,
      Color(BLACK_COLOR) );
  VisUtils::plotBox( node.mbr, outfolder + name_prefix + "_MBB", false,
      Color(BLACK_COLOR) );
  vector<Point> mbb_vertices;
  mbb_vertices.push_back(node.mbr.lo);
  mbb_vertices.push_back(node.mbr.hi);
  VisUtils::plotPoints( mbb_vertices, outfolder + name_prefix + "_MBB_vertices",
      Color(RED_COLOR) );

//  if (NUM_DIMS == 2) {
//    /* These visualizations works only in 2D */

    // Visualize node's rotated MBB:
    VisUtils::plotPolygon( node.rotated_mbr, outfolder + name_prefix + "_RMBB",
            true, bound_color );

    // Visualize node's convex hull:
    if (NUM_DIMS == 2)
      VisUtils::plotPolygon( node.ch_vertices, outfolder + name_prefix + "_CH",
          true, bound_color );
    else {
      // Visualize the actual CH:
      VisUtils::plotPolygons(node.ch_planes, outfolder + name_prefix + "_CH",
          false, bound_color );
      VisUtils::plotPoints(node.ch_vertices, outfolder + name_prefix + "_CH_vertices["
          + std::to_string(node.ch_vertices.size()) + "]",
          Color(RED_COLOR) );
      VisUtils::plotPolygons(node.ch_planes, outfolder + name_prefix + "_CH_mesh",
          true, bound_color );
      VisUtils::plotBoxes(node.ch_pixels, outfolder + name_prefix + "_CH_voxels",
          false, bound_color );
      // CH mesh's lines one-by-one:
//      for (uint32_t t = 0; t < node.ch_planes.size(); ++t) {
//        vector<Point> triangle = node.ch_planes[t];
//        VisUtils::plotPolygon( triangle, outfolder + name_prefix + "_CH_mesh"
//            + to_string(t), true, bound_color );
//      }
    }

    // Visualize node's 4-corner:
    VisUtils::plotPolygon( node.four_corners, outfolder + name_prefix + "_4-C",
        true, bound_color );

    // Visualize node's 5-corner:
    VisUtils::plotPolygon( node.five_corners, outfolder + name_prefix + "_5-C",
        true, bound_color );

    // Visualize MBC:
    VisUtils::plotPolygon( node.triacontakaihexagon, outfolder + name_prefix + "_MBC",
        true, bound_color );

    // Visualize the actual CBBs:
    for (uint32_t cbb = 0; cbb < node.cbbs.size(); ++cbb) {
      string cbb_name("CBB^" + CBB_TYPE + "-k=" + to_string(cbb+1) );
      VisUtils::plotLines(
          node.cbbs[cbb], outfolder + name_prefix + "_" + cbb_name, bound_color
      );
    }
    // Also visualize full CBB without 'k' in the name:
    string cbb_name("CBB^" + CBB_TYPE );
    if ( node.cbbs.empty() )
      VisUtils::plotBox( node.mbr, outfolder + name_prefix + "_" + cbb_name, true,
            bound_color );
    else
      VisUtils::plotLines(
          node.cbbs.back(), outfolder + name_prefix + "_" + cbb_name, bound_color
      );

//  }

  // Visualizing all (oriented) skyline points
  for (uint32_t i = 0; i < node.skylines.size(); ++i) {
    if (node.skylines[i].empty()) continue;

    vector<Point> sky_points;
    sky_points.resize( node.skylines[i].size() );
    std::transform( node.skylines[i].begin(), node.skylines[i].end(),
    		sky_points.begin(), DPoint2Point );

    bitset<NUM_DIMS> bs(i);
    string sname(outfolder + name_prefix + "_sky" + bs.to_string() +
        "[" + to_string(node.skylines[i].size()) + "]");
    VisUtils::plotPoints(sky_points, sname, Color(RED_COLOR) );
  } // end of visualizing all (oriented) skyline points

//  VisUtils::reset(outfolder, node.name + "_skyline");
//  for (uint32_t i = 0; i < node.skylines.size(); ++i) {
//    const vector<Point> &sky = node.skylines[i];
//    for (uint32_t j = 0; j < sky.size(); ++j) {
//      VisUtils::addAndPlot( sky[j], Color(RED_COLOR) );
//    }
//  }

  // Visualizing all (oriented) stairline points
  for (uint32_t i = 0; i < node.stairs.size(); ++i) {
    if (node.stairs[i].empty()) continue;

    vector<Point> stair_points;
    stair_points.resize( node.stairs[i].size() );
    std::transform( node.stairs[i].begin(), node.stairs[i].end(),
    		stair_points.begin(), DPoint2Point );
    bitset<NUM_DIMS> bs(i);
    string sname(outfolder + name_prefix + "_stair" + bs.to_string() +
        "[" + to_string(node.stairs[i].size()) + "]");
    VisUtils::plotPoints(stair_points, sname, Color(BLUE_COLOR) );
  } // end of visualizing all (oriented) stairline points

//  VisUtils::reset(outfolder, node.name + "_stairs");
//  for (uint32_t i = 0; i < node.stairs.size(); ++i) {
//    const vector<Point> &stair = node.stairs[i];
//    for (uint32_t j = 0; j < stair.size(); ++j) {
//      VisUtils::addAndPlot( stair[j], Color(BLUE_COLOR) );
//    }
//  }

  // Visualizing skyline-based dead MBBs
  VisUtils::reset( outfolder, name_prefix + "_sky-dead-MBRs" );
  for (uint32_t s = 0; s < node.skylines.size(); ++s) {
	  const bitset<NUM_DIMS> corner_code(s);
	  const vector<ClipPoint> &sky_dps = node.skylines[s];
	  if ( sky_dps.size() < 3 )
		  continue;
	  for (uint32_t p = 0; p < sky_dps.size(); ++p) {
		  const Box deadmbr = sky_dps[p].toMBR();
		  VisUtils::clearAndPlot( deadmbr, true, Color(RED_COLOR) );
	  }
  } // end of visualizing skyline-based dead MBBs

  // Visualizing stairline-based dead MBBs
  VisUtils::reset( outfolder, name_prefix + "_sta-dead-MBRs" );
  for (uint32_t s = 0; s < node.stairs.size(); ++s) {
    const bitset<NUM_DIMS> corner_code(s);
    const vector<ClipPoint> &sta_dps = node.stairs[s];
    for (uint32_t p = 0; p < sta_dps.size(); ++p) {
      const Box deadmbr = sta_dps[p].toMBR( );
      VisUtils::clearAndPlot(deadmbr, true, Color(BLUE_COLOR) );
    }
  } // end of visualizing stairline-based dead MBBs

  // Visualizing removed dead space by each (chosen) clip point
  VisUtils::reset( outfolder, name_prefix + "_clipped", false );
  for (uint32_t i = 0; i < node.clip_points.size(); ++i) {
	  const Box deadmbr = node.clip_points[i].toMBR( );
	  VisUtils::clearAndPlot( deadmbr, false, Color(YELLOW_COLOR) );
  } // end of visualizing actually removed dead space by each (chosen) clip point

  // Visualize only clip points:
  vector<Point> clip_points;
  clip_points.resize( node.clip_points.size() );
  std::transform( node.clip_points.begin(), node.clip_points.end(),
      clip_points.begin(), DPoint2Point );
  string sname(outfolder + name_prefix + "_CPs" +
      "[" + to_string(node.clip_points.size()) + "]");
  VisUtils::plotPoints(clip_points, sname, Color(GREEN_COLOR) );
}

