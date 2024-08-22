/* marea.c: modify the area of the rectangles of a rectangle file */
#//
#// Copyright (c) 1994 - 2013 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
/* must be compiled using -D options to guarantee that
 * #define typatom [double, float]
 * will be set properly
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
//-----
#include "RSTStdTypes.h"
/* RSTStdTypes.h mainly provides R*-tree standard constants, types and
   functions, normally not needed here.
   But type Rpint is used to provide a pointer compatible integer type on
   platforms, not serving a 32/64 bit transitional long integer and concerning
   I/O functions (Windows).
   Note that if a source includes RSTStdTypes.h, linking requires libPltfHelp
   or libUTIL. */


/*** constants ***/

/* see comments at top */


/*** types ***/

typedef struct {
               typatom  l, h;
               } typinterval;
typedef typinterval *typrect;


/*** declarations ***/

Rpint GetLOF(char *path); /* GetLengthOfFile */
void VaryExtension(void);
void PrintRectFormat(void);
void lh2ce(typrect rect, typatom *c, typatom *e);
void ce2lh(typatom *c, typatom *e, typrect rect);
void PrintHelp(char *arg);

/*** global variables ***/

FILE *inFile, *outFile;
int NumbOfDim;
Rpint begin, end, sizeRect, numbRects;
double factor;

int main(int argc, char *argv[]) {

  Rpint lof;
  char s[160];
  
  if (argc == 2 && (strcmp(argv[1],"-help") == 0 || strcmp(argv[1],"-h") == 0)) {
    PrintHelp(argv[0]);
    exit(0);
  }
  else if (argc != 5 ) {
    fprintf(stderr,"%s%s%s","Usage: ",argv[0]," -help | -h\n");
    fprintf(stderr,"%s%s%s","       ",argv[0]," NumbOfDim inputfile factor outputfile\n");
    exit(1);
  }
  
  NumbOfDim= atof(argv[1]);
  if (NumbOfDim < 0) {
    fprintf(stderr,"%s %d\n","INVALID negative NumbOfDim:",NumbOfDim);
    exit(1);
  }
  
  inFile= fopen(argv[2],"rb");
  if (!inFile) {
    perror(argv[2]);
    exit(2);
  }
  lof= GetLOF(argv[2]);
  if (lof == 0) {
    perror(argv[2]);
    exit(2);
  }
  sizeRect= sizeof(typinterval) * NumbOfDim;
  if (lof % sizeRect != 0) {
    fprintf(stderr,"WARNING:\n");
    fprintf(stderr,strans("%s%P%s%P%s\n",s),"Length of file: ",lof," % ",sizeRect," != 0");
  }
  
  factor= atof(argv[3]);
  if (factor < 0) {
    fprintf(stderr,"%s %f\n","INVALID negative factor:",factor);
    exit(1);
  }
  
  outFile= fopen(argv[4],"wb");
  if (!outFile) {
    perror(argv[4]);
    exit(2);
  }
  
  begin= 0;
  end= lof / sizeRect;
  
  /***-----------------------------------------------------------***/

  numbRects= end - begin;
  VaryExtension();
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

void VaryExtension(void)

{
  typrect rect= malloc(sizeRect);
  typatom *c= malloc(sizeof(typatom) * NumbOfDim);
  typatom *e= malloc(sizeof(typatom) * NumbOfDim);
  double extFac;
  int d;
  Rpint i;
  char s[160];
  
  extFac= pow(factor, 1.0/(double)NumbOfDim);
  
  fprintf(stderr,strans("%s: %P\n",s),"#rectangles",numbRects);
  fprintf(stderr,"%s: %f\n","area modification factor given",factor);
  fprintf(stderr,"%s: %f\n","extension modification factor calculated",extFac);
  i= begin;
  while (i != end) {
    if (fread(rect,sizeRect,1,inFile)) {
      lh2ce(rect,c,e);
      for (d= 0; d < NumbOfDim; d++) {
        e[d]*= extFac;
      }
      ce2lh(c,e,rect);
    }
    else {
      perror("marea: reading");
      break;
    }
    if (fwrite(rect,sizeRect,1,outFile)) {
      i++;
    }
    else {
      perror("marea: writing");
      break;
    }
  }
  if (fclose(inFile)) {
    perror("marea: close inFile");
  }
  if (fclose(outFile)) {
    perror("marea: close outFile");
  }
  if (numbRects != i) {
    fprintf(stderr,strans("%s: %P\n",s),"ERROR: #rectangles scanned",i);
    exit(3);
  }
}

/***********************************************************************/

void PrintRectFormat(void)

{
  if (sizeof(typatom) == sizeof(double)) {
    fprintf(stdout,"struct {double l, h;} rectangle[NumbOfDim].\n");
  }
  else if (sizeof(typatom) == sizeof(float)) {
    fprintf(stdout,"struct {float l, h;} rectangle[NumbOfDim].\n");
  }
  else {
    fprintf(stdout,"struct {UNKNOWN l, h;} rectangle[NumbOfDim].\n");
  }
}

/***********************************************************************/

void ce2lh(typatom *c, typatom *e, typrect rect)

{
  int d;
  
  for (d= 0; d < NumbOfDim; d++) {
    rect[d].l= c[d] - e[d];
    rect[d].h= c[d] + e[d];
  }
}

/***********************************************************************/

void lh2ce(typrect rect, typatom *c, typatom *e)

{
  int d;
  
  for (d= 0; d < NumbOfDim; d++) {
    /* guarantee rectangle edges with 0 extension to be stable: */
    if (rect[d].l == rect[d].h) {
      c[d]= rect[d].l;
      e[d]= 0.0;
    }
    else {
      c[d]= (rect[d].l + rect[d].h) * 0.5;
      e[d]= (rect[d].h - rect[d].l) * 0.5;
    }
  }
}

/***********************************************************************/

void PrintHelp(char *arg) {

    fprintf(stdout,
"SYNOPSYS\n");
    fprintf(stdout,"%s%s%s",
"     Usage: ",arg," -help | -h\n");
    fprintf(stdout,"%s%s%s",
"            ",arg," NumbOfDim inputfile factor outputfile\n");
    fprintf(stdout,"\n");
    fprintf(stdout,"%s%s",
arg," works on files containing rectangles in the following format:\n");
    PrintRectFormat();
    fprintf(stdout,
"It consecutively reads rectangles from inputfile and modifies their\n");
    fprintf(stdout,
"extensions on each axis such that NewArea = OldArea * factor.\n");
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

