/* ----- RSTStdTypes.c ----- */
#//
#// Copyright (c) 1994 - 2013 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//


#include <stdio.h>

#include "RSTStdTypes.h"


/* declarations */

static char *t(double minusOne, double aHalf);
static void fillBuf(char **b, char *str);

/***********************************************************************/

void PrintRSTStdTypesConsts(void) {

  fprintf(stdout,
  "+++  RSTree standard types:\n");
  fprintf(stdout,"\n");
  fprintf(stdout,
  "  NAME           TYPE      SIZE COMMENT\n");
  fprintf(stdout,
  "-----------------------------------------\n");
  fprintf(stdout,
  "   byte   %16s %2d   %s\n",t((byte)-1,(byte)1/2),(int)sizeof(byte),
  "");
  fprintf(stdout,
  " Rshort   %16s %2d   %s\n",t((Rshort)-1,(Rshort)1/2),(int)sizeof(Rshort),
  "");
  fprintf(stdout,
  "Rushort   %16s %2d   %s\n",t((Rushort)-1,(Rushort)1/2),(int)sizeof(Rushort),
  "");
  fprintf(stdout,
  "   Rint   %16s %2d   %s\n",t((Rint)-1,(Rint)1/2),(int)sizeof(Rint),
  "");
  fprintf(stdout,
  "  Ruint   %16s %2d   %s\n",t((Ruint)-1,(Ruint)1/2),(int)sizeof(Ruint),
  "");
  fprintf(stdout,
  "  Rlint   %16s %2d   %s\n",t((Rlint)-1,(Rlint)1/2),(int)sizeof(Rlint),
  "context: counters");
  fprintf(stdout,
  " Rulint   %16s %2d   %s\n",t((Rulint)-1,(Rulint)1/2),(int)sizeof(Rulint),
  "");
  fprintf(stdout,
  "    -     %16s %2d   %s\n","void *",(int)sizeof(void *),
  "");
  fprintf(stdout,
  "  Rpint   %16s %2d   %s\n",t((Rpint)-1,(Rpint)1/2),(int)sizeof(Rpint),
  "context: memory- and file- pointers");
  fprintf(stdout,
  " Rpnint   %16s %2d   %s\n",t((Rpnint)-1,(Rpnint)1/2),(int)sizeof(Rpnint),
  "context: subtree pointers, i.e. page numbers");
  fprintf(stdout,
  " Rfloat   %16s %2d   %s\n",t((Rfloat)-1,(Rfloat)1/2),(int)sizeof(Rfloat),
  "");
  fprintf(stdout,"\n");
  fprintf(stdout,
  "+++  RSTree standard constants:\n");
  fprintf(stdout,"\n");
  fprintf(stdout,
  "DESCRIPTION                       NAME            VALUE\n");
  fprintf(stdout,
  "-------------------------------------------------------\n");
  fprintf(stdout,
  "Minimum Rfloat > 0                MIN_RFLOAT      %.16e\n",MIN_RFLOAT);
  fprintf(stdout,
  "IEEE Infinity                     Infinity        %.16e\n",Infinity);
  fprintf(stdout,
  "Maximum length of a name string   MaxNameLength   %d\n",MaxNameLength);
}

/***********************************************************************/

static char *t(double minusOne, double aHalf) {

  if (aHalf != 0.0) {
    return "  floating point";
  }
  else if (minusOne < 0.0) {
    return "  signed integer";
  }
  else {
    return "unsigned integer";
  }
}

/***********************************************************************/

char *strans(char *ch, char *buf)

{
#define f2 "h"
#define f4 "" /* unused */
#if defined (_WIN64)
# define f8 "I64"
#else
# define f8 "ll"
#endif

  char *s= buf;
  boolean isform= FALSE;
  
  while (*ch != 0) {
    if (*ch == '%') {
      // switch formatting, get *ch unchanged
      isform^= 1;
      *buf= *ch;
    }
    else {
      if (! isform) {
        // get *ch unchanged
        *buf= *ch;
      }
      else {
        // test if *ch is terminator
        if (*ch >= 'c' && *ch <= 'g' || *ch >= 'n' && *ch <= 'p' || *ch == 's' || *ch == 'u' || *ch == 'x') {
          // terminate formatting, get *ch unchanged
          isform= FALSE;
          *buf= *ch;
        }
        else if (*ch == 'I') {
          // terminate formatting, replace *ch
          isform= FALSE;
          switch (sizeof(Rint)) {
            case 2: fillBuf(&buf,f2);
              break;
            case 8: fillBuf(&buf,f8);
              break;
          }
          *buf= 'd';
        }
        else if (*ch == 'L') {
          isform= FALSE;
          switch (sizeof(Rlint)) {
            case 2: fillBuf(&buf,f2);
              break;
            case 8: fillBuf(&buf,f8);
              break;
          }
          *buf= 'd';
        }
        else if (*ch == 'N') {
          isform= FALSE;
          switch (sizeof(Rpnint)) {
            case 2: fillBuf(&buf,f2);
              break;
            case 8: fillBuf(&buf,f8);
              break;
          }
          *buf= 'd';
        }
        else if (*ch == 'P') {
          isform= FALSE;
          switch (sizeof(Rpint)) {
            case 2: fillBuf(&buf,f2);
              break;
            case 8: fillBuf(&buf,f8);
              break;
          }
          *buf= 'u';
        }
        else if (*ch == 'Q') {
          isform= FALSE;
          switch (sizeof(Rpint)) {
            case 2: fillBuf(&buf,f2);
              break;
            case 8: fillBuf(&buf,f8);
              break;
          }
          *buf= 'x';
        }
        else if (*ch == 'y') {
          isform= FALSE;
          switch (sizeof(Ruint)) {
            case 2: fillBuf(&buf,f2);
              break;
            case 8: fillBuf(&buf,f8);
              break;
          }
          *buf= 'x';
        }
        else if (*ch == 'Y') {
          isform= FALSE;
          switch (sizeof(Rulint)) {
            case 2: fillBuf(&buf,f2);
              break;
            case 8: fillBuf(&buf,f8);
              break;
          }
          *buf= 'x';
        }
        else {
          // *ch not terminator, get *ch unchanged
          *buf= *ch;
        }
      }
    }
    ch++; buf++;
  }
  *buf= 0;
  return s;
}

/***********************************************************************/

static void fillBuf(char **b, char *str) {

  while (*str != 0) {
    **b= *str;
    (*b)++; str++;
  }
}

/***********************************************************************/

