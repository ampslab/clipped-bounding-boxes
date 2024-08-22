/* mbb.c: print parameters of the minimum bounding box of a rectangle file */
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

typedef typatom  *typpoint;
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
void GetEdges(void);
void PrintRectFormat(void);
void PrintHelp(char *arg);


/*** global variables ***/

FILE *f;
int NumbOfDim;
Rpint begin, end, sizeRect, numbRects;


int main(int argc, char *argv[])

{
  Rpint lof, firstPos, maxEnd;
  char s[160];
  
  if (argc == 2 && (strcmp(argv[1],"-help") == 0 || strcmp(argv[1],"-h") == 0)) {
    PrintHelp(argv[0]);
    exit(0);
  }
  else if (argc < 3 || argc == 4 || argc > 5) {
    fprintf(stderr,"%s%s%s","Usage: ",argv[0]," -help -h\n");
    fprintf(stderr,"%s%s%s","       ",argv[0]," NumbOfDim filename [ first last ]\n");
    fprintf(stderr,"                                  1     n\n");
    exit(1);
  }
  
  NumbOfDim= atof(argv[1]);
  if (NumbOfDim < 0) {
    fprintf(stderr,"%s %d\n","INVALID negative NumbOfDim:",NumbOfDim);
    exit(1);
  }
  
  f= fopen(argv[2],"rb");
  if (!f) {
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
  maxEnd= lof / sizeRect;
  
  if (argc > 3) {
    begin= atoi(argv[3]);
    end= atoi(argv[4]);
    if (begin <= 0 || end < begin) {
      fprintf(stderr,"%s%s%s","       ",argv[0]," filename [ first last ]\n");
      fprintf(stderr,"                         1     n\n");
      exit(1);
    }
    begin--;
    if (end > maxEnd) {
      fprintf(stdout,"WARNING:\n");
      fprintf(stdout,strans("%s %s %P %s\n",s),argv[2],"contains only",maxEnd,"rectangles");
      end= maxEnd;
      fprintf(stdout,strans("%s %P %s %P\n",s),"Scanning",begin+1,"..",end);
    }
  }
  else {
    begin= 0;
    end= maxEnd;
  }
  
  /***-----------------------------------------------------------***/
  
  firstPos= fseek(f,begin*sizeRect,0);
  if (firstPos == -1) {
    perror(argv[2]);
    exit(2);
  }
  numbRects= end - begin;
  GetEdges();
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

void GetEdges(void)

{
  typrect rect= malloc(sizeRect);
  typedges c= malloc(sizeRect);
  typedges e= malloc(sizeRect);
  typcountedges cc= malloc(sizeof(typcountinterval) * NumbOfDim);
  typcountedges ce= malloc(sizeof(typcountinterval) * NumbOfDim);
  typlength length= malloc(sizeof(double) * NumbOfDim);
  typatom center, maxLen, maxCentDist;
  double area;
  int d;
  Rpint i;
  char s[160];
  
  i= begin;
  if (fread(rect,sizeRect,1,f)) {
    for (d= 0; d < NumbOfDim; d++) {
      c[d].l= (rect[d].l + rect[d].h) * 0.5;
      cc[d].l= 1;
      c[d].h= c[d].l;
      cc[d].h= 1;
      e[d].l= rect[d].l;
      ce[d].l= 1;
      e[d].h= rect[d].h;
      ce[d].h= 1;
    }
  }
  else {
    perror("mbb reading");
    exit(3);
  }
  i++;
  while (i != end) {
    if (fread(rect,sizeRect,1,f)) {
      for (d= 0; d < NumbOfDim; d++) {
        center= (rect[d].l + rect[d].h) * 0.5;
        if (center < c[d].l) {
          c[d].l= center;
          cc[d].l= 1;
        }
        else if (center == c[d].l) {
          cc[d].l++;
        }
        
        if (center > c[d].h) {
          c[d].h= center;
          cc[d].h= 1;
        }
        else if (center == c[d].h) {
          cc[d].h++;
        }
        
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
      perror("mbb reading");
      exit(3);
    }
    i++;
  }
  fprintf(stdout,strans("%s: %P\n",s),"#rectangles",numbRects);
  area= 1.0;
  fprintf(stdout,"----- Area covered by the Center Points: ------\n");
  for (d= 0; d < NumbOfDim; d++) {
    fprintf(stdout,strans("   %s%d%s: %f     %s%P\n",s),"c[",d,"].l",c[d].l,"#",cc[d].l);
    fprintf(stdout,strans("   %s%d%s: %f     %s%P\n",s),"c[",d,"].h",c[d].h,"#",cc[d].h);
    length[d]= c[d].h - c[d].l;
    area*= length[d];
  }
  for (d= 0; d < NumbOfDim; d++) {
    fprintf(stdout,"%s%d%s: %f\n","Length[",d,"]",length[d]);
  }
  fprintf(stdout,"     %s: %f\n","Area",area);

  area= 1.0;
  fprintf(stdout,"----------------- Total Area: -----------------\n");
  for (d= 0; d < NumbOfDim; d++) {
    fprintf(stdout,strans("   %s%d%s: %f     %s%P\n",s),"e[",d,"].l",e[d].l,"#",ce[d].l);
    fprintf(stdout,strans("   %s%d%s: %f     %s%P\n",s),"e[",d,"].h",e[d].h,"#",ce[d].h);
    length[d]= e[d].h - e[d].l;
    area*= length[d];
  }
  for (d= 0; d < NumbOfDim; d++) {
    fprintf(stdout,"%s%d%s: %f\n","Length[",d,"]",length[d]);
  }
  fprintf(stdout,"     %s: %f\n","Area",area);
  
  maxLen= length[0];
  for (d= 1; d < NumbOfDim; d++) {
    if (length[d] > maxLen) {
      maxLen= length[d];
    }
  }
  fprintf(stdout,"--------------- Minimum Bounds: ---------------\n");
  for (d= 0; d < NumbOfDim; d++) {
    fprintf(stdout,"  %f %f",e[d].l,e[d].h);
  }
  fprintf(stdout,"\n");
  fprintf(stdout,"-------- Minimum Square (origin bound): --------\n");
  for (d= 0; d < NumbOfDim; d++) {
    fprintf(stdout,"  %f %f",e[d].l,e[d].l + maxLen);
  }
  fprintf(stdout,"\n");
  fprintf(stdout,"-------- Minimum Square (center bound): --------\n");
  for (d= 0; d < NumbOfDim; d++) {
    center= (e[d].l + e[d].h) * 0.5;
    maxCentDist= maxLen * 0.5;
    fprintf(stdout,"  %f %f",center - maxCentDist,center + maxCentDist);
  }
  fprintf(stdout,"\n");
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
"            ",arg," NumbOfDim filename [ first last ]\n");
  fprintf(stdout,
"                                       1     n\n");
  fprintf(stdout,"\n");
  fprintf(stdout,"%s%s",
arg," works on files containing rectangles in the following format:\n");
  PrintRectFormat();
  fprintf(stdout,
"It computes information about the minimum bounding box of the rectangles\n");
  fprintf(stdout,
"in the given range and prints it to standard output.\n");
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

