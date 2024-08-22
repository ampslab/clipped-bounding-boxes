/* cpeach: write each k-th record of specified size from inputFile to outputFile */
#//
#// Copyright (c) 1994 - 2013 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
//-----
#include "RSTFileAccess.h"
/*implies "RSTStdTypes.h" */


/*** constants ***/

/*** types ***/

/*** declarations ***/

Rpint GetLOF(char *path); /* GetLengthOfFile */
void PrintHelp(char *arg);

/*** global variables ***/


int main(int argc, char *argv[])

{
  Rpint lof, recordSize, numbIn, numbOut, i, k,
       nbytes, pnb, clres;
  File inFile, outFile;
  void *refBuf;
  char s[160];
  
  if (argc == 2 && (strcmp(argv[1],"-help") == 0 || strcmp(argv[1],"-h") == 0)) {
    PrintHelp(argv[0]);
    exit(0);
  }
  else if (argc != 5 ) {
    fprintf(stderr,"%s%s%s","Usage: ",argv[0]," -help | -h\n");
    fprintf(stderr,"%s%s%s","       ",argv[0]," k-th recordSize inputFile outputFile\n");
    exit(1);
  }
    
  k= atoi(argv[1]);
  fprintf(stderr,strans("k = %P\n",s),k);
  
  recordSize= atoi(argv[2]);
  fprintf(stderr,strans("Recordsize: %P\n",s),recordSize);
  refBuf= malloc(recordSize);
  if (refBuf == NULL) {
    perror("Record buffer allocation");
    exit(2);
  }
  
  if (! OpenRdOnlyF(argv[3],&inFile)) {
    exit(2);
  }
  lof= GetLOF(argv[3]);
  if (lof == 0) {
    perror(argv[3]);
    exit(2);
  }
  if (lof % recordSize != 0) {
    fprintf(stderr,"WARNING:\n");
    fprintf(stderr,strans("%s%P%s%P%s\n",s),"Length of file: ",lof," % ",recordSize," != 0");
  }
  
  if (! CreateTruncF(argv[4],&outFile)) {
    exit(2);
  }
  
  numbIn= lof / recordSize;
  fprintf(stderr,strans("Number of input records: %P\n",s),numbIn);
  fprintf(stderr,strans("Resulting number of output records: %P\n",s),numbIn / k);
  fprintf(stderr,"WORKING\n");
  numbOut= 0;
  for (i= k-1; i < numbIn; i+= k) {
    pnb= lseek(inFile,i * recordSize,SEEK_SET);
    if (pnb != -1) {
      pnb= read(inFile,refBuf,recordSize);
    }
    if (pnb != recordSize) {
      perror(argv[3]);
      exit(2);
    }
    nbytes= write(outFile,refBuf,recordSize);
    if (nbytes != recordSize) {
      perror(argv[4]);
      exit(2);
    }
    numbOut++;
  }
  fprintf(stderr,strans("Actual number of output records: %P\n",s),numbOut);
  clres= close(inFile);
  if (clres == -1) {
    perror(argv[3]);
    exit(2);
  }
  clres= close(outFile);
  if (clres == -1) {
    perror(argv[4]);
    exit(2);
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
"            ",arg," k-th recordSize inputFile outputFile\n");
  fprintf(stdout,"\n");
  fprintf(stdout,"%s%s",
arg," copies each k-th record of recordSize bytes from inputFile to\n");
  fprintf(stdout,
"outputFile.\n");
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

