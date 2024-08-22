/* asc2doub.c: convert ASCII numbers to binary doubles */
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
           FILE *outFile);
void PreView(FILE *inFile,
             char *inName,
             FILE *outFile,
             char *outName);
void ExitPrintUsage(char *arg);


/************************************************************************/

int main(int argc, char *argv[])

{
  FILE *inFile, *outFile;
  int clres;
  Rpint doubCount;
  char s[160];

  if (argc != 3) {
    ExitPrintUsage(argv[0]);
  }
  
  if (strcmp(argv[1],"-") == 0) {
    inFile= stdin;
  }
  else {
    inFile= fopen(argv[1],"rb");
    if (inFile == NULL) {
      perror(argv[1]);
      exit(2);
    }
  }
  if (strcmp(argv[2],"-") == 0) {
    outFile= stdout;
  }
  else {
    outFile= fopen(argv[2],"w+b");
    if (outFile == NULL) {
      perror(argv[2]);
      exit(2);
    }
  }
  
  if (inFile == stdin) {
    doubCount= Work(inFile,
                    outFile);
  }
  else {
    PreView(inFile,
            argv[1],
            outFile,
            argv[2]);
    doubCount= Work(inFile,
                    outFile);
  }
  
  clres= fclose(inFile);
  if (clres != 0) {
    perror(argv[1]);
    exit(2);
  }
  clres= fclose(outFile);
  if (clres != 0) {
    perror(argv[2]);
    exit(2);
  }
  
  fprintf(stderr,strans("%P doubles processed:\n",s),doubCount);
  if (inFile == stdin) {
    fprintf(stderr,"Done.\n");
  }
  else {
    fprintf(stderr,"Done.\n");
  }
  return 0;
}

/************************************************************************/

Rpint Work(FILE *inFile,
           FILE *outFile)

{
  int nitems;
  Rpint doubCount, nbytes;
  double Double;
  char s[160];

  doubCount= 0;
  for (;;) {
    nitems= fscanf(inFile,"%lf",&Double);
    if (feof(inFile) != 0) {
      return doubCount;
    }
    if (nitems != 1) {
      fprintf(stderr,"nitems = %d: ",nitems);
      perror("ERROR reading");
      return doubCount;
    }
    
    nbytes= fwrite(&Double,sizeof(double),1,outFile);
    if (nbytes < 0) {
      fprintf(stderr,strans("nbytes = %P: ",s),nbytes);
      perror("ERROR writing");
      return doubCount;
    }
    doubCount++;
  }
}

/************************************************************************/

void PreView(FILE *inFile,
             char *inName,
             FILE *outFile,
             char *outName)

{
  fprintf(stderr,"Reading from %s\n",inName);
  fprintf(stderr,"Writing to %s\n",outName);
  fprintf(stderr,"WORKING ...\n");
}

/************************************************************************/

void ExitPrintUsage(char *arg) {

  fprintf(stderr,"Usage:  %s %s %s\n\n",arg,"inFile","outFile");
  fprintf(stderr,"%s %s\n",arg,
"reads white-space separated, ASCII represented numbers from inFile,"
  );
  fprintf(stderr,"%s\n",
"converts them to binary doubles, and writes them gap free to outFile."
  );
  fprintf(stderr,"%s\n",
"If inFile is set to \"-\", the input is taken from stdin."
  );
  fprintf(stderr,"%s\n",
"If outFile is set to \"-\", the output is directed to stdout."
  );
  fprintf(stderr,"%s\n",
"Accompanying messages are directed to stderr."
  );
fprintf(stderr,"%s\n",
"Exit status:"
);
  fprintf(stderr,"%s\n",
"  Returns 0 on success;"
  );
  fprintf(stderr,"%s\n",
"  Returns 1 if a parameter mismatch was detected;"
  );
  fprintf(stderr,"%s\n",
"  Returns 2 if a fatal error occured."
  );
  exit(1);
}

/************************************************************************/

