/* CDpar: grep Param. */
#//
#// Copyright (c) 1994 - 2013 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
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
#include "drand48.h"
/* for drand48 on platforms not providing it (Windows); linking: see above. */


/* constants */

#define boolean int
#define FALSE 0
#define TRUE 1

#define RANDSEED 7877 /* <--- set Param. */

/* define NumbOfDim in Makefile !! *//* <--- set Param. */
#define MaxDim (NumbOfDim -1)


/* types */

typedef double typatom;       /* <--- set Param. */

typedef struct {
  typatom c[NumbOfDim], e[NumbOfDim];
  } typcerect;

typedef struct {
               typatom  l, h;
               } typinterval;
typedef typinterval  typrect[NumbOfDim];


/* declarations */

void CreatePart(double lo[NumbOfDim],
                double hi[NumbOfDim],
                double CellWidth[NumbOfDim],
                int step);
void Realize(double lo[NumbOfDim],
             double hi[NumbOfDim]);
void Produce(double CellWidth[NumbOfDim],
             typcerect *rect);
void ce2lh(typcerect *ce, typrect rstrect);


/* global variables */

int maxstep;
Rpint rectCount, writeEach;
boolean randAxes;
double tr, DitherPos, DitherWidth;
double wi[NumbOfDim];
FILE *f;


int main(int argc, char *argv[])

{
  int firstskip, i, dummy, d;
  double WorldWidth, RectWidth, A;
  double start[NumbOfDim];
  double end[NumbOfDim];
  double CellWidth[NumbOfDim];
  
  if (argc != 3 || strcmp(argv[1],"-d") != 0 && strcmp(argv[1],"-q") != 0) {
    printf("Usage: %s %s %s\n",argv[0],"( -d | -q )","filename");
  }
  else {
    /* CDpar produces rectangles by recursively splitting the world into two
       ---------------------------------------------------------------------
       parts, modulo alternating or randomly choosing the split axis before
       --------------------------------------------------------------------
       each split.
       -----------
       Distribution: partitions
       Order: Z-order
       start[d]: origin-corner of the world
       -----------------------------------------------------------------------
       
       The world, consisting of "cells", is a square with edges of length
       WorldWidth.
       A cell, containing exactly one rectangle, is a rectangular partition,
       obtained by recursively splitting the world into two parts at an
       arbitrary position, thereby modulo alternating or randomly choosing
       respectively the split axis.
       As the partitions, cells here may have extremly different area and
       shape.
       Unit-rectangles (A = 1, undithered) are completely located inside the
       world. Generally rectangles may stick out by an epsilon increasing by
       position and width dithering.
       
    */
    /* ---------- world specification ---------- */
    for (d= 0; d < NumbOfDim; d++) {
      start[d]= 0.0;
    }
    WorldWidth= 1.0;
    /* ---------- CDpar --- distribution Param. setting ---------- */
    /*
      - typcerect: see "constants" and "types".
      - maxstep: number of times, the world is splitted recursively.
                 REMIND that (tr = 0.0, randAxes = FALSE provided) the
                 creation of quadratic partitions requires
                 maxstep % NumbOfDim = 0.
      - writeEach: a newly produced rectangle is actually written out
                   iff rectCount % writeEach = 0.
                   rectCount, initially 0, is incremented after each production
                   of a new rectangle.
                   This allows to create part of (actually the same)
                   distribution.
      - tr: tiling range: Examples:
                                    1.0: fully randomly tiling [0.0 .. 1.0]
                                    0.6: [0.2 .. 0.8] 
                                    0.0: [0.5 .. 0.5]
      - randAxes: if set to TRUE, the split axis is chosen randomly otherwise
                  it alternates.
      - firstskip: skip firstskip random numbers to get another distribution.
      - A: rectangle area relative to the area of a cell.
      - DitherPos: rectangle position dithering relative to CellWidth.
                   For each dimension d, the center-position c[d] of the
                   rectangle is determined as follows:
                   c[d]= c[d] +|- X * DitherPos * CellWidth;
                   X uniformly distributed in [0, 1).
      - DitherWidth: rectangle width dithering relative to the rectangle width.
                     For each dimension d, the width wi[d] of the rectangle
                     is determined as follows:
                     wi[d]= wi[d] * (1 +|- X * DitherWidth);
                     X uniformly distributed in [0, 1);
                     0 <= DitherWidth <= 1  REQUIRED!
    */
    if (strcmp(argv[1],"-d") == 0) { /* <--- set Param. */
      maxstep= 20;                /* <--- set Param. */
      writeEach= 1;               /* <--- set Param. */
      tr= 1.0;                    /* <--- set Param. */
      randAxes= FALSE;            /* <--- set Param. */
      firstskip= 0;               /* <--- set Param. */
      A= 0.5;                     /* <--- set Param. */
      DitherPos= 1.0;             /* <--- set Param. */
      DitherWidth= 0.0;           /* <--- set Param. */
    }
    else if (strcmp(argv[1],"-q") == 0) { /* <--- set Param. */
      maxstep= 20;                /* <--- set Param. */
      writeEach= 1;               /* <--- set Param. */
      tr= 1.0;                    /* <--- set Param. */
      randAxes= FALSE;            /* <--- set Param. */
      firstskip= 0;               /* <--- set Param. */
      A= 0.0;                     /* <--- set Param. */
      DitherPos= 0.0;             /* <--- set Param. */
      DitherWidth= 0.0;           /* <--- set Param. */
    }
    
    /* ------------------------------------------------------------------- */
    
    RectWidth= pow(A, 1.0 / (double)NumbOfDim);
    for (d= 0; d < NumbOfDim; d++) {
      wi[d]= RectWidth;
    }
    
    for (d= 0; d < NumbOfDim; d++) {
      CellWidth[d]= WorldWidth;
      end[d]= start[d] + WorldWidth;
    }
    
    /* ------------------------------------------------------------------- */
    
    rectCount= 0;
    f= fopen(argv[2],"wb");
    srand48(RANDSEED);
    printf("CDpar: General warning:\n");
    printf("Due to floating point calculation the distribution may have\n");
    printf("gaps or overlapping regions (negative gaps) which produce\n");
    printf("warnings only for values greater 1.0e-15 (about pow(2,-50)).\n");
    for (i= 1; i <= firstskip; i++) {
      printf("%s%d%s\n","skipping ",i,". random number!");
      dummy= drand48();
    }
    
    CreatePart(start,end,CellWidth,0);
    fclose(f);
  }
  return 0;
}

/**********************************************************************/

void CreatePart(double lo[NumbOfDim],
                double hi[NumbOfDim],
                double CellWidth[NumbOfDim],
                int step)

{
  int axis, d;
  double ratio1, ratio2, RealRand, GapOvlp;
  double lonew[NumbOfDim];
  double hinew[NumbOfDim];
  double CellWidthNew[NumbOfDim];
  
  if (step == maxstep) {
    Realize(lo,hi);
  }
  else {
  
    step+= 1;
    RealRand= drand48();
    /* ratio1= 0.5; ratio2= 0.5; test correctness */
    ratio1= 0.5 * (1.0 - tr) + RealRand * tr;
    ratio2= 1 - ratio1;
    
    for (d= 0; d < NumbOfDim; d++) {
      lonew[d]= lo[d];
      hinew[d]= hi[d];
      CellWidthNew[d]= CellWidth[d];
    }
    
    RealRand= drand48();
    if (randAxes) {
      axis= (int)(RealRand * (double)NumbOfDim);     /* random alternation */
    }
    else {
      axis= step % NumbOfDim;     /* modulo alternation */
    }
    
    CellWidthNew[axis]= CellWidth[axis] * ratio1;
    hinew[axis]= lo[axis] + CellWidthNew[axis];
    CreatePart(lo,hinew,CellWidthNew,step);
    
    lonew[axis]= hinew[axis];
    CellWidthNew[axis]= CellWidth[axis] * ratio2;
    GapOvlp= fabs(hi[axis] - (lonew[axis] + CellWidthNew[axis]));
    if (GapOvlp > 1.0e-15) {
      printf("WARNING: gap/ovlp: %11.3e\n",GapOvlp);
    }
    CreatePart(lonew,hi,CellWidthNew,step);
  }
}

/**********************************************************************/

void Realize(double lo[NumbOfDim],
             double hi[NumbOfDim])

{
  int d;
  typcerect rect;
  double CellWidth[NumbOfDim];
  
  for (d= 0; d < NumbOfDim; d++) {
    if (lo[d] > hi[d]) {
      printf("%s %.20f %.20f\n","NEGATIVE EXTENSION:",lo[d],hi[d]);
    }
    rect.c[d]= (lo[d] + hi[d]) * 0.5;
    CellWidth[d]= hi[d] - lo[d];
    rect.e[d]= CellWidth[d] * 0.5 * wi[d];
  }
  Produce(CellWidth,&rect);
}

/* CellWidth is newly computed here to assure it to be (hi - lo) */

/**********************************************************************/

void Produce(double CellWidth[NumbOfDim],typcerect *rect)

{
  int d;
  double RealRand;
  typcerect ce;
  typrect rstrect;

  for (d= 0; d < NumbOfDim; d++) {
    RealRand= drand48();
    if (mrand48() >= 0) {
      ce.c[d]= (*rect).c[d] + RealRand * DitherPos * CellWidth[d];
    }
    else {
      ce.c[d]= (*rect).c[d] - RealRand * DitherPos * CellWidth[d];
    }
    
    RealRand= drand48();
    if (mrand48() >= 0) {
      ce.e[d]= (*rect).e[d] + RealRand * DitherWidth * (*rect).e[d];
    }
    else {
      ce.e[d]= (*rect).e[d] - RealRand * DitherWidth * (*rect).e[d];
    }
  }
  rectCount++;
  if (writeEach != 0) {
    if (rectCount % writeEach == 0) {
      ce2lh(&ce,rstrect);
      fwrite(rstrect,sizeof(typrect),1,f);
    }
  }
}

/**********************************************************************/

void ce2lh(typcerect *ce, typrect rstrect)

{
  int d;
  
  for (d= 0; d < NumbOfDim; d++) {
    rstrect[d].l= (*ce).c[d] - (*ce).e[d];
    rstrect[d].h= (*ce).c[d] + (*ce).e[d];
  }
}

/***********************************************************************/
