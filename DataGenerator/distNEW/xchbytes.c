/* interchange the bytes in segments of arbitrary length e.g. 123 --> 321 */
#//
#// Copyright (c) 1994 - 2013 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//-----
#include "RSTStdTypes.h"
/* RSTStdTypes.h mainly provides R*-tree standard constants, types and
   functions, normally not needed here.
   But type Rpint is used to provide a pointer compatible integer type on
   platforms, not serving a 32/64 bit transitional long integer and concerning
   I/O functions (Windows).
   Note that if a source includes RSTStdTypes.h, linking requires libPltfHelp
   or libUTIL. */


#define boolean int
#define FALSE 0
#define TRUE 1


Rpint Work(FILE *inFile,
           FILE *outFile,
           int segSize);
Rpint PreView(FILE *inFile,
              char *inName,
              FILE *outFile,
              char *outName,
              int segSize);
Rpint LenOfFile(FILE *f);

/* global variables */

char s[160];


/************************************************************************/

int main(int argc, char *argv[])

{
  FILE *inFile, *outFile;
  int segSize, clres;
  Rpint numbsegs, segcount;
  
  if (argc != 4 || atoi(argv[1]) <= 0) {
    fprintf(stderr,"Usage:  %s %s %s %s\n\n",argv[0],"segSize","inFile","outFile");
    fprintf(stderr,"%s %s\n",argv[0],
    "reads segments of length segSize bytes from inFile, converts"
    );
    fprintf(stderr,"%s\n",
    "them from little to big endian (or vice versa) and writes them to"
    );
    fprintf(stderr,"%s\n",
    "outFile. Example: For segSize = 4, it changes the succession of the bytes"
    );
    fprintf(stderr,"%s\n",
    "in each segment as follows: [a][b][c][d] --> [d][c][b][a]."
    );
    fprintf(stderr,"%s\n",
    "If inFile is set to \"-\" the input is taken from stdin. If outFile is"
    );
    fprintf(stderr,"%s\n",
    "set to \"-\" the output is directed to stdout. Accompanying messages"
    );
    fprintf(stderr,"%s\n",
    "are directed to stderr."
    );
    fprintf(stderr,"%s\n",
    "The program returns 0 on success; in case of a detected parameter"
    );
    fprintf(stderr,"%s\n",
    "mismatch it returns 1 and issues this message; in case of a fatal"
    );
    fprintf(stderr,"%s\n",
    "error it issues an error message and returns 2."
    );
    exit(1);
  }
  
  segSize= atoi(argv[1]);
  
  if (strcmp(argv[2],"-") == 0) {
    inFile= stdin;
  }
  else {
    inFile= fopen(argv[2],"rb");
    if (inFile == NULL) {
      perror(argv[2]);
      exit(2);
    }
  }
  if (strcmp(argv[3],"-") == 0) {
    outFile= stdout;
  }
  else {
    outFile= fopen(argv[3],"w+b");
    if (outFile == NULL) {
      perror(argv[3]);
      exit(2);
    }
  }
  
  numbsegs= 0;
  if (inFile == stdin) {
    segcount= Work(inFile,
                   outFile,
                   segSize);
  }
  else {
    numbsegs= PreView(inFile,
                      argv[2],
                      outFile,
                      argv[3],
                      segSize);
    segcount= Work(inFile,
                   outFile,
                   segSize);
  }
  
  clres= fclose(inFile);
  if (clres != 0) {
    perror(argv[2]);
    exit(2);
  }
  clres= fclose(outFile);
  if (clres != 0) {
    perror(argv[3]);
    exit(2);
  }
  
  fprintf(stderr,strans("%P segments processed:\n",s),segcount);
  if (inFile == stdin) {
    /* no control for incomplete items continuosly reading, thus: */
    fprintf(stderr,"Done.\n");
  }
  else {
    if (segcount == numbsegs) {
      fprintf(stderr,"Done.\n");
    }
    else {
      fprintf(stderr,"Checksum ERROR detected.\n");
      exit(2);
    }
  }
  return 0;
}

/************************************************************************/

Rpint Work(FILE *inFile,
           FILE *outFile,
           int segSize)

{
  int j;
  Rpint segcount, nitems;
  char *inbuf= malloc(segSize);
  char *outbuf= malloc(segSize);
  int seglast= segSize - 1;
  
  segcount= 0;
  for (;;) {
    nitems= fread(inbuf,segSize,1,inFile);
    if (feof(inFile) != 0) {
      return segcount;
    }
    if (nitems != 1) {
      fprintf(stderr,strans("nitems = %P: ",s),nitems);
      perror("ERROR reading");
      return segcount;
    }
    
    for (j= 0; j < segSize; j++) {
      outbuf[j]= inbuf[seglast-j];
    }
    
    nitems= fwrite(outbuf,segSize,1,outFile);
    if (nitems != 1) {
      fprintf(stderr,strans("nitems = %P: ",s),nitems);
      perror("ERROR writing");
      return segcount;
    }
    segcount++;
  }
}

/************************************************************************/

Rpint PreView(FILE *inFile,
              char *inName,
              FILE *outFile,
              char *outName,
              int segSize)

{
  int segremain;
  Rpint lof, numbsegs;
  
  lof= LenOfFile(inFile);
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

Rpint LenOfFile(FILE *f)

{
  int res;
  Rpint pos, lof;
  
  
  pos= ftell(f);
  if (pos == -1) {
    perror("ERROR checking file length");
    exit(2);
  }
  
  res= fseek(f,0,SEEK_END);
  if (res != 0) {
    perror("ERROR checking file length");
    exit(2);
  }
  lof= ftell(f);
  if (lof == -1) {
    perror("ERROR checking file length");
    exit(2);
  }
  
  res= fseek(f,pos,SEEK_SET);
  if (res != 0) {
    perror("ERROR checking file length");
    exit(2);
  }
  
  return lof;
}

/************************************************************************/
