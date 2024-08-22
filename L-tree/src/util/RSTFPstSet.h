/* -----  RSTFPstSet.h  ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTFPstSet_h
#define __RSTFPstSet_h


/**:   Ordered Set of FPst
       ===================                                              **/


/**    Implementation:  Norbert Beckmann
              Version:  2.0
                 Date:  06/14                                           **/

/** Properties:
    Ordered set of FPst of arbitrary cardinality.
    Level: normal use.                                                  **/


#include <stdio.h>

#include "RSTStdTypes.h"


/* ----- types ----- */

typedef t_FilePageSet  *t_FPS;

        /* abstract data type t_FPS,
           set of FPst. */


/* ----- declarations ----- */

t_FPS  FPS_Empty(void);

/* returns an initial set, i.e. NULL. */


void  FPS_Delete(t_FPS *set);

/* deallocates the memory of set, reinitializes set. */


boolean  FPS_Include(t_FPS *set, FPst value, boolean *Done);

/* includes value into set;
   returns TRUE if value has been newly included, otherwise FALSE.
   Done:
   Done is intended to maintain a success-variable in the callers
   environment, once set to TRUE at program start. It should keep FALSE
   after an error occurred though passed through multiple functions.
   Hence Done is NEVER set to TRUE. It is set to FALSE if an error
   occurred. Errors additionally produce messages on stderr.
   Done might only be set to FALSE if FALSE is returned. */


boolean  FPS_Exclude(t_FPS *set, FPst value);

/* excludes value from set;
   returns TRUE if value has been in set, otherwise FALSE. */


boolean  FPS_Pop(t_FPS *set, FPst *value);

/* fast output-then-delete routine (unsorted output),
   picks an element, passes it to value then excludes it;
   returns TRUE if set has not been empty, otherwise
   returns FALSE and leaves value unmodified. */


boolean  FPS_PopMin(t_FPS *set, FPst *value);

/* output-then-delete routine (sorted by increasing values),
   picks the minimal element, passes it to value then excludes it;
   returns TRUE if set has not been empty, otherwise
   returns FALSE and leaves value unmodified. */


boolean  FPS_PopMax(t_FPS *set, FPst *value);

/* output-then-delete routine (sorted by decreasing values),
   picks the maximal element, passes it to value then excludes it;
   returns TRUE if set has not been empty, otherwise
   returns FALSE and leaves value unmodified. */


boolean  FPS_In(t_FPS set, FPst value);

/* returns TRUE if value is in set, otherwise FALSE. */


Rint  FPS_Cardinality(t_FPS set);

/* returns the cardinality of set. */


void FPS_Print(FILE *stream, t_FPS set, Rint epl);

/* prints the elements of a set to stream as a text, in lines of epl elements
   per line.
   Notation: " f<file>.p<page> f<file>.p<page> ...". */


void FPS_PrintTree(FILE *stream, t_FPS set);

/* in a tree like notation prints the elements of a set to stream.
   The output is a long row, consisting of left and right brackets, dots "."
   and data. Subtrees are enclosed in brackets, dots denote NULL. */


#endif /* __RSTFPstSet_h */
