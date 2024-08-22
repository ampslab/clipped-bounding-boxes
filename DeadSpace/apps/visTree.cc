/*
 * visTree.cc
 *
 *  Created on: Jan 26, 2016
 *      Author: sidlausk
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

#include <unistd.h>
#include <stdint.h>

#include "Color.h"
#include "VisUtils.h"
#include "SSkyline.h"
#include "ASCIDumpParser.h"
#include "NodeVisualizer.h"
#include "Stats.h"
#include "Utils.h"
#include "SpatialObjects.h"

#define LEAF_LEVEL 0

using namespace std;

static uint32_t ERROR_COUNT = 0;

// To use a different color for each level:
Color colors[] = {
    Color(WHITE_COLOR), Color(YELLOW_COLOR), Color(GREEN_COLOR),
    Color(GREY_COLOR), Color(BLACK_COLOR), Color (BLUE_COLOR),
    Color(RED_COLOR)
};

vector<Box> readQueryFile( string qfile ) {
  vector<Box> queries;
  if ( qfile.empty() )
    return queries;

  std::ifstream ifs( qfile.c_str() );
  std::string line;
  double lo, hi;
  uint32_t line_counter = 0;
  Box raw;
  uint32_t current_dim = 0;
  uint32_t query_cnt = 0;
  while ( getline(ifs, line) ) {
    ++line_counter;

    std::istringstream iss(line);

    if ( !(iss >> lo >> hi) ) {
      printf("Error: txt2vtk failed to read a line!\n");
      break; // error
    }
    assert( lo <= hi );
    raw.lo[current_dim] = lo;
    raw.hi[current_dim] = hi;
    ++current_dim;

    if ( line_counter % NUM_DIMS == 0 ) {
      assert( current_dim == NUM_DIMS );

      ++query_cnt;
      current_dim = 0;
      queries.push_back(raw);
    }
  }
  assert( query_cnt == queries.size() );
  return queries;
}

void visQueryIntersectingNodes(const vector<Box> &qboxes,
    const vector< vector<NodeInfo> > &qnodes, string path, string qfile) {

  if ( qboxes.empty() ) return;

  const uint32_t lastof = qfile.find_last_of('/');
  string qfilename = qfile.substr( lastof );
  qfilename = qfilename.substr(1, qfilename.length() - 4 );

  for (uint32_t q = 0; q < qboxes.size(); ++q) {
    printf(" `-> Visualizing nodes interesting with query #%u\n", q);

    string base_path = path + "/queries/" + qfilename;
    VisUtils::plotBox( qboxes[q], base_path, true, Color(RED_COLOR) );
    const vector<NodeInfo> &nodes = qnodes[q];

    for (uint32_t i = 0; i < nodes.size(); ++i) {
      string temp_path = base_path + "_L" + to_string(nodes[i].level)
          + "_n" + to_string(nodes[i].id) + "_";
      VisUtils::plotBox( nodes[i].mbr, temp_path + to_string(i), true, colors[nodes[i].level] );

      if ( nodes[i].level == LEAF_LEVEL )
        VisUtils::plotBoxes( nodes[i].entries, temp_path + to_string(i)
            + "_recs", false, colors[nodes[i].level] );
    }
  }

}

void printUsage(char *prog) {
  string exec(prog);
  exec = exec.substr( exec.find_last_of('/') + 1 );
  printf("\n%s - Visualizer of R-tree and its variants ASCI dump files\n\n",
      exec.c_str() );
  printf("USAGE: ./%s -i inputfile \n", exec.c_str() );
  printf(" -h: this help message\n");
  printf(" -v: verbose\n");
  printf(" -i: ASCI dump file (.ASC) created using 'A' command\n");
  printf(" -l: lowest level to visualize (default is 1, i.e., the last"
      " level before leaf-level, which is l=0\n");
  printf(" -a: visualize data records (i.e., actual spatial objects)\n");
  printf(" -q: visualize nodes only intersecting with the queries in the given file\n");
  printf("Examples: ");
  printf("./%s -i ../L-tree/dumps/RRS_axo03.RSF.ASC -v -l 0\n", exec.c_str() );
  printf("./%s -i asci_dumps/RRS_axo03.RSF.ASC -l 0 -q asci_dumps/queries/query2_axo03_25697.rs", exec.c_str() );
  printf("\n\n");
}

int main(int argc, char **argv) {

  string inputfile = "";
  uint32_t llevel = 1;
  bool verbose = false;
  string queryfile;

  int c;
  opterr = 0;
  while ((c = getopt(argc, argv, "i:l:q:vh")) != -1) {
    switch (c) {
    case 'h':
      printUsage( argv[0] );
      return 1;
    case 'i':
      inputfile = string(optarg);
      break;
    case 'q':
      queryfile = string(optarg);
      break;
    case 'l':
      llevel = atoi(optarg);
      break;
    case 'v':
      verbose = true;
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

  vector<Box> queries = readQueryFile( queryfile );
  vector<vector<NodeInfo> > queried_nodes( queries.size() );

  ASCIDumpParser parser( inputfile, llevel );

  NodeInfo node;
  map<uint32_t, vector<Box> > levels; // all nodes (their MBBs) at each level

  string path = "vis/" + parser.index_name() + "_" + parser.data_name();
  VisUtils::reset( path + "/", "q" );

  // temp variables for plotting children_of
//  vector<Box> children;
//  vector<Box> children_records;
//  string child_path = "vis/" + parser.index_name() + "_" + parser.data_name()
//      + "/" + "children_of_" + to_string(plot_children_of);

  while ( parser.getNextNode(node) ) {
    if ( node.level < llevel ) continue;

    node.mbr = Box::computeMBR( node.entries );
    if ( levels.count( node.level ) > 0 ) {
      levels[node.level].push_back( node.mbr );
    } else {
      vector<Box> level_boxes;
      level_boxes.push_back( node.mbr );
      levels[node.level] = level_boxes;
    }

    if ( !queries.empty() )
      for (uint32_t q = 0; q < queries.size(); ++q)
        if ( node.mbr.overlap( queries[q] ) )
          queried_nodes[q].push_back( node );

//    if ( node.path_from_root.find( parent_str ) != string::npos ) {
//      children.push_back( node.mbr );
//              VisUtils::plotBoxes( children, child_path + "_L"
//                  + to_string(node.level) + "_c" + to_string(children.size()),
//                  true, colors[node.level] );
//
//      if ( node.level == LEAF_LEVEL && vis_data )
//        VisUtils::addAndPlot( node.entries, false, colors[node.level] );
//    }
//    else if ( node.level == LEAF_LEVEL && vis_data ) {
//      VisUtils::addAndPlot( node.entries, false, colors[node.level] );
//    }

    if ( parser.nodes_parsed() % 1000) {
      // and output to console:
      printf( "Nodes processed: %uk\r", parser.nodes_parsed() / 1000 );
      fflush(stdout);
    }
  }
  printf("\nPrinting nodes at each level..\n");
  for (auto l = levels.begin(); l != levels.end(); ++l) {
      VisUtils::plotBoxes( l->second, path + "/L" + to_string(l->first)
          + "_nodes", true, colors[l->first] );
  }


  visQueryIntersectingNodes( queries, queried_nodes, path, queryfile );

  if (  ERROR_COUNT != 0 )
    printf("ERRORS: %u\n", ERROR_COUNT );

  printf( "\nDONE!\n" );
  printf( "'-> #lines: %ld\n", parser.line_counter() );
  printf( "'-> #visualization files saved: %u\n", VisUtils::files_saved() );
  printf( "'-> #nodes parsed: %u\n", parser.nodes_parsed() );

  return EXIT_SUCCESS;
}
