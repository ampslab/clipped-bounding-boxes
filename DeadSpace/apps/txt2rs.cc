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

#include "SpatialObjects.h"

using namespace std;

void printUsage(char *prog) {
  string exec( prog );
  exec = exec.substr( exec.find_last_of('/') + 1 );
  printf("\n%s - Converts .txt files used in our SpatialIndex lib to the\n"
      "format understood by DataGeneretor/tools/asc2dbl (also text).\n"
      "Warning: only 3D supported!\n\n",
      exec.c_str() );
  printf( "USAGE: ./%s -i input-file [-o output-file -v -h]\n", exec.c_str() );
  printf( " -h: this help message\n");
  printf( " -v: verbose \n");
  printf( " -i: input file \n");
  printf( " -o: output file. Default: input-file + '.rs' \n");
  printf( "Example: ");
  printf( "./%s -i 100K-axon-mbr-644000.bin.txt \n\n", exec.c_str());
}

int main(int argc, char **argv) {
  std::string infile, outfile;
  bool verbose = false;

  int c;
  opterr = 0;
  while ((c = getopt(argc, argv, "i:o:vh")) != -1) {
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

  if (NUM_DIMS != 3) {
    printf("ERROR: txt2RS supports only 3D!\n");
    return EXIT_FAILURE;
  }

  std::ifstream ifs( infile.c_str() );
  if ( outfile.empty() )
    outfile = infile + ".rs";
//  std::ofstream ofs( outfile.c_str() );

  FILE* pFile = fopen(outfile.c_str(), "w" );

  std::string line;
  double x_lo, y_lo, z_lo, x_hi, y_hi, z_hi;

  uint32_t line_counter = 0;
  // read node header
  while ( getline(ifs, line) ) {
    ++line_counter;

    std::istringstream iss(line);
    if ( !(iss >> x_lo >> y_lo >> z_lo >> x_hi >> y_hi >> z_hi) ) {
      printf("Error: txt2RS failed to read a line!\n");
      break; // error
    }
    assert( x_lo <= x_hi && y_lo <= y_hi && z_lo <= z_hi );

//    long written = 0;
//    written += fwrite( &x_lo, sizeof(double), 1, pFile );
//    written += fwrite( &x_hi, sizeof(double), 1, pFile );
//    written += fwrite( &y_lo, sizeof(double), 1, pFile );
//    written += fwrite( &y_hi, sizeof(double), 1, pFile );
//    written += fwrite( &z_lo, sizeof(double), 1, pFile );
//    written += fwrite( &z_hi, sizeof(double), 1, pFile );

    long nbytes = 0;
    nbytes += fprintf( pFile, "% .15e % .15e\n", x_lo, x_hi );
    nbytes += fprintf( pFile, "% .15e % .15e\n", y_lo, y_hi );
    nbytes += fprintf( pFile, "% .15e % .15e\n", z_lo, z_hi );

    if ( nbytes != 138 ) {
      printf("ERROR: wrong number of bytes written to file!\n");
    }

   }

//  ifs.close();

  fclose(pFile);
//  ofs.close();

  if ( verbose ) {
    printf( "Number of lines read: %d\n", line_counter );
  }

  return EXIT_SUCCESS;
}

