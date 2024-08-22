/* dither.c: dither the rectangles of a rectangle file */
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
#if defined (_WIN32) || defined (_WIN64)
# include "drand48.h"
#endif


/*** constants ***/

/* see comments at top */
#define RANDSEED 9803

/*** types ***/

typedef struct {
               typatom  l, h;
               } typinterval;
typedef typinterval   *typrect;
typedef typinterval  *typedges;

typedef struct {
               Rpint l, h;
               } typcountinterval;
typedef typcountinterval *typcountedges;

typedef double *typlength;

/*** declarations ***/

Rpint GetLOF(char *path); /* GetLengthOfFile */
void DitherPosition(void);
void GetEdges(FILE *file);
void PrintRectFormat(void);
void PrintHelp(char *arg);

/*** global variables ***/

FILE *infile, *outfile;
int NumbOfDim;
Rpint begin, end, sizeRect, numbRects;
double Area, DitherRate;


int main(int argc, char *argv[])

{
  Rpint lof, firstPos;
  char s[160];
  
  if (argc == 2 && (strcmp(argv[1],"-help") == 0 || strcmp(argv[1],"-h") == 0)) {
    PrintHelp(argv[0]);
    exit(0);
  }
  else if (argc != 5 ) {
    fprintf(stderr,"%s%s%s","Usage: ",argv[0]," -help | -h\n");
    fprintf(stderr,"%s%s%s","       ",argv[0]," NumbOfDim inputfile DitherRate outputfile\n");
    exit(1);
  }
  
  NumbOfDim= atof(argv[1]);
  if (NumbOfDim < 0) {
    fprintf(stderr,"%s %d\n","INVALID negative NumbOfDim:",NumbOfDim);
    exit(1);
  }
  
  infile= fopen(argv[2],"rb");
  if (!infile) {
    perror(argv[2]);
    exit(2);
  }
  if (strcmp(argv[4],argv[2]) == 0) {
    fprintf(stderr,"Cannot write to source file\n");
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
  
  DitherRate= atof(argv[3]);
  if (DitherRate < 0) {
    fprintf(stderr,"%s %f\n","INVALID negative DitherRate:",DitherRate);
    exit(1);
  }
  
  outfile= fopen(argv[4],"w+b");
  if (!outfile) {
    perror(argv[4]);
    exit(2);
  }
  
  begin= 0;
  end= lof / sizeRect;
  
  /***-----------------------------------------------------------***/

  numbRects= end - begin;
  fprintf(stderr,strans("%s: %P\n",s),"#rectangles",numbRects);
  fprintf(stderr,"---------- %s: ----------\n",argv[2]);
  GetEdges(infile);
  firstPos= fseek(infile,begin*sizeRect,0);
  if (firstPos == -1) {
    perror(argv[2]);
    exit(2);
  }
  DitherPosition();
  firstPos= fseek(outfile,begin*sizeRect,0);
  if (firstPos == -1) {
    perror(argv[4]);
    exit(2);
  }
  if (fclose(infile)) {
    perror("dither: close infile");
  }
  fprintf(stderr,"---------- %s: ----------\n",argv[4]);
  GetEdges(outfile);
  if (fclose(outfile)) {
    perror("dither: close outfile");
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

void GetEdges(FILE *file)

{
  typrect rect= malloc(sizeRect);
  typedges e= malloc(sizeRect);
  typcountedges ce= malloc(sizeof(typcountinterval) * NumbOfDim);
  typlength length= malloc(sizeof(double) * NumbOfDim);
  typatom maxLen;
  int d;
  Rpint i;
  
  i= begin;
  if (fread(rect,sizeRect,1,file)) {
    for (d= 0; d < NumbOfDim; d++) {
      e[d].l= rect[d].l;
      ce[d].l= 1;
      e[d].h= rect[d].h;
      ce[d].h= 1;
    }
  }
  else {
    perror("dither reading");
    exit(3);
  }
  i++;
  while (i != end) {
    if (fread(rect,sizeRect,1,file)) {
      for (d= 0; d < NumbOfDim; d++) {
        if (rect[d].l < e[d].l) {
          e[d].l= rect[d].l;
          ce[d].l= 1;
        }
        else if (rect[d].l == e[d].l) {
          ce[d].l++;
        }
        
        if (rect[d].h > e[d].h) {
          e[d].h= rect[d].h;
          ce[d].h= 1;
        }
        else if (rect[d].h == e[d].h) {
          ce[d].h++;
        }
      }
    }
    else {
      perror("dither reading");
      exit(3);
    }
    i++;
  }
  Area= 1.0;
  for (d= 0; d < NumbOfDim; d++) {
    length[d]= e[d].h - e[d].l;
    Area*= length[d];
  }
  fprintf(stderr,"%s: %f\n","Area",Area);
  
  maxLen= length[0];
  for (d= 1; d < NumbOfDim; d++) {
    if (length[d] > maxLen) {
      maxLen= length[d];
    }
  }
  fprintf(stderr,"Minimum Bounds:\n");
  for (d= 0; d < NumbOfDim; d++) {
    fprintf(stderr," %f %f",e[d].l,e[d].h);
  }
  fprintf(stderr,"\n");
  fprintf(stderr,"Minimum Square:\n");
  for (d= 0; d < NumbOfDim; d++) {
    fprintf(stderr," %f %f",e[d].l,e[d].l + maxLen);
  }
  fprintf(stderr,"\n");
}

/***********************************************************************/

void DitherPosition(void)

{
  typrect rect= malloc(sizeRect);
  int d;
  Rpint i;
  double RealRand, CellArea, CellWidth, posModif;
  CellArea= Area / (double)numbRects;
  CellWidth= pow(CellArea, 1.0/(double)NumbOfDim);
  char s[160];
  
  // srand48(RANDSEED); /* inadvertantly not set originally! */
  
  i= begin;
  while (i != end) {
    if (fread(rect,sizeRect,1,infile)) {
      for (d= 0; d < NumbOfDim; d++) {
        RealRand= drand48();
        posModif= RealRand * DitherRate * CellWidth;
        if (mrand48() >= 0) {
          rect[d].l+= posModif;
          rect[d].h+= posModif;
        }
        else {
          rect[d].l-= posModif;
          rect[d].h-= posModif;
        }
      }
    }
    else {
      perror("dither: reading");
      break;
    }
    if (fwrite(rect,sizeRect,1,outfile)) {
      i++;
    }
    else {
      perror("dither: writing");
      break;
    }
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

void PrintHelp(char *arg) {

  fprintf(stdout,
"SYNOPSYS\n");
  fprintf(stdout,"%s%s%s",
"     Usage: ",arg," -help | -h\n");
  fprintf(stdout,"%s%s%s",
"            ",arg," NumbOfDim inputfile DitherRate outputfile\n");
  fprintf(stdout,"\n");
  fprintf(stdout,"%s%s",
arg," works on files containing rectangles of the following format:\n");
  PrintRectFormat();
  fprintf(stdout,
"It consecutively reads rectangles from inputfile, dithers their positions,\n");
  fprintf(stdout,
"and writes them this way modified to outputfile.\n");
  fprintf(stdout,
"For the position (center) c of each rectangle it computes:\n");
  fprintf(stdout,
"c[d]= c[d] +|- X * DitherRate * CellWidth,\n");
  fprintf(stdout,
"where:  X is a random variable, uniformly distributed in [0, 1),\n");
  fprintf(stdout,
"        CellWidth = pow(CellArea, 1/NumbOfDim),\n");
  fprintf(stdout,
"        CellArea = Area / #rectangles,\n");
  fprintf(stdout,
"and     Area is the minimum bounding rectangle containing all rectangles\n");
  fprintf(stdout,
"        of inputfile.\n");
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

