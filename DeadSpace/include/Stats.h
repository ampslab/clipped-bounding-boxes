/*
 * Stats.h
 *
 *  Created on: Jan 18, 2016
 *      Author: sidlausk
 */

#ifndef STATS_H_
#define STATS_H_

#include <string>
#include <vector>

#include "ASCIDumpParser.h"
#include "SpatialObjects.h"
#include "VisUtils.h"

typedef struct LevelInfo {
  uint32_t num_nodes;
  uint32_t num_dpoints;
  uint32_t capacity; // node capacity
  uint32_t max_dpoints; // max number of deadly points
  double total_volume;
  double total_deadspace;
  double pruned_deadspace;
  std::vector<double> ds_savings;

  LevelInfo(const uint32_t max_num_dpoints) :
      num_nodes(0), num_dpoints(0), capacity(0), max_dpoints(max_num_dpoints),
      total_volume(0.0), total_deadspace(0.0), pruned_deadspace(0.0) {

	  ds_savings.resize( max_dpoints, 0);
  }
} LevelInfo;

typedef std::map<uint32_t, LevelInfo>::iterator LevelInfoIter;

class Stats {
  std::string prefix_;
  uint32_t num_processed_;
  uint32_t num_dir_pages_;
  uint32_t num_data_pages_;
  uint32_t max_dpoints_;
  double tot_volume_abs_; // summed absolute volume of all nodes
  double tot_mbb_deadspace_pc_; // summed mbb dead space (in %) for each node
  double tot_clipped_pc_; // summed clipped dead space (in %) for each node
  double tot_mbc_deadspace_pc_; // summed dead space (in %) for min. encl. sphere
  double tot_ch_deadspace_pc_; // summed dead space (in %) for convex hull
  double tot_rmbb_deadspace_pc_;  // summed dead space (in %) for rotated MBR
  double tot_4c_deadspace_pc_;  // summed dead space (in %) for four-cornered polygon
  double tot_5c_deadspace_pc_;  // summed dead space (in %) for five-cornered polygon
  vector<double> accum_clipped_pc_; // accumulated clipped area (in %) by each clip point
  uint32_t tot_clip_points_;
  uint32_t tot_occupancy_; // total summed node occupancy (i.e., #records)
  std::map<uint32_t, LevelInfo> levels;

  // extra counters:
  uint32_t num_zero_pixel_coverage_nodes_;
  uint32_t num_nan_pixel_coverage_nodes_;
  bool end_vis_sequence_;

public:
  Stats(  std::string prefix = "Collected",
		  const uint32_t max_num_dpoints = NUM_CORNERS):
      prefix_(prefix), num_processed_(0), num_dir_pages_(0), num_data_pages_(0),
	  max_dpoints_(max_num_dpoints), tot_volume_abs_(0.0), tot_mbb_deadspace_pc_(0.0),
	  tot_clipped_pc_(0.0), tot_mbc_deadspace_pc_(0.0), tot_ch_deadspace_pc_(0.0),
	  tot_rmbb_deadspace_pc_(0.0), tot_4c_deadspace_pc_(0.0), tot_5c_deadspace_pc_(0.0),
	  tot_clip_points_(0), tot_occupancy_(0), num_zero_pixel_coverage_nodes_(0), 
	  num_nan_pixel_coverage_nodes_(0), end_vis_sequence_(false) {

	accum_clipped_pc_.resize( max_dpoints_, 0.0 );
  }
  virtual ~Stats();

  void update(const NodeInfo &node);

  void printSilent(bool with_header = false);
  void print() const;
  void visExtremes(const std::string prefix = "");

  double getAvg() const {
    double total_savings = 0.0;
    uint32_t total_nodes = 0;
    for (auto l = levels.begin(); l != levels.end(); ++l) {
      total_savings += l->second.pruned_deadspace;
      total_nodes += l->second.num_nodes;
    }

    return total_savings / total_nodes;
  }
  ;
  double getLeast() const {
    double least_savings = 100.0;
    for (auto l = levels.begin(); l != levels.end(); ++l) {
      const double savings = l->second.pruned_deadspace / l->second.num_nodes;
      least_savings = std::min(savings, least_savings);
    }
    return least_savings;
  }
  ;
  double getMost() const {
    double most_savings = 0.0;
    for (auto l = levels.begin(); l != levels.end(); ++l) {
      const double savings = l->second.pruned_deadspace / l->second.num_nodes;
      most_savings = std::max(savings, most_savings);
    }
    return most_savings;
  }
  ;
  double getAvg(const uint32_t at_level) const {
    auto level = levels.find(at_level);
    return level->second.pruned_deadspace / level->second.num_nodes;
  }
  ;

  const NodeInfo getMostDeadNode();
  const NodeInfo getLeastDeadNode();
  const NodeInfo getMostDeadNodeAt(const uint32_t level);
  const NodeInfo getLeastDeadNodeAt(const uint32_t level);

};

#endif /* STATS_H_ */
