/* CDabs: grep Param. */
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

#define RANDSEED 7817 /* <--- set Param. */

/* define NumbOfDim in Makefile !! *//* <--- set Param. */


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

void CreateAbs(typcerect *rect, int d);
void Produce(typcerect *rect);
void ce2lh(typcerect *ce, typrect rstrect);


/* global variables */

Rpint cols, rectCount, writeEach;
double CellWidth, DitherPos, DitherWidth;
double MPstart[NumbOfDim];
double wi[NumbOfDim];
FILE *f;
char s[160];


int main(int argc, char *argv[])

{
  int d;
  Rpint rcts;
  double WorldWidth, RectWidth, A;
  double start[NumbOfDim];
  typcerect rect;
  
  if (argc != 3 || strcmp(argv[1],"-d") != 0 && strcmp(argv[1],"-q") != 0) {
    printf("Usage: %s %s %s\n",argv[0],"( -d | -q )","filename");
  }
  else {
    /* CDabs produces about rcts rectangles in a (hyper-) square bounded
       -----------------------------------------------------------------
       world.
       ------
       Distribution: absolutely uniform
       Order: line by line, layer by layer, ...
       start[d]: origin-corner of the world
       -----------------------------------------------------------------------
       
       The world, consisting of "cells", is a square with edges of length
       WorldWidth.
       A cell, containing exactly one rectangle, is a square with edges of
       length CellWidth, where
       CellWidth = WorldWidth/cols and
       cols = round( pow(rcts, 1/NumbOfDim) ).
       Undithered rectangles with A = 1 are equivalent to cells.
       Generally the rectangles may stick out of the world by an epsilon
       increasing by position and width dithering.
       
    */
    /* ---------- world specification ---------- */
    for (d= 0; d < NumbOfDim; d++) {
      start[d]= 0.0;
    }
    WorldWidth= 1.0;
    /* ---------- CDabs --- distribution Param. setting ---------- */
    /*
      - typcerect: see "constants" and "types".
      - rcts: number of rectangles (approximated value!).
      - writeEach: a newly produced rectangle is actually written out
                   iff rectCount % writeEach = 0.
                   rectCount, initially 0, is incremented after each production
                   of a new rectangle.
                   This allows to create part of (actually the same)
                   distribution.
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
      rcts= 1000000;              /* <--- set Param. */
      writeEach= 1;               /* <--- set Param. */
      A= 0.7;                     /* <--- set Param. */
      DitherPos= 0.2;             /* <--- set Param. */
      DitherWidth= 0.2;           /* <--- set Param. */
    }
    else if (strcmp(argv[1],"-q") == 0) { /* <--- set Param. */
      rcts= 1000000;              /* <--- set Param. */
      writeEach= 1;               /* <--- set Param. */
      A= 0.0;                     /* <--- set Param. */
      DitherPos= 0.0;             /* <--- set Param. */
      DitherWidth= 0.0;           /* <--- set Param. */
      printf("%s%s%s\n","CAUTION! ",argv[0]," -q: All data near world crossing axes parallels!");
    }
    
    /* ------------------------------------------------------------------- */
    
    RectWidth= pow(A, 1.0 / NumbOfDim);
    for (d= 0; d < NumbOfDim; d++) {
      wi[d]= RectWidth;
    }
    
    cols= pow((double)rcts, 1.0 / NumbOfDim) + 0.5;
    printf(strans("%s: %P\n",s),"cols",cols);
    printf("%s: %.1f\n","rcts",pow((double)cols,(double)NumbOfDim));
    
    CellWidth= WorldWidth / cols;
    
    for (d= 0; d < NumbOfDim; d++) {
      MPstart[d]= start[d] + 0.5 * CellWidth;
    }
    
    /* ------------------------------------------------------------------- */
    
    rectCount= 0;
    f= fopen(argv[2],"wb");
    srand48(RANDSEED);
    for (d= 0; d < NumbOfDim; d++) {
      rect.e[d]= wi[d] * 0.5 * CellWidth;
    }
    CreateAbs(&rect,NumbOfDim - 1);
    fclose(f);
  }
  return 0;
}

/**********************************************************************/

void CreateAbs(typcerect *rect, int d)

{
  Rpint i;
  
  for (i= 0; i < cols; i++) {
    (*rect).c[d]= MPstart[d] + i * CellWidth;
    if (d > 0) {
      CreateAbs(rect,d - 1);
    }
    else {
      Produce(rect);
    }
  }  
}

/**********************************************************************/

void Produce(typcerect *rect)

{
  int d;
  double RealRand;
  typcerect ce;
  typrect rstrect;

  for (d= 0; d < NumbOfDim; d++) {
    RealRand= drand48();
    if (mrand48() >= 0) {
      ce.c[d]= (*rect).c[d] + RealRand * DitherPos * CellWidth;
    }
    else {
      ce.c[d]= (*rect).c[d] - RealRand * DitherPos * CellWidth;
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
