/* 2Dar2dr: convert x1, y1, x2, y2 ASCII formatted rectangles to lhlh double
#//
#// Copyright (c) 1994 - 2013 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
   representation.
   May easily be modified to other conversions.
   Note that lhlh = is equivalent to x1, x2, y1, y2 with ?1 <= ?2 */

#include <stdlib.h>
#include <stdio.h>
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
  Rpint numbrecs, itemsRd, itemsWr;
  typrect rect;
  
  
  if (argc != 3) {
    fprintf(stderr,"Usage: %s %s %s\n",argv[0],"(ascii-)inputfile","(double-)outputfile");
    fprintf(stderr,"%s %s\n",argv[0],"converts rectangles from x1, y1, x2, y2 ASCII to lhlh DOUBLE format.");
    exit(1);
  }
  else {
    infile= fopen(argv[1],"rb");
    outfile= fopen(argv[2],"wb");
    
    numbrecs= 0;
    for (;;) {
      /* NOTE the order of scan below!! */
      itemsRd= fscanf(infile,"%lf %lf %lf %lf",&rect[0].l,&rect[1].l,&rect[0].h,&rect[1].h);
      if (itemsRd == EOF) {
        break;
      }
      
      numbrecs++;
      
      if (itemsRd != 4) {
        ErrorExit("read FAILED",numbrecs);
      }
      if (rect[0].h < rect[0].l || rect[1].h < rect[1].l) {
        fprintf(stderr,strans("NEGATIVE EXTENSION in rectangle %P:\n",s),numbrecs);
        fprintf(stderr,"x1 x2: %f %f\n",rect[0].l,rect[0].h);
        fprintf(stderr,"y1 y2: %f %f\n",rect[1].l,rect[1].h);
        fprintf(stderr,"EXIT\n");
        exit(2);
      }
      
      itemsWr= fwrite(rect,sizeof(typrect),1,outfile);
      if (itemsWr != 1) {
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

