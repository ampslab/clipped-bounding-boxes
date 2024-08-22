/* ap2dr: convert arbitrary dimensional ASCII formatted POINTS to lhlh..
#//
#// Copyright (c) 1994 - 2013 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
   DOUBLE RECTANGLE representation (usable for RSTree) */

#include <stdlib.h>
#include <stdio.h>
#include "RSTStdTypes.h"
/* RSTStdTypes.h mainly provides R*-tree standard constants, types and
   functions, normally not needed here.
   But type Rpint is used to provide a pointer compatible integer type on
   platforms, not serving a 32/64 bit transitional long integer and concerning
   I/O functions (Windows).
   Note that if a source includes RSTStdTypes.h, linking requires libPltfHelp
   or libUTIL. */

/* constants */

#define boolean int
#define FALSE 0
#define TRUE 1


/* types */

typedef double typatom;     /* <--- set Param. */
/* UNUSED:
typedef struct {
  typatom c[NumbOfDim], e[NumbOfDim];
  } datarect, *refdatarect;
*/
typedef struct {
               typatom  l, h;
               } typinterval;


/* declarations */


/* global variables */

char s[160];


int main(int argc, char *argv[])

{
  FILE *infile, *outfile;
  int NumbOfDim, itemsRd, d, dimAtEof;
  Rpint numbrecs;
  boolean endOfFile;
  double coord;
  typinterval interval;
  
  
  if (argc != 4) {
    fprintf(stderr,"Usage: %s %s %s %s\n",argv[0],"NumbOfDim","(ascii-)inputfile","(double-)outputfile");
    fprintf(stderr,"%s %s\n",argv[0],"converts ASCII POINTS to lhlh.. DOUBLE formatted RECTANGLES.");
    exit(1);
  }
  else {
    NumbOfDim= atoi(argv[1]);
    if (NumbOfDim <= 0) {
      fprintf(stderr,"INVALID VALUE for NumbOfDim: %d\n",NumbOfDim);
      fprintf(stderr,"Usage: %s %s %s %s\n",argv[0],"NumbOfDim","(ascii-)inputfile","(double-)outputfile");
      exit(1);
    }
    else {
      fprintf(stderr,"NumbOfDim: %d\n",NumbOfDim);
      fprintf(stderr,"WORKING\n");
    }
    infile= fopen(argv[2],"rb");
    if (infile == NULL) {
      fprintf(stderr,"Cannot read %s.\n",argv[2]);
      fprintf(stderr,"Usage: %s %s %s %s\n",argv[0],"NumbOfDim","(ascii-)inputfile","(double-)outputfile");
      exit(1);
    }
    outfile= fopen(argv[3],"wb");
    
    numbrecs= 0; endOfFile= FALSE;
    for (;;) {
      for (d= 0; d < NumbOfDim; d++) {
        itemsRd= fscanf(infile,"%lf",&coord);
        if (itemsRd == EOF) {
          dimAtEof= d;
          endOfFile= TRUE;
          break;
        }
        else {
          interval.l= coord;
          interval.h= coord;
          fwrite(&interval,sizeof(typinterval),1,outfile);
        }
      }
      if (endOfFile) {
        if (dimAtEof != 0) {
          fprintf(stderr,"FATAL ERROR: incomplete surplus record!\n");
        }
        break;
      }
      else {
        numbrecs++;
      }
    }
    fprintf(stderr,strans("Number of records: %P\n",s),numbrecs);
  }
  return 0;
}


/***********************************************************************/
