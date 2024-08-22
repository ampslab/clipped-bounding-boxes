/* rstplain: AllQuery(RStarTree) --> plain file */
#//
#// Copyright (c) 1994 - 2013 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
/* rstplain.c: grep Param. */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "RSTOtherFuncs.h"


/* constants */


/* declarations */

void PrintRectFormat(void);
void Store2File(t_RT rst,
                Rint numbDim,
                const typinterval *rstrect,
                typinfo *info,
                Rint infoSize,
                void *dummy,
                boolean *modify,
                boolean *finish);


/* global variables */

Rint rectSize= NumbOfDim * sizeof(typinterval);
Rint pointSize= NumbOfDim * sizeof(typcoord);
Rpint rectcount;
FILE *outfile;
char s[160];


int main(int argc, char *argv[])

{
  boolean success;
  t_RT rst;
  QueryManageFunc MANAGE= Store2File;
  
  if (argc == 2 && strcmp(argv[1],"-help") == 0) {
    printf(
"SYNOPSYS\n");
    printf("%s%s%s",
"     Usage: ",argv[0]," -help\n");
    printf("%s%s%s",
"            ",argv[0]," RStarTree outputfile\n");
    printf("\n");
    printf("%s%s",
argv[0]," fetches all rectangles from RStarTree and stores them in\n");
    printf(
"outputfile in the following format:\n");
    PrintRectFormat();
    exit(1);
  }
  else if (argc != 3 ) {
    printf("%s%s%s",
"Usage: ",argv[0]," -help\n");
    printf("%s%s%s",
"       ",argv[0]," RStarTree outputfile\n");
    exit(1);
  }
  else {
    InitRSTreeIdent(&rst);
    printf("%s%s%s","OpenRST(",argv[1],"):\n");
    if (OpenRST(&rst,argv[1])) {
      printf("Done\n");
    }
    else {
      printf("FAILED\n");
      exit(2);
    }
    PrintRectFormat();
    outfile= fopen(argv[2],"wb");
    rectcount= 0;
    printf("Querying RStarTree ...\n");
    success= AllQuery(rst,NULL,MANAGE);
    if (! success) {
      printf("FAILURE in AllQuery\n");
      exit(2);
    }
    printf(strans("%s%P\n",s),"Number of rectangles: ",rectcount);
    printf("Done.\n");
  }
  return 0;
}

/***********************************************************************/

void PrintRectFormat()

{
  if (sizeof(typatomkey) == sizeof(double)) {
    printf("rectangle: struct{ DOUBLE l, h } ");
  }
  else if (sizeof(typatomkey) == sizeof(float)) {
    printf("rectangle: struct{ FLOAT l, h } ");
  }
  else {
    printf("rectangle: struct{ UNKNOWN(!?) l, h } ");
  }
  printf("%s%d%s\n","[",NumbOfDim,"].");
}

/***********************************************************************/

void Store2File(t_RT rst,
                Rint numbDim,
                const typinterval *rstrect,
                typinfo *info,
                Rint infoSize,
                void *dummy,
                boolean *modify,
                boolean *finish) {

  rectcount++;
  fwrite(rstrect,rectSize,1,outfile);
}

/***********************************************************************/
