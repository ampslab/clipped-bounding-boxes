/* check rectangles for consistency */
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


typedef struct {
               double  l, h;
               } typinterval;


Rpint Work(FILE *inFile,
           int numbOfDim,
           int *interremain);
Rpint PreView(FILE *inFile,
              char *inName,
              int numbOfDim);
Rpint LenOfFile(FILE *f);

/* global variables */

char s[160];


/************************************************************************/

int main(int argc, char *argv[])

{
  FILE *inFile;
  int interremain, numbOfDim, clres;
  Rpint numbinter, intercount;
  
  if (argc != 3 || atoi(argv[1]) <= 0) {
    fprintf(stderr,"Usage:  %s %s %s\n\n",argv[0],"numbOfDim","inFile");
    fprintf(stderr,"%s %s\n",argv[0],
    "reads rectangles of the following (C language) type:"
    );
    fprintf(stderr,"\n");
    fprintf(stderr,"%s\n",
    "struct {"
    );
    fprintf(stderr,"%s\n",
    "       double l, h;"
    );
    fprintf(stderr,"%s\n",
    "       } rectangle[numbOfDim];"
    );
    fprintf(stderr,"\n");
    fprintf(stderr,"%s\n",
    "from inFile and performs the following consistency checks:"
    );
    fprintf(stderr,"%s\n",
    "- for each interval l <= h should hold"
    );
    fprintf(stderr,"%s\n",
    "- the last rectangle should be complete."
    );
    fprintf(stderr,"%s\n",
    "If inFile is set to \"-\" the input is taken from stdin. Accompanying"
    );
    fprintf(stderr,"%s\n",
    "messages are directed to stderr."
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
  
  numbOfDim= atoi(argv[1]);
  
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
  
  numbinter= 0;
  if (inFile == stdin) {
    intercount= Work(inFile,
                     numbOfDim,
                     &interremain);
  }
  else {
    numbinter= PreView(inFile,
                       argv[2],
                       numbOfDim);
    intercount= Work(inFile,
                     numbOfDim,
                     &interremain);
  }
  
  clres= fclose(inFile);
  if (clres != 0) {
    perror(argv[2]);
    exit(2);
  }
  
  fprintf(stderr,strans("%P rectangles, ",s),intercount / numbOfDim);
  fprintf(stderr,strans("%P intervals resp. processed:\n",s),intercount);
  if (inFile == stdin) {
    if (interremain == 0) {
      fprintf(stderr,"Done.\n");
    }
    else {
      fprintf(stderr,"WARNING: incomplete last rectangle (%d intervals).\n",interremain);
      fprintf(stderr,"Done.\n");
      /* no control for incomplete items (intervals here) continuosly reading */
    }
  }
  else {
    if (intercount == numbinter) {
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
           int numbOfDim,
           int *interremain)

{
  Rpint intercount, nitems, i;
  typinterval interval;
  
  intercount= 0;
  for (;;) {
    for (i= 0; i < numbOfDim; i++) {
      nitems= fread(&interval,sizeof(interval),1,inFile);
      if (feof(inFile) != 0) {
        *interremain= i;
        return intercount;
      }
      if (nitems != 1) {
        fprintf(stderr,strans("nitems = %P: ",s),nitems);
        perror("ERROR reading");
        return intercount;
      }
      intercount++;
      if (interval.l > interval.h) {
         fprintf(stderr,strans("NEGATIVE EXTENSION in the %P. interval.\n",s),intercount);
      }
    }
  }
}

/************************************************************************/

Rpint PreView(FILE *inFile,
              char *inName,
              int numbOfDim)

{
  int interremain, rectsize, rectremain;
  Rpint lof, numbinter, numbrects;
  
  lof= LenOfFile(inFile);
  numbinter= lof / sizeof(typinterval);
  interremain= lof % sizeof(typinterval);
  rectsize= numbOfDim * sizeof(typinterval);
  numbrects= lof / rectsize;
  rectremain= lof % rectsize;
  
  fprintf(stderr,strans("Reading %P rectangles, %P intervals resp. from %s.\n",s),numbrects,numbinter,inName);
  if (rectremain != 0) {
    fprintf(stderr,"WARNING: remainder (incomplete last rectangle) of length %d.\n",rectremain);
  }
  if (interremain != 0) {
    fprintf(stderr,"WARNING: IGNORING remainder (incomplete last interval) of length %d.\n",interremain);
  }
  fprintf(stderr,"WORKING ...\n");
  return numbinter;
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
