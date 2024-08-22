/* cmpsegs.c: compare segments of specified size */
#//
#// Copyright (c) 1994 - 2013 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//-----
#include "RSTFileAccess.h"
/*implies "RSTStdTypes.h" */

#define MAX_NAME_SIZE 160

Rpint Work(File inFile1,
           File inFile2,
           int segSize,
           Rpint numbSegs,
           boolean verbose);
Rpint PreView(File inFile1,
              Rpint lof1,
              File inFile2,
              Rpint lof2,
              int segSize,
              boolean verbose);
boolean SegsEql(char *seg1, char *seg2, int len);
void PrintHelp(char *arg);

/* global variables: */

  char *buf1;
  char *buf2;

/************************************************************************/

int main(int argc, char *argv[])

{
  File inFile1, inFile2;
  char inName1[MAX_NAME_SIZE + 1];
  char inName2[MAX_NAME_SIZE + 1];
  int segSize;
  Rpint lof1, lof2, numbSegs, nFaults, remain1, remain2;
  char s[160];
  
  if (argc == 2 && (strcmp(argv[1],"-help") == 0 || strcmp(argv[1],"-h") == 0)) {
    PrintHelp(argv[0]);
    exit(0);
  }
  else if (argc < 4 || argc > 5) {
    fprintf(stderr,"%s%s%s","Usage: ",argv[0]," -help | -h\n");
    fprintf(stderr,"%s%s%s","       ",argv[0]," [ -v ] segSize file1 file2\n");
    exit(1);
  }
  if (argc == 4 && atoi(argv[1]) > 0) {
    segSize= atoi(argv[1]);
    strlcpy(inName1,argv[2],MAX_NAME_SIZE);
    strlcpy(inName2,argv[3],MAX_NAME_SIZE);
  }
  else if (argc == 5 && strncmp(argv[1],"-v",2) == 0 && atoi(argv[2]) > 0) {
    segSize= atoi(argv[2]);
    strlcpy(inName1,argv[3],MAX_NAME_SIZE);
    strlcpy(inName2,argv[4],MAX_NAME_SIZE);
  }
  else {
    fprintf(stderr,"%s%s%s","Usage: ",argv[0]," -help | -h\n");
    fprintf(stderr,"%s%s%s","       ",argv[0]," [ -v ] segSize file1 file2\n");
    exit(1);
  }
  
  buf1= malloc(segSize);
  buf2= malloc(segSize);
  
  if (! OpenRdOnlyF(inName1,&inFile1)) {
    exit(2);
  }
  if (! OpenRdOnlyF(inName2,&inFile2)) {
    exit(2);
  }
  lof1= LenOfF(inFile1);
  if (lof1 == -1) {
    exit(2);
  }
  lof2= LenOfF(inFile2);
  if (lof2 == -1) {
    exit(2);
  }
  
  numbSegs= 0;
  numbSegs= PreView(inFile1,
                    lof1,
                    inFile2,
                    lof2,
                    segSize,
                    argc == 5);
  nFaults= Work(inFile1,
                inFile2,
                segSize,
                numbSegs,
                argc == 5);
  
  if (! CloseF(inFile1)) {
    exit(2);
  }
  if (! CloseF(inFile2)) {
    exit(2);
  }
  
  remain1= lof1 - numbSegs * segSize;
  remain2= lof2 - numbSegs * segSize;
  fprintf(stdout,strans("file(1): %s\n",s),inName1);
  fprintf(stdout,strans("file(2): %s\n",s),inName2);
  fprintf(stdout,strans("Segment size: %10P\n",s),segSize);
  fprintf(stdout,strans("size(1): %15P  remainder: %15P\n",s),lof1,remain1);
  fprintf(stdout,strans("size(2): %15P  remainder: %15P\n",s),lof2,remain2);
  fprintf(stdout,strans("------------------------------------------------------\n",s));
  fprintf(stdout,strans("Number of differing segments:  %10P\n",s),nFaults);
  fprintf(stdout,strans("                              ------------\n",s));
  fprintf(stdout,strans("Number of segments  compared:  %10P\n",s),numbSegs);
  fprintf(stdout,strans("Differing segments: %.2f%%\n",s),nFaults * 100.0 / numbSegs);
  
  free(buf1);
  free(buf2);
  return 0;
}

/************************************************************************/

Rpint Work(File inFile1,
           File inFile2,
           int segSize,
           Rpint numbSegs,
           boolean verbose)

{
  Rpint i, faultCnt;
  char numbBuf[80];
  char s[80];
  boolean linefeed= TRUE;
  
  faultCnt= 0;
  for (i= 0; i < numbSegs; i++) {
    if (! RdPage(inFile1,segSize,i,buf1)) {
      exit(2);
    }
    if (! RdPage(inFile2,segSize,i,buf2)) {
      exit(2);
    }
    if (! SegsEql(buf1,buf2,segSize)) {
      faultCnt++;
      if (verbose) {
        sprintf(numbBuf,strans("%P,%#Q",s),i,i * segSize);
        fprintf(stdout," %20s",numbBuf);
        linefeed= FALSE;
        if (faultCnt % 5 == 0) {
          fprintf(stdout,"\n");
          linefeed= TRUE;
        }
      }
    }
  }
  if (!linefeed) {
    fprintf(stdout,"\n");
  }
  return faultCnt;
}

/************************************************************************/

Rpint PreView(File inFile1,
              Rpint lof1,
              File inFile2,
              Rpint lof2,
              int segSize,
              boolean verbose)

{
  #define f1 "file(1)"
  #define f2 "file(2)"
  
  int segRemain1, segRemain2;
  Rpint numbSegs1, numbSegs2, numbSegs;
  char *gtrFstr;
  char s[160];
  
  numbSegs1= lof1 / segSize;
  segRemain1= lof1 % segSize;
  if (segRemain1 != 0) {
    fprintf(stderr,"WARNING: file(1): IGNORING remainder (incomplete last segment) of length %d.\n",segRemain1);
  }
  numbSegs2= lof2 / segSize;
  segRemain2= lof2 % segSize;
  if (segRemain2 != 0) {
    fprintf(stderr,"WARNING: file(2): IGNORING remainder (incomplete last segment) of length %d.\n",segRemain2);
  }
  
  if (numbSegs1 != numbSegs2) {
    fprintf(stderr,"WARNING: Files of different length.\n");
    if (numbSegs1 < numbSegs2) {
      numbSegs= numbSegs1;
      gtrFstr= f2;
    }
    else {
      numbSegs= numbSegs2;
      gtrFstr= f1;
    }
    fprintf(stderr,strans("WARNING: %s: Comparing only the first %P segments.\n",s),gtrFstr,numbSegs);
  }
  else {
    numbSegs= numbSegs1;
  }
  fprintf(stderr,strans("Comparing %P segments of %d bytes.\n",s),numbSegs,segSize);
  if (verbose) {
    printf("VERBOSE: Numbering(!) and hex addresses start with 0\n");
  }
  fprintf(stderr,"WORKING ...\n");
  return numbSegs;
  
  #undef f1
  #undef f2
}

/************************************************************************/

boolean SegsEql(char *seg1, char *seg2, int len) {

  int i= 0;
  while (i < len) {
    if (*seg1 != *seg2) {
      return FALSE;
    }
    i++; seg1++; seg2++;
  }
  return TRUE;
}

/************************************************************************/

void PrintHelp(char *arg) {

  fprintf(stdout,
"SYNOPSYS\n");
  fprintf(stdout,"%s%s%s",
"     Usage: ",arg," -help | -h\n");
  fprintf(stdout,"%s%s%s",
"            ",arg," [ -v ] segSize file1 file2\n");
  fprintf(stdout,"\n");
  fprintf(stdout,"%s%s\n",
arg," compares files segment by segment (instead of byte by byte, as the"
  );
  fprintf(stdout,"%s\n",
"Unix command \"cmp\" does)."
  );
  fprintf(stdout,"%s\n",
"An arbitrary segment size can be set in the command line."
  );
  fprintf(stdout,"%s\n",
"If files of different size are compared, comparison is performed for the"
  );
  fprintf(stdout,"%s\n",
"number of segments in the shorter file. If the size of a file is not"
  );
  fprintf(stdout,"%s\n",
"divisible by the segment size, the remainder is ignored. In both cases"
  );
  fprintf(stdout,"%s\n",
"warnings are issued on stderr."
  );
  fprintf(stdout,"%s\n",
"Options:"
  );
  fprintf(stdout,"%s\n",
"  -v  \"verbose\""
  );
  fprintf(stdout,"%s\n",
"  In standard mode a summary is printed to stdout, containing the absolute"
  );
  fprintf(stdout,"%s\n",
"  and the percental number of differing segments."
  );
  fprintf(stdout,"%s\n",
"  In verbose mode the summary is prepended by a list, denoting all differing"
  );
  fprintf(stdout,"%s\n",
"  segments, where a list element consists of the running number of the"
  );
  fprintf(stdout,"%s\n",
"  segment and its hexadecimal address, separated by a comma. The output is"
  );
  fprintf(stdout,"%s\n",
"  formatted in rows of 5 elements."
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


