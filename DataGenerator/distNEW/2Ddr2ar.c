/* 2Ddr2ar: convert lhlh double formatted rectangles to x1, y1, x2, y2 ASCII
#//
#// Copyright (c) 1994 - 2013 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
   representation.
   May easily be modified to other conversions.
   Note that lhlh = is equivalent to x1, x2, y1, y2 with ?1 <= ?2 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
//-----
#include "RSTStdTypes.h"
/* RSTStdTypes.h mainly provides R*-tree standard constants, types and
   functions, normally not needed here.
   But type Rpint is used to provide a pointer compatible integer type on
   platforms, not serving a 32/64 bit transitional long integer and concerning
   I/O functions (Windows).
   Note that if a source includes RSTStdTypes.h, linking requires libPltfHelp
   or libUTIL. */

/* constants */

#define NumbOfDim 2


/* types */

typedef double typatom;
/* UNUSED:
typedef struct {
  typatom c[NumbOfDim], e[NumbOfDim];
  } datarect, *refdatarect;
*/
typedef struct {
               typatom  l, h;
               } typinterval;
typedef typinterval  typrect[NumbOfDim];


/* declarations */

void ErrorExit(char *str, Rpint numbrecs);


/* global variables */

char s[160];


int main(int argc, char *argv[])

{
  FILE *infile, *outfile;
  Rpint numbrecs, itemsRd, resultWr;
  typrect rect;
  
  
  if (argc != 3) {
    fprintf(stderr,"Usage: %s %s %s\n",argv[0],"(double-)inputfile","(ascii-)outputfile");
    fprintf(stderr,"%s %s\n",argv[0],"converts rectangles from lhlh DOUBLE to x1, y1, x2, y2 ASCII format.");
    fprintf(stderr,"NOTE: The number of records has to be checked for correctness since\n");
    fprintf(stderr,"      an incomplete last record does not cause an error message, but\n");
    fprintf(stderr,"      is silently cut off.\n");
    exit(1);
  }
  else {
    infile= fopen(argv[1],"rb");
    outfile= fopen(argv[2],"wb");
    
    numbrecs= 0;
    for (;;) {
      itemsRd= fread(rect,sizeof(typrect),1,infile);
      /* No EOF available for incomplete fread */
      if (itemsRd != 1) {
   /* if (feof(infile) != 0) does the same job,
          no item or an incomplete item may have been read,
          incomplete items do not set errno != 0. */
        break;
      }
      
      numbrecs++;
      
      if (rect[0].h < rect[0].l || rect[1].h < rect[1].l) {
        fprintf(stderr,strans("NEGATIVE EXTENSION in rectangle %P:\n",s),numbrecs);
        fprintf(stderr,"rect[0].l rect[0].h: %f %f\n",rect[0].l,rect[0].h);
        fprintf(stderr,"rect[1].l rect[1].h: %f %f\n",rect[1].l,rect[1].h);
        fprintf(stderr,"EXIT\n");
        exit(2);
      }
      
      resultWr= fprintf(outfile,"%f %f %f %f\n",rect[0].l,rect[1].l,rect[0].h,rect[1].h);
      if (resultWr == 0) { /* even the previous record may be incomplete!! */
        ErrorExit("write FAILED",numbrecs);
      }
    }
    fprintf(stderr,strans("Number of records: %P\n",s),numbrecs);
  }
  return 0;
}


/***********************************************************************/

void ErrorExit(char *str, Rpint numbrecs)

{
  fprintf(stderr,strans("%s at record %P\n",s),str,numbrecs);
  fprintf(stderr,strans("%P records written\n",s),numbrecs - 1);
  exit(2);
}

/***********************************************************************/

