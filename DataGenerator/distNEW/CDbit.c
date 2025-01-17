/* CDbit: grep Param. */
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

#define RANDSEED 7823 /* <--- set Param. */

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

double RandBits(void);
void CreateBitDist(double start[NumbOfDim]);
void Produce(typcerect *rect);
double LeastBitValue(void);
void printbits(double value);
void PrintBits(void);
void ce2lh(typcerect *ce, typrect rstrect);


/* global variables */

int BitNumb;
Rpint rcts, rectCount, writeEach;
double WorldWidth, CellWidth, ConstFac, ConstProbab,
       DitherPos, DitherWidth;
double wi[NumbOfDim];
FILE *f;


int main(int argc, char *argv[])

{
  int MAXBitNumb, d;
  double RectWidth, A;
  double start[NumbOfDim];
  char ch;
  boolean creation;
  
  if (argc != 3 || strcmp(argv[1],"-d") != 0 && strcmp(argv[1],"-q") != 0) {
    printf("Usage: %s %s %s\n",argv[0],"( -d | -q )","filename");
  }
  else {
    /* CDbit produces rcts rectangles in a (hyper-) square bounded world.
       ------------------------------------------------------------------
       Distribution: For each (mid-)point the coordinates are found by
                     setting the upper BitNumb bits of their real-mantissa
                     to 1 with probability ConstProbab. The remaining lower
                     bits are set to 0 (and thus left for other purposes).
       Order: randomly uniform if ConstProbab == 0.5,
              concentrated in the origin-corner if ConstProbab < 0.5,
              concentrated opposite to the origin-corner ConstProbab > 0.5.
       start[d]: origin-corner of the world
       -----------------------------------------------------------------------
       
       The world, consisting of "cells", is a square with edges of length
       WorldWidth.
       A cell, containing exactly one rectangle, is a square with edges of
       length CellWidth, where
       CellWidth = WorldWidth / pow(rcts, 1/NumbOfDim).
       Points (A = 0, undithered) are completely located inside the world.
       Generally rectangles may stick out by an epsilon increasing by position
       and width dithering.
       
    */
    /* ---------- world specification ---------- */
    MAXBitNumb= 53; /* see "IEEE" below */
    for (d= 0; d < NumbOfDim; d++) {
      start[d]= 0.0;
    }
    WorldWidth= 1.0;
    /* ---------- CDbit --- distribution Param. setting ---------- */
    /*
      - typcerect: see "constants" and "types".
      - rcts: number of rectangles.
      - writeEach: a newly produced rectangle is actually written out
                   iff rectCount % writeEach = 0.
                   rectCount, initially 0, is incremented after each production
                   of a new rectangle.
                   This allows to create part of (actually the same)
                   distribution.
      - ConstFac: recursively multiplies the start value 1. If ConstFac = 0.5
                  the algorithm produces real bits.
      - BitNumb: maximum number of bits to be set.
      - ConstProbab: probability of setting a bit to 1.
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
      ConstFac= 0.5;              /* <--- set Param. */
      BitNumb= 53;                /* <--- set Param. */
      ConstProbab= 0.3;           /* <--- set Param. */
      A= 0.0;                     /* <--- set Param. */
      DitherPos= 0.0;             /* <--- set Param. */
      DitherWidth= 0.0;           /* <--- set Param. */
    }
    else if (strcmp(argv[1],"-q") == 0) { /* <--- set Param. */
      rcts= 1000000;              /* <--- set Param. */
      writeEach= 1;               /* <--- set Param. */
      ConstFac= 0.5;              /* <--- set Param. */
      BitNumb= 53;                /* <--- set Param. */
      ConstProbab= 0.3;           /* <--- set Param. */
      A= 0.0;                     /* <--- set Param. */
      DitherPos= 1.0;             /* <--- set Param. */
      DitherWidth= 0.0;           /* <--- set Param. */
    }
    
    /* ------------------------------------------------------------------- */
    
    RectWidth= pow(A, 1.0 / NumbOfDim);
    for (d= 0; d < NumbOfDim; d++) {
      wi[d]= RectWidth;
    }
    
    CellWidth= WorldWidth / pow((double)rcts, 1.0 / NumbOfDim);
    
    /* totally available (double IEEE): 53 bits */
    PrintBits();
    
    printf("MAX(BitNumb): %d\n",MAXBitNumb);
    printf("     BitNumb: %d\n",BitNumb);
    if (ConstFac != 0.5) {
      printf("WARNING: \"bits\" are no genuine bits! (ConstFac = %g)\n",ConstFac);
      printf("Available different numbers (1D): ? (see table)\n");
      printf("Create file anyway?, (y/n): ");
      do {
        ch= getchar();
      } while ((ch != 'y') && (ch != 'n'));
      creation= ch == 'y';
    }
    else {
      if (BitNumb > MAXBitNumb) {
        printf("ERROR:\n");
        printf("BitNumb > MAX(BitNumb)\n");
        printf("FILE NOT CREATED!\n");
        creation= FALSE;
      }
      else {
        printf("Available different numbers (1D): %g\n",pow(2.0,(double)BitNumb));
        creation= TRUE;
      }
    }
    
    /* ------------------------------------------------------------------- */
    
    if (creation) {
      rectCount= 0;
      f= fopen(argv[2],"wb");
      srand48(RANDSEED);
      CreateBitDist(start);
      fclose(f);
    }
  }
  return 0;
}

/**********************************************************************/

double RandBits()

{
  int i;
  double RealRand;
  double add= 1.0;
  double value= 0.0;
  
  for (i= 1; i <= BitNumb; i++) {
    add*= ConstFac;
    RealRand= drand48();
    if (RealRand < ConstProbab) {
      value+= add;
    }
  }
  return value;
}

/**********************************************************************/

void CreateBitDist(double start[NumbOfDim])

{
  int d;
  Rpint i;
  typcerect rect;
  
  for (d= 0; d < NumbOfDim; d++) {
    rect.e[d]= wi[d] * 0.5 * CellWidth;
  }
  
  for (i= 1; i <= rcts; i++) {
    for (d= 0; d < NumbOfDim; d++) {
      rect.c[d]= start[d] + WorldWidth * RandBits();
    }
    Produce(&rect);
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

double LeastBitValue()

{
  int i;
  double value= 1.0;
  
  for (i= 1; i <= BitNumb; i++) {
    value*= ConstFac;
  }
  return value;
}

/**********************************************************************/

void printbits(double value)

{  
  int j;
  unsigned long long *u;
  
  u= (unsigned long long *)&value;
  for (j= 1; j <= 64; j++) {
    if (*u & 0x8000000000000000) {
      printf("1");
    }
    else {
      printf("0");
    }
    if (j % 4 == 0) { printf(" ");}
    *u= *u << 1;
  }
  printf("\n");
}

/**********************************************************************/

void PrintBits()

{
  int i;
  double add= 1.0;
  double value= 0.0;
  
  printf("Bits control table (1st line: 0.0, 2nd line: ConstFac = %lf):\n",ConstFac);
  printbits(0.0);
  for (i= 1; i <= BitNumb; i++) {
    add*= ConstFac;
    value+= add;
    printbits(value);
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
