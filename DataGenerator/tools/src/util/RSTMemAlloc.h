/* -----  RSTMemAlloc.h  ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTMemAlloc_h
#define __RSTMemAlloc_h


/**:   Standard Memory Allocation (wrappers for malloc and free)
       =========================================================        **/


/**    Implementation:  Norbert Beckmann
              Version:  2.3
                 Date:  05/14                                           **/

/**    Level: normal use.                                               **/


#include "RSTStdTypes.h"


void *allocM(Rpint size);

/* allocM() returns a pointer to a block of at least size bytes, suitably
   aligned for any use.
   If there is no available memory, allocM returns a null pointer. */


void *reallocM(void *ptr, Rpint size);

/* reallocM() changes the size of the block, pointed to by ptr, to size bytes,
   and returns a pointer to the (possibly moved) block. The contents will
   be unchanged up to the lesser of the new and old sizes. If the new size of
   the block requires movement of the block, the space for the previous
   instantiation of the block is freed. If ptr is NULL, reallocM() behaves
   like allocM() for the specified size. If size is 0, and ptr is not a null
   pointer, the space pointed to is made available for further allocation by
   the application.
   If there is no additional available memory, reallocM returns a null
   pointer, but the block pointed to by ptr is left intact. */


#define freeM(x) free(*(x)); *(x)= NULL

/* The macro replaces the following function:

void freeM(void * *ptr);

   The argument to freeM() is the address of a pointer to a block previously
   allocated by allocM() or reallocM(). After freeM() is executed,
   - the previously allocated space is made available for further allocation
     by the application
   - the pointer is set to NULL(!).
   
   NOTE:
   freeM() could not be implemented by a true function because:
   - the address of an arbitrary pointer may not be assigned to a (void **)
     function parameter without causing warnings, whereas
   - an arbitrary pointer may be assigned to a (void *) function parameter.
   Be that as it may; with the macro above, the compiler emits proper error
   messages if freeM() is called with an incorrect parameter. */


#endif /* __RSTMemAlloc_h */

