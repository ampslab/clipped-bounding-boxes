/* proj.c: project the rectangles of an nD rectangle file to a 2D plain */
#//
#// Copyright (c) 1994 - 2013 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
/* must be compiled using -D options to guarantee that
 * #define typatom [double, float]
 * will be set properly
*/


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
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
typedef typinterval   *typrect;
typedef typinterval  *typedges;

typedef struct {
  Rpint l, h;
  } typcountinterval;
typedef typcountinterval *typcountedges;

typedef double *typlength;


/*** declarations ***/

Rpint GetLOF(char *path); /* GetLengthOfFile */
void Project(void);
void GetEdges(FILE *file, int numbDim);
void PrintRectFormat(void);
void PrintHelp(char *arg);


/*** global variables ***/

FILE *infile, *outfile;
int NumbOfDim, dim1, dim2;
Rpint begin, end, sizeINrect, sizeOUTrect, NumbRects;
double Area, DitherRate;

int main(int argc, char *argv[])

{
  Rpint lof, firstPos;
  char s[160];
  
  if (argc == 2 && (strcmp(argv[1],"-help") == 0 || strcmp(argv[1],"-h") == 0)) {
    PrintHelp(argv[0]);
    exit(0);
  }
  else if (argc != 6 ) {
    fprintf(stderr,"%s%s%s","Usage: ",argv[0]," -help | -h\n");
    fprintf(stderr,"%s%s%s","       ",argv[0]," NumbOfDim 1stDim 2ndDim inputfile outputfile\n");
    exit(1);
  }
  
  NumbOfDim= atof(argv[1]);
  if (NumbOfDim < 0) {
    fprintf(stderr,"%s %d\n","INVALID negative NumbOfDim:",NumbOfDim);
    exit(1);
  }
  
  infile= fopen(argv[4],"rb");
  if (!infile) {
    perror(argv[4]);
    exit(2);
  }
  if (strcmp(argv[4],argv[5]) == 0) {
    fprintf(stderr,"Cannot write to source file\n");
    exit(2);
  }
  
  lof= GetLOF(argv[4]);
  if (lof == 0) {
    perror(argv[4]);
    exit(2);
  }
  sizeINrect= sizeof(typinterval) * NumbOfDim;
  if (lof % sizeINrect != 0) {
    fprintf(stderr,"WARNING:\n");
    fprintf(stderr,strans("%s%P%s%P%s\n",s),"Length of file: ",lof," % ",sizeINrect," != 0");
  }
  
  dim1= atof(argv[2]);
  if (dim1 < 1 || dim1 > NumbOfDim) {
    fprintf(stderr,"1stDim: %d OUT OF RANGE %d .. %d\n",dim1,1,NumbOfDim);
    exit(1);
  }
  
  dim2= atof(argv[3]);
  if (dim2 < 1 || dim2 > NumbOfDim) {
    fprintf(stderr,"2ndDim: %d OUT OF RANGE %d .. %d\n",dim2,1,NumbOfDim);
    exit(1);
  }
  
  if (dim1 == dim2) {
    fprintf(stderr,"ERROR: 1stDim = 2ndDim = %d\n",dim1);
    exit(1);
  }
  
  outfile= fopen(argv[5],"w+b");
  if (!outfile) {
    perror(argv[5]);
    exit(2);
  }
  
  begin= 0;
  end= lof / sizeINrect;
  
  /***-----------------------------------------------------------***/

  NumbRects= end - begin;
  fprintf(stdout,strans("%s: %P\n",s),"#rectangles",NumbRects);
  fprintf(stdout,"---------- %s: ----------\n",argv[4]);
  GetEdges(infile,NumbOfDim);

  firstPos= fseek(infile,begin*sizeINrect,0);
  if (firstPos == -1) {
    perror(argv[4]);
    exit(2);
  }
  sizeOUTrect= sizeof(typinterval) * 2;
  Project();

  if (fclose(infile)) {
    perror("proj: close infile");
  }

  firstPos= fseek(outfile,begin*sizeOUTrect,0);
  if (firstPos == -1) {
    perror(argv[5]);
    exit(2);
  }
  fprintf(stdout,"---------- %s: ----------\n",argv[5]);
  GetEdges(outfile,2);
  if (fclose(outfile)) {
    perror("proj: close outfile");
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

void GetEdges(FILE *file, int numbDim)

{
  int sizeRect= sizeof(typinterval) * numbDim;
  typrect rect= malloc(sizeRect);
  typedges e= malloc(sizeRect);
  typcountedges ce= malloc(sizeof(typcountinterval) * numbDim);
  typlength length= malloc(sizeof(double) * numbDim);
  typatom maxLen;
  int d;
  Rpint i;
  
  i= begin;
  if (fread(rect,sizeRect,1,file)) {
    for (d= 0; d < numbDim; d++) {
      e[d].l= rect[d].l;
      ce[d].l= 1;
      e[d].h= rect[d].h;
      ce[d].h= 1;
    }
  }
  else {
    perror("proj reading");
    exit(2);
  }
  i++;
  while (i != end) {
    if (fread(rect,sizeRect,1,file)) {
      for (d= 0; d < numbDim; d++) {
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
      perror("proj reading");
      exit(2);
    }
    i++;
  }
  Area= 1.0;
  for (d= 0; d < numbDim; d++) {
    length[d]= e[d].h - e[d].l;
    Area*= length[d];
  }
  fprintf(stdout,"%s: %f\n","Area",Area);
  
  maxLen= length[0];
  for (d= 1; d < numbDim; d++) {
    if (length[d] > maxLen) {
      maxLen= length[d];
    }
  }
  fprintf(stdout,"Minimum Bounds:\n");
  for (d= 0; d < numbDim; d++) {
    fprintf(stdout," %f %f",e[d].l,e[d].h);
  }
  fprintf(stdout,"\n");
  fprintf(stdout,"Minimum Square:\n");
  for (d= 0; d < numbDim; d++) {
    fprintf(stdout," %f %f",e[d].l,e[d].l + maxLen);
  }
  fprintf(stdout,"\n");
}

/***********************************************************************/

void Project(void)

{
  typrect INrect= malloc(sizeINrect);
  typrect OUTrect= malloc(sizeOUTrect);
  Rpint i;
  char s[160];
  
  i= begin;

  while (i != end) {
    if (fread(INrect,sizeINrect,1,infile)) {
      OUTrect[0]= INrect[dim1-1];
      OUTrect[1]= INrect[dim2-1];
    }
    else {
      perror("proj: reading");
      break;
    }

    if (fwrite(OUTrect,sizeOUTrect,1,outfile)) {
      i++;
    }
    else {
      perror("proj: writing");
      break;
    }
  }
  if (NumbRects != i) {
    fprintf(stderr,strans("%s: %P\n",s),"ERROR: #rectangles scanned",i);
    exit(2);
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
"            ",arg," NumbOfDim 1stDim 2ndDim inputfile outputfile\n");
  fprintf(stdout,"\n");
  fprintf(stdout,"%s%s",
arg," works on files containing rectangles in the following format:\n");
  PrintRectFormat();
  fprintf(stdout,
"It consecutively reads rectangles from inputfile.\n");
  fprintf(stdout,
"From each rectangle it extracts the intervals 1stDim and 2ndDim, this way\n");
  fprintf(stdout,
"building a 2D rectangle.\n");
  fprintf(stdout,
"The extracted 2D rectangles are stored in outputfile.\n");
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


