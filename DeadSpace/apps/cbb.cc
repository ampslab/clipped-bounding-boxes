/*
 * staircaseDeadSpace.cc
 *
 *  Created on: Jan 12, 2016
 *      Author: darius
 */

#include <cstdlib>
#include <cstdio>
#include <string>
#include <fstream>
#include <sstream>
#include <cassert>
#include <vector>
#include <set>
#include <utility>
#include <bitset>
#include <map>
#include <iomanip>
#include <algorithm>

#include <unistd.h>
#include <stdint.h>

#include "Color.h"
#include "VisUtils.h"
#include "SSkyline.h"
#include "ASCIDumpParser.h"
#include "NodeVisualizer.h"
#include "Stats.h"
#include "Utils.h"

using namespace std;

static uint32_t ERROR_COUNT = 0;

set<uint32_t> findL1NormNeighbours( vector<Point> &points,
    const uint32_t ref ) {
  set<uint32_t> res;

  uint32_t next = ref + 1;
  double min_l1norm_val[NUM_DIMS];
  uint32_t min_l1norm_idx[NUM_DIMS];

  // initial distances
  for (uint32_t d = 0; d < NUM_DIMS; ++d) {
    min_l1norm_idx[d] = next;
    min_l1norm_val[d] = fabs( points[ref][d] - points[next][d] );
  }

  for ( uint32_t p = next + 1; p < points.size(); ++p )
    for (uint32_t d = 0; d < NUM_DIMS; ++d) {
      const double next_dist = fabs( points[ref][d] - points[p][d] );
      if ( min_l1norm_val[d] > next_dist ) {
        min_l1norm_val[d] = next_dist;
        min_l1norm_idx[d] = p;
      }
    }

  for (uint32_t d = 0; d < NUM_DIMS; ++d) {
      /*auto ret = */res.insert( min_l1norm_idx[d] );
//      if ( !ret.second ) {
//        printf("The element already existed!\n");
//      }
  }

  return res;
}

/*
 * First, sorts the points regularly. Then, takes the first point as a reference
 * point and finds its nearest neighbor. The NN point becomes the 2nd point in
 * the order. Next, the previous found NN point becomes a reference point and a
 * NN point (from the remaining points) is found, which becomes the 3rd point
 * in the order. And so on..
 *
 * Warning: does not deliver what we want!
 */
void sortByDistance(vector<Point> &points) {
  if ( points.size() < 2 ) return;

  std::sort( points.begin(), points.end() );
  for ( uint32_t ref = 0; ref < points.size() - 1; ++ref ) {
    uint32_t closest = ref + 1;
//    double min_dist = points[ref].manhattan_distance( points[closest] );
    double min_dist = points[ref].euclidean_distance( points[closest] );
    for ( uint32_t nxt = closest + 1; nxt < points.size(); ++nxt ) {
//      double dist = points[ref].manhattan_distance( points[nxt] );
      double dist = points[ref].euclidean_distance( points[nxt] );
      if (dist < min_dist) {
        min_dist = dist;
        closest = nxt;
      }
    }
    std::swap( points[ref+1], points[closest] );
  }
}

void printDPoint(const ClipPoint &dp) {
	printf(" (%u <", dp.corner_code);
	for (uint32_t d = 0; d < NUM_DIMS; ++d) {
		printf( "%.2f", dp.ccoord[d] );
		if ( d+1 < NUM_DIMS )
			printf(",");
	}
	printf("> <");
	for (uint32_t d = 0; d < NUM_DIMS; ++d) {
		printf( "%.2f", dp.dpoint[d] );
		if (d+1 < NUM_DIMS)
			printf(",");
	}
	printf("> ");
	printf( " sco=%f", dp.score );

}

void printDPoints(const vector<ClipPoint> &dpoints, const string &prefix) {

	printf( "%s", prefix.c_str() );
	for (uint32_t i = 0; i < dpoints.size(); ++i) {
		printf("  dp%d ", i);
		printDPoint( dpoints[i] );
		printf(")\n");
	}
}

/*
 * Computes all staircase-dead-space savings.
 */
void computeMCRSavings( NodeInfo &node, bool staircase ) {
	vector<ClipPoint> &dpoints = node.clip_points;
	if ( dpoints.empty() )
		return;

//	const double total_vol = node.mbr.volume();

	std::sort( dpoints.begin(), dpoints.end() );
	assert( dpoints.size() <= node.max_dpoints );

//	node.accum_abs_chamfered[0] = dpoints[0].score;
//	for (uint32_t p = 1; p < node.max_dpoints; ++p) {
//		if ( p < dpoints.size() )
//			node.accum_abs_chamfered[p] += (node.accum_abs_chamfered[p-1] + dpoints[p].score);
//		else
//			node.accum_abs_chamfered[p] = node.accum_abs_chamfered[p-1];
//	}
//
//	// convert to %
//	for (uint32_t p = 0; p < node.max_dpoints; ++p) {
//		node.accum_abs_chamfered[p] = node.accum_abs_chamfered[p] / total_vol * 100.0;
//	}
//
//	node.total_chamfered_percent = node.accum_abs_chamfered.back() / total_vol * 100.0;
}

vector<vector<Box> > computeDeadMBRs(const Box &mbr,
    const vector<vector<Point> > &skies) {
  vector<vector<Box> > deadspace;
  deadspace.resize( skies.size() );

  for (uint32_t s = 0; s < skies.size(); ++s) {
    const Point curr_corner = mbr.getCorner( s );

    const vector<Point> &sky = skies[s];
    for (uint32_t p = 0; p < sky.size(); ++p) {
      double vol = curr_corner.volumeTo( sky[p] );
      if (vol == 0) continue;

      Box deadmbr = { curr_corner, sky[p] };
      for (uint32_t d = 0; d < NUM_DIMS; ++d) {
        if ( sky[p][d] <= curr_corner[d] ) {
          deadmbr.lo = sky[p];
          deadmbr.hi= curr_corner;
          break;
        }
      }
      deadspace[s].push_back( deadmbr );
    }
  }
  return deadspace;
}

void printSkies(FILE* out_sky, const uint32_t nid,
    const vector<vector<Point> > &skies) {
  long nbytes = 0;

  nbytes += fprintf( out_sky, "%u\n", nid );
  for (uint32_t s = 0; s < skies.size(); ++s) {
    nbytes += fprintf( out_sky, " %lu: ", skies[s].size() );
    for (uint32_t i = 0; i < skies[s].size(); ++i) {
      const Point &p = skies[s][i];
      nbytes += fprintf( out_sky, "< " );
      for (uint32_t d = 0; d < NUM_DIMS; ++d)
        nbytes += fprintf( out_sky, "%.2f ", p[d] );
//        nbytes += fprintf( out_sky, "% .15e ", p[d] );
      nbytes += fprintf( out_sky, ">" );

    }
    nbytes += fprintf( out_sky, "\n" );
  }
}

void printStairs(FILE* out_stair, const uint32_t nid,
    const vector<vector<Point> > &stairs) {
  long nbytes = 0;

  nbytes += fprintf( out_stair, "%u\n", nid );
  for (uint32_t s = 0; s < stairs.size(); ++s) {
    nbytes += fprintf( out_stair, " %lu: ", stairs[s].size() );
    for (uint32_t i = 0; i < stairs[s].size(); ++i) {
      const Point &p = stairs[s][i];
      nbytes += fprintf( out_stair, "< " );
      for (uint32_t d = 0; d < NUM_DIMS; ++d)
        nbytes += fprintf( out_stair, "%.2f ", p[d] );
//        nbytes += fprintf( out_sky, "% .15e ", p[d] );
      nbytes += fprintf( out_stair, ">" );

    }
    nbytes += fprintf( out_stair, "\n" );
  }
}

void printDeadlyPoints(FILE* out_file, const uint32_t nid,
    const vector<ClipPoint> &dpoints) {

  if ( dpoints.empty() ) return;

  long nbytes = 0;

  nbytes += fprintf( out_file, "%u:", nid );

  for (uint32_t s = 0; s < dpoints.size(); ++s) {
    ClipPoint dp = dpoints[s];
    nbytes += fprintf(out_file, " %d (", dp.corner_code);
    for (uint32_t d = 0; d < NUM_DIMS; ++d) {
//      nbytes += fprintf( out_file, " (%f, %f)",
//          std::min(dp.corner[d], dp.deadly_point[d]),
//          std::max(dp.corner[d], dp.deadly_point[d]) );
      nbytes += fprintf( out_file, "%f", dp.dpoint[d] );
      if (d != NUM_DIMS - 1)
        nbytes += fprintf( out_file, " " );
      else
        nbytes += fprintf( out_file, ")" );
    }
//    nbytes += fprintf( out_file, " [%.5f]", dp.vol_abs );
    nbytes += fprintf( out_file, ";" );
  }
  nbytes += fprintf( out_file, "\n" );
}

vector<ClipPoint> filterAndScore( vector<ClipPoint> &DPs,
		const double kNodeVol ) {

	// Calculate scores based on the deadliest candidate:
	vector<ClipPoint> best_candidates;

	uint32_t max_vol_idx = 0;
	double max_vol = 0.0;
	for (uint32_t i = 0; i < DPs.size(); ++i) {
		assert( DPs[i].score != -1.0 );
		if ( DPs[i].score / kNodeVol > DP_MIN_DSPACE ) {
			best_candidates.push_back( DPs[i] );
			// track max volume candidate
			if ( max_vol < DPs[i].score ) {
				max_vol = DPs[i].score;
				max_vol_idx = best_candidates.size() - 1;
				assert( max_vol == best_candidates.back().score );
			}
		}
	}

	if ( best_candidates.size() < 2 )
		return best_candidates;

	// Make largest the first:
	std::swap( best_candidates.at(max_vol_idx), best_candidates.at(0) );
	const Box largest_dMBR( best_candidates[0].dpoint,
			best_candidates[0].ccoord );
	assert( best_candidates[0].score == largest_dMBR.volume() );

	uint32_t next = 1, tail = best_candidates.size() - 1;
	while (next <= tail) {

		Box next_dspace( best_candidates[next].dpoint, best_candidates[next].ccoord );
		double overlap = next_dspace.overlappingBox( largest_dMBR ).volume();
		assert( overlap > 0 && overlap < best_candidates[next].score ); // or <=
		best_candidates[next].score -= overlap;

		if ( best_candidates[next].score / kNodeVol < DP_MIN_DSPACE ) {
			std::swap( best_candidates[next], best_candidates[tail] );
			best_candidates.pop_back();
			--tail;
		} else ++next;
	}

	std::sort( best_candidates.begin(), best_candidates.end() );

	return best_candidates;
}

void printUsage(char *prog) {
  printf("\n%s - computes clipped bounding box (CBB) for each node in an RStar"
		  "tree dump.\n\n", prog);
  printf("USAGE: ./%s -i ASCI_treedump [-h | -v | -o | -m max_#dpoins | -s [skyline|staircase]] | -l llevel | -n 'list of nodes ids']\n", prog);
  printf(" -h: this help message\n");
  printf(" -v: verbose\n");
  printf(" -s: skyline variant (default: staircase)\n");
  printf(" -o: write output file for skyline, staircase, and clip points (flag)\n");
  printf(" -i: input file with (.RSF.ASC) extension\n");
  printf(" -l: lowest level that is considered for clipping (default: 0, i.e., all levels from root- to leaf-level\n");
  printf(" -n: string of node ids that will be visualized. Note: root node id =0,\n"
         "     non-leaf node id = [1..m], leaf node id = [1..n]. As such, there \n"
         "     might be two nodes with the same id.\n");
  printf(" -m: max number of clip points considered per node (default: #corners)\n");
  printf("Example: ");
  printf("%s -v -i ../L-tree/test_data/test_data.RSF.ASC -n \"0 1 2\" -o\n\n",
		  prog);
}

/*
 * Computes Clipped Bounding Box(CBB) of a node.
 *
 * This where all the skyline- and stair-line based clipping is happening.
 */
void computeCBB( NodeInfo &node, bool staircase ) {

	for (uint32_t c = 0; c < NUM_CORNERS; ++c) {
		vector<ClipPoint> &cskyline = node.skylines[c];

		bitset<NUM_DIMS> bs(c);
		const Point ccoord = node.mbr.getCorner(c);
		vector<ClipPoint> temp_points; // deadly candidates
		for (uint32_t i = 0; i < node.entries.size(); ++i) {
			temp_points.push_back( ClipPoint(c, ccoord, node.entries[i].getCorner(c) ) );
		}

		cskyline = SSkyline::skyline( temp_points );
		assert( cskyline.size() > 0 && cskyline.size() <= node.entries.size() );
//		printDPoints( cskyline, "SKY:\n" );

		vector<ClipPoint> deadline;
		if ( staircase ) {
			vector<ClipPoint> &cstairline = node.stairs[c];
			cstairline = SSkyline::staircase( cskyline, node.mbr.volume() );
			assert( cstairline.size() == node.stairs[c].size() );
//			printDPoints( cstairline, "STA:\n" );
			deadline = filterAndScore( cstairline, node.mbr.volume() );
		} else {
			deadline = filterAndScore( cskyline, node.mbr.volume() );
		}

		if ( deadline.empty() ) continue;

		// There are potential candidates from this node, store them:
		uint32_t cnt = 0;
		if ( node.clip_points.size() < node.max_cpoints ) {
			// First, fill the empty spots, if possible:

			for (; cnt < deadline.size() && node.clip_points.size() < node.max_cpoints; ++cnt)
				node.clip_points.push_back( deadline[cnt] );

			std::sort(node.clip_points.begin(), node.clip_points.end());
			// Then, replace only if a more 'deadly' candidate exists
			while ( cnt < deadline.size() &&
					deadline[cnt].score > node.clip_points.back().score ) {
				std::swap( node.clip_points.back(), deadline.at(cnt) );
				std::sort(node.clip_points.begin(), node.clip_points.end());
				++cnt;
			}
		} else { // Replace only a more 'deadly' candidate exists
			while ( cnt < deadline.size() &&
					deadline[cnt].score > node.clip_points.back().score ) {
				std::swap( node.clip_points.back(), deadline.at(cnt) );
				std::sort(node.clip_points.begin(), node.clip_points.end());
				++cnt;
			}
		}
//		printDPoints( node.deadly_points, "CURRENT DPs:\n" );
	} // END OF FOR EACH CORNER
}

int main(int argc, char **argv) {

  string inputfile = "", variant_name = "staircase";
  uint32_t llevel = 0;
  uint32_t max_dpoints = NUM_CORNERS;
  bool vis = false;
  vector<uint32_t> nodeids;
  bool verbose = false, write_output = false, staircase = true;

  FILE *out_stair = NULL;
  uint32_t num_zero_vol_cbbs = 0;

  int c;
  opterr = 0;
  while ((c = getopt(argc, argv, "i:l:n:m:ovsh")) != -1) {
    switch (c) {
    case 'h':
      printUsage( argv[0] );
      return 1;
    case 'i':
      inputfile = string(optarg);
      break;
    case 'm':
      max_dpoints = atoi(optarg);
      break;
    case 'l':
      llevel = atoi(optarg);
      break;
    case 'n': {
      istringstream sin( (string(optarg)) );
      uint32_t id = 0;
      while (sin >> id) {
        nodeids.push_back( id );
      }
      vis = true;
      assert( !nodeids.empty() );
      break;
    }
    case 'o':
      write_output = true;
      break;
    case 'v':
      verbose = true;
      break;
    case 's':
      staircase = false;
      variant_name = "skyline";
      NodeVisualizer::CBB_TYPE = "sky";
      break;
    default:
      fprintf( stderr, "Unknown option `-%c'.\n", optopt);
      printUsage( argv[0] );
      return 1;
    }
  }

  if (argc == 1 || optind != argc) {
    printf("argc = %d, optind = %d\n", argc, optind);
    printf("next remaining opt: %s \n", argv[optind]);
    printUsage( argv[0] );
    return 1;
  }

  if ( write_output ) {
    // open files for output writing:
    string infileprefix = inputfile.substr(0, inputfile.find_last_of('.') - 1 );
    string outfile = infileprefix + "_"  + (staircase ? "MCR-sta" : "MCR-sky") + "-dspace.txt";
    out_stair = fopen(outfile.c_str(), "w" );
    printf( "Output file with deadly points: %s\n", outfile.c_str() );
  }

  ASCIDumpParser parser( inputfile, llevel );
  assert( parser.num_dims() == NUM_DIMS );

  std::string outfilename = parser.index_name() + "_" + parser.data_name()
      + (staircase ? "_CBB-sta.csv" : "_CBB-sky.csv");
  std::ofstream ofs( outfilename );
  // Output file header:
  ofs << "node level #entries exact-coverage pixel-coverage "
         "MBB RMBR MBC MBE 4-C 5-C CH |CH| CBB |CPs|";
  // Dead space with increasing k for each CBB
  for (uint32_t c = 0; c < max_dpoints; ++c) {
    ofs << " CBB_" << c + 1;
  }
  ofs << endl;
  ofs << std::fixed << std::setprecision(18);

  NodeInfo node( max_dpoints );
  NodeInfo least_chamfered( max_dpoints ), most_chamfered( max_dpoints );
  least_chamfered.total_chamfered_percent = numeric_limits<double>::max();

  // Few vars for measuring some statistics globally:
  const string prefix( "MCR-sta_" + parser.index_name() + "_"
      + parser.data_name() );
  Stats stats( prefix, max_dpoints );
//  bool reached_bottom = false;

  uint32_t max_dir_node = numeric_limits<uint32_t>::min();
  uint32_t max_dat_node = numeric_limits<uint32_t>::min();
  uint32_t min_dir_node = numeric_limits<uint32_t>::max();
  uint32_t min_dat_node = numeric_limits<uint32_t>::max();
  uint32_t chamfered_count = 0;

  while ( parser.getNextNode(node) ) {
    if (node.level == 0) {
      min_dat_node = std::min( node.id, min_dat_node );
      max_dat_node = std::max( node.id, max_dat_node );
    } else {
      min_dir_node = std::min( node.id, min_dir_node );
      max_dir_node = std::max( node.id, max_dir_node );
    }

    if ( node.level < llevel ) continue;
    if ( vis && std::find(nodeids.begin(), nodeids.end(), node.id) == nodeids.end() )
      continue;

    node.mbr = Box::computeMBR( node.entries );

    computeCBB( node, staircase );

    computeMCRSavings( node, staircase );

#ifndef NDEBUG
    Utils::computeDeadspace( node, true );
#else
    Utils::computeDeadspace( node, false );
#endif

    stats.update( node );

    /* VISUALIZE EVERYTHING ABOUT THIS NODE! */
    if ( vis && std::find(nodeids.begin(), nodeids.end(), node.id) != nodeids.end() ) {
      // For visualizing all possible CBBs:
      for (uint32_t k = 1; k <= node.clip_points.size(); ++k) {
        node.cbbs.push_back(
             Utils::cbb2lines( node.mbr, node.clip_points, k )
        );
      }
      string suffix = parser.index_name() + "_";
      NodeVisualizer::vis( node, suffix );
    }
    /* END OF VISUALIZE EVERYTHING ABOUT THIS NODE! */


    // Saving measurements (% of dead space) to file:
#if NUM_DIMS == 2
    const double obj_coverage = node.exact_coverage_abs;
#else
    const double obj_coverage = node.pixel_coverage_abs;
#endif
    //     "node level #entries"
    ofs << node.id << " " << node.level << " " << node.size << " "
        // exact-coverage pixel-coverage
        << node.exact_coverage_abs << " " << node.pixel_coverage_abs << " "
        // "MBR"
        << node.mbr.volume() << " "
//        << (node.mbr.volume() - obj_coverage) / node.mbr.volume() << " "
        // "RMBR";
        << node.area_min_rotated_mbr_abs << " "
//        << (node.area_min_rotated_mbr_abs - obj_coverage) / node.area_min_rotated_mbr_abs << " "
        // "MBC"
        << node.area_min_enclosing_circle_abs << " "
//        << (node.area_min_enclosing_circle_abs - obj_coverage) / node.area_min_enclosing_circle_abs << " "
        // "MBE"
        << 100.0 << " "
//        << (node.area_min_enclosing_ellipse_abs - obj_coverage) / node.area_min_enclosing_ellipse_abs << " "
        // 4-C
        << node.area_four_corners << " "
//        << (node.area_four_corners - obj_coverage) / node.area_four_corners << " "
        // 5-C
        << node.area_five_corners << " "
//        << (node.area_five_corners - obj_coverage) / node.area_five_corners << " "
        // "CH"
        << node.convex_hull_area << " "
//        << (node.convex_hull_area - obj_coverage) / node.convex_hull_area << " "
        // "|CH|" --- #points in CH
        << node.ch_size << " ";

    const double CBB_vol = node.mbr.volume() - node.accum_clipped_abs.back();
    assert( CBB_vol >= 0 );
    if ( CBB_vol > 0 ) {
      // "CBB" #clip-points"
      ofs << CBB_vol << " " << node.clip_points.size();
//      ofs << (CBB_vol - obj_coverage) / ( CBB_vol ) << " " << node.clip_points.size();
      for (uint32_t c = 0; c < max_dpoints; ++c) {
        const double cbb_k_vol = node.mbr.volume() - node.accum_clipped_abs[c];
        ofs << " " << cbb_k_vol;
//        ofs << " " << (cbb_k_vol - obj_coverage) / cbb_k_vol;
      }
    } else {
      /*
       *  With exact volume calculation Currently only in 2D) this branch is
       *  never executed.
       *
       *  While very rarely, with pixel-based volume calculation (used in 3D),
       *  this branch is sometimes executed.. It basically means clipping is
       *  very effective and CBB volume is very small. I just assume it's 1% of
       *  the original MBB.
       *
       */

      // "CBB" #clip-points"
      ofs << node.mbr.volume() * 0.01 << " " << node.clip_points.size();
      for (uint32_t c = 0; c < max_dpoints; ++c)
        ofs << " " << node.mbr.volume() * 0.01;
      ++num_zero_vol_cbbs;
    }

    ofs << endl;

    if ( verbose && parser.nodes_parsed() % 1000) {
      // and output to console:
      printf( "Nodes processed: %uk\r", parser.nodes_parsed() / 1000 );
      fflush(stdout);
    }

    if ( write_output ) {
      printDeadlyPoints( out_stair, node.id, node.clip_points );
    }
    chamfered_count += (!node.clip_points.empty());

    if ( node.level == 0 &&
    		node.total_chamfered_percent < least_chamfered.total_chamfered_percent )
    	least_chamfered = node;
    if ( node.level == 0 &&
    		node.total_chamfered_percent > most_chamfered.total_chamfered_percent )
    	most_chamfered = node;
  } // GET NEXT NODE

  printf("\n");
  if (verbose) {
	  stats.printSilent( true );
//    stats.print();

//    string suffix = parser.index_name() + "_leastC@L0";
//    NodeVisualizer::vis( least_chamfered, suffix );
//    suffix = parser.index_name() + "_mostC@L0";
//    NodeVisualizer::vis( most_chamfered, suffix );

    printf( "DONE!\n" );
    printf( "'-> #lines: %ld\n", parser.line_counter() );
    printf( "'-> #visualization files saved: %u\n", VisUtils::files_saved() );
    printf( "'-> #nodes parsed: %u\n", parser.nodes_parsed() );
    printf( "'-> #clipped: %u (%.2f%%)\n", chamfered_count,
    		chamfered_count * 100.0 / parser.nodes_parsed() );
    printf( "'-> #zero-volume CBBs: %u\n", num_zero_vol_cbbs );
    printf( "DIR node [min--max] ids: [%u--%u]\n", min_dir_node, max_dir_node );
    printf( "DAT node [min--max] ids: [%u--%u]\n", min_dat_node, max_dat_node );
    printf( "All stats about each node are written to %s\n", outfilename.c_str() );
  }

  if (  ERROR_COUNT != 0 )
    printf("ERRORS: %u\n", ERROR_COUNT );

  ofs.close();

  if ( write_output ) {
    fclose(out_stair);
  }

  return EXIT_SUCCESS;
}
