/* ----- drand48.c ----- */
#//
#// Copyright (c) 2013 - 2013 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
/* Re-implementation of Unix's drand48 with 64 bit constants and variables.
   Provides exactly the same interface.
   Behaves exactly as the original. */

#include "drand48.h"

/*** constants ***/
#define m 0x1000000000000ll               /* constant modulo */

/*** types ***/

/*** declarations ***/
static void next(void);
static long long nextState(unsigned short *X);

/*** global variables ***/
unsigned long long a= 0x5DEECE66Dll;      /* constant multiplier */
unsigned short c= 0xB;                    /* constant addend */
/* Multiplier and addend, set to the values above, generate the standard
   behavior of drand48. However, they may be modified by lcong48(). */
unsigned long long x= 0x1234ABCD330Ell;   /* initial state */
/* The initial state is used if neither srand48() nor seed48() or lcong48()
   is called and either drand48(), lrand48 or mrand48() is used. */

unsigned short state[3];
/* State buffer of seed48(). */


/*********************************************************/

static void next(void) {

  x= (a * x + c) % m;
}

/*********************************************************/

static long long nextState(unsigned short *X) {
  
  long long y;
  long long midd, high;
  
  /* store X[] in y corresponding to significance */
  y= X[0];
  midd= X[1]; midd<<= 16;
  high= X[2]; high<<= 32;
  y+= midd + high;
  
  y= (a * y + c) % m;
  
  /* store y in X[] corresponding to significance */
  X[0]= y;
  X[1]= y >> 16;
  X[2]= y >> 32;
  
  return y;
}

/*********************************************************/

double drand48(void) {

  next();
  return (double)x / (double)m;
}

/*********************************************************/

double erand48(unsigned short X[3]) {

  return (double)nextState(X) / (double)m;
}

/*********************************************************/

long lrand48(void) {

  next();
  return (int)(x >> 17);
}

/*********************************************************/

long nrand48(unsigned short X[3]) {

  return (int)(nextState(X) >> 17);
}

/*********************************************************/

long mrand48(void) {

  next();
  return (int)(x >> 16);
}

/*********************************************************/

long jrand48(unsigned short X[3]) {

  return (int)(nextState(X) >> 16);
}

/*********************************************************/

void srand48(long seedval) {

  x= (seedval << 16) + 0x330E;
}

/*********************************************************/

unsigned short *seed48(unsigned short seed16v[3]) {

  long long midd, high;
  
  /* store x in state[] corresponding to significance */
  state[0]= x;
  state[1]= x >> 16;
  state[2]= x >> 32;
  
  /* store seed16v[] in x corresponding to significance */
  x= seed16v[0];
  midd= seed16v[1]; midd<<= 16;
  high= seed16v[2]; high<<= 32;
  x+= midd + high;
  
  return state;
}

/*********************************************************/

void lcong48(unsigned short param[7]) {

  long long midd, high;
  
  /* store param[0-2] in x corresponding to significance */
  x= param[0];
  midd= param[1]; midd<<= 16;
  high= param[2]; high<<= 32;
  x+= midd + high;
  
  /* store param[3-5] in a corresponding to significance */
  a= param[3];
  midd= param[4]; midd<<= 16;
  high= param[5]; high<<= 32;
  a+= midd + high;
  
  c= param[6];
}

/*********************************************************/

