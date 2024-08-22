/* randsegs.c: randomize a specificly segmented file */
#//
#// Copyright (c) 1994 - 2013 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//-----
#include "RSTFileAccess.h"
/*implies "RSTStdTypes.h" */
#if defined (_WIN32) || defined (_WIN64)
# include "drand48.h"
#endif


Rpint Work(File inFile,
           File outFile,
           int segSize,
           Rpint numbsegs);
Rpint PreView(File inFile,
              char *inName,
              File outFile,
              char *outName,
              int segSize);
Rpint LenOfFile(FILE *f);
void PrintHelp(char *arg);

/************************************************************************/

int main(int argc, char *argv[])

{
  File inFile, outFile;
  int segSize;
  Rpint numbsegs, segcount;
  char s[160];
  
  if (argc == 2 && (strcmp(argv[1],"-help") == 0 || strcmp(argv[1],"-h") == 0)) {
    PrintHelp(argv[0]);
    exit(0);
  }
  else if (argc != 4 || atoi(argv[1]) <= 0) {
    fprintf(stderr,"%s%s%s","Usage: ",argv[0]," -help | -h\n");
    fprintf(stderr,"%s%s%s","       ",argv[0]," segSize inFile outFile\n");
    exit(1);
  }
  
  segSize= atoi(argv[1]);
  
  if (! OpenRdOnlyF(argv[2],&inFile)) {
    exit(2);
  }
  if (! CreateTruncF(argv[3],&outFile)) {
    exit(2);
  }
  
  numbsegs= 0;
  numbsegs= PreView(inFile,
                    argv[2],
                    outFile,
                    argv[3],
                    segSize);
  segcount= Work(inFile,
                 outFile,
                 segSize,
                 numbsegs);
  
  if (! CloseF(inFile)) {
    exit(2);
  }
  if (! CloseF(outFile)) {
    exit(2);
  }
  
  fprintf(stderr,strans("%P segments processed:\n",s),segcount);
  if (segcount == numbsegs) {
    fprintf(stderr,"Done.\n");
  }
  else {
    fprintf(stderr,"Checksum ERROR detected.\n");
    exit(2);
  }
  return 0;
}

/************************************************************************/

Rpint Work(File inFile,
           File outFile,
           int segSize,
           Rpint numbsegs)

{
#define RANDSEED 9811   /* <--- set Param. */
  
  unsigned int i;
  Rpint pos, lastseg, segcount, segNumb;
  char *buf= malloc(segSize);
  int *ind= malloc(numbsegs * sizeof(int)); /* limits numbsegs to 2^31-1 */
                                            /* already 8 GB */

  for (i= 0; i < numbsegs; i++) {
    ind[i]= i;
  }
  
  srand48(RANDSEED);
  
  lastseg= numbsegs - 1;
  segcount= 0;
  while (numbsegs > 0) {
    pos= numbsegs * drand48();
    
    segNumb= ind[pos];
    if (! RdPage(inFile,segSize,segNumb,buf)) {
      exit(2);
    }
    if (! WrBytes(outFile,buf,segSize)) {
      return segcount;
    }
    
    if (pos != lastseg) {
      ind[pos]= ind[lastseg];
    }
    numbsegs--; lastseg--;
    segcount++;
  }
  free(buf); buf= NULL;
  free(ind); ind= NULL;
  return segcount;
  
#undef RANDSEED
}

/************************************************************************/

Rpint PreView(File inFile,
              char *inName,
              File outFile,
              char *outName,
              int segSize)

{
  int segremain;
  Rpint lof, numbsegs;
  char s[160];
  
  lof= LenOfF(inFile);
  if (lof == -1) {
    exit(2);
  }
  numbsegs= lof / segSize;
  segremain= lof % segSize;
  
  fprintf(stderr,strans("Reading %P segments of %d bytes from %s.\n",s),numbsegs,segSize,inName);
  fprintf(stderr,"Writing to %s\n",outName);
  if (segremain != 0) {
    fprintf(stderr,"WARNING: IGNORING remainder (incomplete last segment) of length %d.\n",segremain);
  }
  fprintf(stderr,"WORKING ...\n");
  return numbsegs;
}

/************************************************************************/

void PrintHelp(char *arg) {

  fprintf(stdout,
"SYNOPSYS\n");
  fprintf(stdout,"%s%s%s",
"     Usage: ",arg," -help | -h\n");
  fprintf(stdout,"%s%s%s",
"            ",arg," segSize inFile outFile\n");
  fprintf(stdout,"\n");
  fprintf(stdout,"%s%s\n",
arg," randomly reads segments of length segSize bytes from inFile"
  );
  fprintf(stdout,"%s\n",
"and writes them to outFile consecutively."
  );
  fprintf(stdout,"%s\n",
"At the end outFile contains the segments of inFile in random order."
  );
fprintf(stdout,"%s\n",
"Exit status:"
);
  fprintf(stdout,"%s\n",
"  Returns 0 on success;"
  );
  fprintf(stdout,"%s\n",
"  Returns 1 if a parameter mismatch was detected;"
  );
  fprintf(stdout,"%s\n",
"  Returns 2 if a fatal error occured."
  );
}

/************************************************************************/


