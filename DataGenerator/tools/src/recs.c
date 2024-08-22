/* recs.c: get the number of specificly sized records of a file */
#//
#// Copyright (c) 1994 - 2013 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
//-----
#include "RSTStdTypes.h"
/* RSTStdTypes.h mainly provides R*-tree standard constants, types and
   functions, normally not needed here.
   But type Rpint is used to provide a pointer compatible integer type on
   platforms, not serving a 32/64 bit transitional long integer and concerning
   I/O functions (Windows).
   Note that if a source includes RSTStdTypes.h, linking requires libPltfHelp
   or libUTIL. */


/*** declarations ***/

Rpint GetLOF(char *path); /* GetLengthOfFile */
void PrintHelp(char *arg);


int main(int argc, char *argv[])

{
  int recordSize, remain;
  Rpint i, lof, numbRecs;
  char s[160];
  
  if (argc == 2 && (strcmp(argv[1],"-help") == 0 || strcmp(argv[1],"-h") == 0)) {
    PrintHelp(argv[0]);
    exit(0);
  }
  else if (argc < 3) {
    fprintf(stderr,"%s%s%s","Usage: ",argv[0]," -help | -h\n");
    fprintf(stderr,"%s%s%s","       ",argv[0]," recordSize fileName ...\n");
    exit(1);
  }
  
  recordSize= atoi(argv[1]);
  if (recordSize <= 0) {
    fprintf(stderr,"%s %d\n","INVALID recordSize:",recordSize);
    exit(1);
  }
  fprintf(stdout,"\n");
  fprintf(stdout,"%20s %25s %20s  %10s\n","length","name","#records","remainder");
  fprintf(stdout,"%20s %25s %20s  %10s\n",
                      "--------------------","-------------------------","--------------------","----------");
  for (i= 2; i < argc; i++) {
    lof= GetLOF(argv[i]);
    if (lof == -1) {
      perror(argv[i]);
    }
    else {
      numbRecs= lof / recordSize;
      remain= lof % recordSize;
      fprintf(stdout,strans("%20P %25s %20P  %10d\n",s),lof,argv[i],numbRecs,remain);
    }
  }
  return 0;
}

/***********************************************************************/

Rpint GetLOF(char *path) /* GetLengthOfFile */

{
  struct stat status;
  int ferr;
  
  ferr= stat(path,&status);
  if (ferr == -1) {
    return 0;
  }
  else {
    return status.st_size;
  }
}

/***********************************************************************/

void PrintHelp(char *arg) {

  fprintf(stdout,
"SYNOPSYS\n");
  fprintf(stdout,"%s%s%s",
"     Usage: ",arg," -help | -h\n");
  fprintf(stdout,"%s%s%s",
"            ",arg," recordSize fileName ...\n");
  fprintf(stdout,"\n");
  fprintf(stdout,"%s%s",
arg," determines the number of records of size recordSize bytes of the\n");
  fprintf(stdout,
"named files and prints the result to standard output.\n");
  fprintf(stdout,
"Output format:\n");
  fprintf(stdout,
"length in bytes, file name, number of records, remainder in bytes\n");
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

/***********************************************************************/


