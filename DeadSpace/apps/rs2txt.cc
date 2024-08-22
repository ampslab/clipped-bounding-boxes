/*
 * asc2txt.cc
 *
 *  Created on: Jan 26, 2016
 *      Author: sidlausk
 */

#include <string>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include <unistd.h>

#include "Color.h"
#include "VisUtils.h"
#include "SpatialObjects.h"

using namespace std;

void printUsage(char *prog) {
  string exec( prog );
  exec = exec.substr( exec.find_last_of('/') + 1 );
  printf("\n%s---Converts RStar ASCI files (.rs) to .txt files understood.\n"
      "by SpatialIndexing library. That is, translates this textual format:\n"
      "  x_low x_high\n"
      "  y_low y_high\n"
      "  ...\n"
      "to this:\n"
      "  x_low y_low x_high y_high\n"
      "  ...\n"
      "\n",
      exec.c_str() );
  printf( "USAGE: ./%s -i input-file [-o output-file -v -h]\n", exec.c_str() );
  printf( " -h: this help message\n" );
  printf( " -i: input file \n" );
  printf( " -o: output file. Default: input-file-name.txt \n" );
  printf( "Example: ");
  printf( "./%s -i test_data_queries.txt\n\n", exec.c_str() );
}

int main(int argc, char **argv) {
  std::string infile, outfile;

  int c;
  opterr = 0;
  while ((c = getopt(argc, argv, "i:o:h")) != -1) {
    switch (c) {
    case 'h':
      printUsage( argv[0] );
      return 1;
    case 'i':
      infile = std::string(optarg);
      break;
    case 'o':
      outfile = std::string(optarg);
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
    printf( "ERROR: asc2txt works only with 2D and 3D data!\n" );
    return EXIT_FAILURE;
  }

  std::ifstream ifs( infile.c_str() );
  if ( outfile.empty() ) {
    string file_name = infile.substr( infile.find_last_of("/\\") + 1 );
    file_name = file_name.substr( 0, file_name.find_last_of(".") );
    outfile = infile.substr( 0, infile.find_last_of("/\\") + 1 );
    outfile = outfile + file_name + ".txt";
  }

  std::ofstream ofs(outfile, std::ofstream::out);
  ofs << std::fixed << std::setprecision( 15 );

  std::string line;
  double lo, hi;

  uint32_t line_counter = 0;
  // read node header
  Box raw;
  uint32_t current_dim = 0;
  uint32_t cnt = 0;
  while ( getline(ifs, line) ) {
    ++line_counter;

    std::istringstream iss(line);

    if ( !(iss >> lo >> hi) ) {
      printf("Error: asc2txt failed to read a line!\n");
      break; // error
    }
    assert( lo <= hi );
    raw.lo[current_dim] = lo;
    raw.hi[current_dim] = hi;
    ++current_dim;

    if ( line_counter % NUM_DIMS == 0 ) {
      assert( current_dim == NUM_DIMS );

      for (uint32_t d = 0; d < NUM_DIMS; ++d)
        ofs << raw.lo[d] << " ";
      for (uint32_t d = 0; d < NUM_DIMS; ++d) {
        ofs << raw.hi[d];
        if (d == NUM_DIMS - 1)
          ofs << endl;
        else
          ofs << " ";
      }

      ++cnt;
      current_dim = 0;
    }

    if ( cnt % 100000 == 0 ) {
      printf( "Objects processed: %uk\r", cnt / 1000 );
      fflush(stdout);
    }
  }

  printf( "Number of lines read: %d\n", line_counter );
  printf( "Number of boxes read: %d\n", cnt );
  printf( "Output file: %s\n", outfile.c_str() );
  printf("DONE!\n");

  return EXIT_SUCCESS;
}

