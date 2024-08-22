/* -----  RSTMemAlloc.c  ----- */
#//
#// Copyright (c) 1994 - 2012 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//


#include <stdlib.h>

#include "RSTMemAlloc.h"


/*********************************************************************/

void *allocM(Rpint size)

{
  return malloc(size);
}

/*********************************************************************/

void *reallocM(void *ptr, Rpint size)

{
  return realloc(ptr,size);
}

/*********************************************************************/
/*
void freeM(void * *ptr) is a macro, see header file.
*/
/*********************************************************************/
