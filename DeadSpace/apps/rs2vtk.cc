/*
 * txt2RS.cc
 *
 *  Created on: Jan 26, 2016
 *      Author: sidlausk
 */

#include <string>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <cassert>

#include <unistd.h>

#include "Color.h"
#include "VisUtils.h"
#include "SpatialObjects.h"

using namespace std;

void printUsage(char *prog) {
  string exec( prog );
  exec = exec.substr( exec.find_last_of('/') + 1 );
  printf("\n%s---Visualizes rectangles stored in RStar text (ASCI) files.\n"
      "That is, the input (.rs) file must be in the RStar format, i.e.:\n"
      "  x_low x_high\n"
      "  y_low y_high\n"
      "  ...\n"
      "\n",
      exec.c_str() );
  printf( "USAGE: ./%s -i input-file [-o output-file -v -h]\n", exec.c_str() );
  printf( " -h: this help message\n");
  printf( " -i: input file \n");
  printf( " -o: output directory. Default: the same as of input file.\n");
  printf( " -a: all rectangles in one file. Default: one rectangle per file\n");
  printf( " -f: fill rectangles. Default: just bounds, not filled\n");
  printf( "Example: ");
  printf( "./%s -i test_data_queries.rs\n\n", exec.c_str());
}

int main(int argc, char **argv) {
  std::string infile, outdir;
  bool all = false, transparent = true;

  int c;
  opterr = 0;
  while ((c = getopt(argc, argv, "i:o:haf")) != -1) {
    switch (c) {
    case 'h':
      printUsage( argv[0] );
      return 1;
    case 'i':
      infile = std::string(optarg);
      break;
    case 'o':
      outdir = std::string(optarg);
      break;
    case 'a':
      all = true;
      break;
    case 'f':
      transparent = false;
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

  if (NUM_DIMS > 3) {
    printf( "ERROR: rs2vtk works only with 2D and 3D data!\n" );
    return EXIT_FAILURE;
  }

  std::ifstream ifs( infile.c_str() );
  if ( outdir.empty() ) {
    string file_name = infile.substr( infile.find_last_of("/\\") + 1 );
    file_name = file_name.substr( 0, file_name.find_last_of(".") );
    outdir = infile.substr( 0, infile.find_last_of("/\\") + 1 );
    VisUtils::reset( outdir, file_name );
  } else {
    VisUtils::reset( outdir, "query" );
  }

  std::string line;
  double lo, hi;

  uint32_t line_counter = 0;
  // read node header
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
      if ( all )
    	  VisUtils::addAndHold( raw, transparent, Color(RED_COLOR) );
      else
    	  VisUtils::clearAndPlot( raw, transparent, Color(RED_COLOR) );

      ++query_cnt;
      current_dim = 0;
    }
  }
  if ( all )
	  VisUtils::plotAllin1();

  printf( "Number of lines read: %d\n", line_counter );
  printf( "Number of queries plotted: %d\n", query_cnt );
  printf( "Output directory: %s\n", outdir.c_str() );
  printf("DONE!\n");

  return EXIT_SUCCESS;
}

