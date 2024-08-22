/*
 * Stats.cc
 *
 *  Created on: Jan 18, 2016
 *      Author: sidlausk
 */

#include "Stats.h"

#include "NodeVisualizer.h"

Stats::~Stats() {
	levels.clear();
}

void Stats::update(const NodeInfo &node) {
  if (node.level == 0) {
    ++num_data_pages_;
  } else {
    ++num_dir_pages_;
  }

  assert( node.mbr.volume() > 0.0 );

  if ( std::isnan( node.pixel_coverage_abs ) )
    ++num_nan_pixel_coverage_nodes_;

//  if (!end_vis_sequence_ && node.level == 0) {
//    VisUtils::clearAndAdd( node.mbr, true, Color(GREEN_COLOR) );
//    VisUtils::addAndHold( node.entries, true, Color(GREEN_COLOR) );
//  }

  if ( node.pixel_coverage_abs == 0 ) {
	  ++num_zero_pixel_coverage_nodes_;
	  if ( num_zero_pixel_coverage_nodes_ < 10 )
	    printf(" `-> NodeID with zero dead space: %u (level=%u)\n",
	        node.id, node.level);
//    exit( EXIT_FAILURE );
  }

  tot_volume_abs_ += node.mbr.volume();

  tot_mbb_deadspace_pc_ +=
      ( ( node.mbr.volume() - node.pixel_coverage_abs) * 100.0
        / node.mbr.volume() );
  tot_mbc_deadspace_pc_ += 
  	( node.spherical_dead_space * 100.0 / node.area_min_enclosing_circle_abs );
  tot_ch_deadspace_pc_ += 
  	( ( node.convex_hull_area - node.exact_coverage_abs ) * 100.0 
  		/ node.convex_hull_area
  	);
  tot_rmbb_deadspace_pc_ +=
  	( ( node.area_min_rotated_mbr_abs - node.exact_coverage_abs ) * 100.0 
  		/ node.area_min_rotated_mbr_abs
  	);
  tot_4c_deadspace_pc_ += 
  	( ( node.area_four_corners - node.exact_coverage_abs ) * 100.0 
  		/ node.area_four_corners
  	);
  tot_5c_deadspace_pc_ += 
  	( ( node.area_five_corners - node.exact_coverage_abs ) * 100.0 
  		/ node.area_five_corners
  	);
  tot_clipped_pc_ += node.accum_clipped_abs.back();

  tot_clip_points_ += node.clip_points.size();
  tot_occupancy_ += node.entries.size();

  if ( levels.count( node.level ) > 0 ) {
	  LevelInfoIter lii = levels.find(node.level);
	  assert( lii != levels.end() );
	  lii->second.num_nodes++;
	  lii->second.total_volume += node.mbr.volume();
//	  lii->second.total_deadspace += node.total_deadspace_abs;
	  lii->second.num_dpoints += (node.clip_points.size());
	  lii->second.capacity += node.entries.size();
	  lii->second.pruned_deadspace += node.total_chamfered_percent;
	  for ( uint32_t c = 0; c < node.accum_clipped_abs.size(); ++c )
    	lii->second.ds_savings[c] += node.accum_clipped_abs[c];
  } else {
	  LevelInfo li( max_dpoints_ );
	  li.num_nodes = 1;
	  li.total_volume = node.mbr.volume();
//	  li.total_deadspace = node.total_deadspace_abs;
	  li.num_dpoints = (node.clip_points.size() );
	  li.capacity = node.entries.size();
	  li.pruned_deadspace = node.total_chamfered_percent;
	  assert( node.accum_clipped_abs.size() == max_dpoints_ && li.ds_savings.size() == max_dpoints_ );
	  for ( uint32_t c = 0; c < max_dpoints_; ++c )
		  li.ds_savings[c] = node.accum_clipped_abs[c];

	  std::pair<LevelInfoIter, bool> res = levels.insert( std::make_pair(node.level, li) );
	  assert( res.second == true );
  }

  for (uint32_t p = 0; p < node.max_cpoints; ++p)
	  accum_clipped_pc_[p] += ( node.accum_clipped_abs[p] * 100.0 / node.mbr.volume() );

  ++num_processed_;
}

void Stats::printSilent(bool with_header) {
	// HEADER:
	if (with_header) {
		printf( "%9s %9s %9s %12s %12s %12s %12s %12s %12s %12s",
				"#nodes", "avg_size", "avg_#DPs", "avg_mbb(%)", "avg_mbc(%)",
				"avg_ch(%)", "avg_rmbb(%)", "avg_4-c(%)", "avg_5-c(%)",
				"avg_cbb(%)" );
		for (uint32_t c = 0; c < accum_clipped_pc_.size(); ++c)
			printf("%9s%u", "cbb-k=", c+1);
		printf("\n");
	}

	printf("%9u %9.1f %9.1f %12.3f %12.3f %12.3f %12.3f %12.3f %12.3f %12.3f", num_processed_, // #nodes processed
	  tot_occupancy_ / (double) num_processed_, 	  // avg. node size
	  tot_clip_points_ / (double) num_processed_, 	  // --//-- #clip points
	  tot_mbb_deadspace_pc_ / (double) num_processed_,      // --//-- %dead-space
	  tot_mbc_deadspace_pc_ / (double) num_processed_, // --//-- %dead-space (spheres)
	  tot_ch_deadspace_pc_ / (double) num_processed_,   // --//-- %dead-space (convex hull)
	  tot_rmbb_deadspace_pc_ / (double) num_processed_,   // --//-- %dead-space (rotated mbr)
	  tot_4c_deadspace_pc_ / (double) num_processed_,   // --//-- %dead-space (4 corners)
	  tot_5c_deadspace_pc_ / (double) num_processed_,   // --//-- %dead-space (5 corners)
	  (tot_mbb_deadspace_pc_ - accum_clipped_pc_.back()) / num_processed_ );	  // --//-- %clipped (total)
	for (uint32_t c = 0; c < accum_clipped_pc_.size(); ++c)
	  printf( " %9.4f", (tot_mbb_deadspace_pc_ - accum_clipped_pc_[c]) / num_processed_ );
	printf("\n");

//  printf( "'-> #data pages: %u\n", num_data_pages_ );
//  printf( "'-> #dir pages: %u\n", num_dir_pages_ );
//  printf( "'-> #pages: %u\n", num_dir_pages_ + num_data_pages_ );
  if ( num_zero_pixel_coverage_nodes_ != 0 )
    printf( "'-> zero-dead-space nodes: %u\n", num_zero_pixel_coverage_nodes_ );
  if ( num_nan_pixel_coverage_nodes_ != 0 )
    printf( "'-> nan-dead-space nodes: %u\n", num_nan_pixel_coverage_nodes_ );

//  assert( tot_clipped_pc_ / num_processed_
//      == accum_clipped_pc_.back() / num_processed_ );
}

