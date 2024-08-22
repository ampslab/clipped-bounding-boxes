/* -----  RSTRpnintSet.h  ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTRpnintSet_h
#define __RSTRpnintSet_h


/**:   Ordered Set of Rpnint
       =====================                                            **/


/**    Implementation:  Norbert Beckmann
              Version:  5.0
                 Date:  06/14                                           **/

/** Properties:
    Ordered set of Rpnint of arbitrary cardinality.
    Level: normal use.                                                  **/


#include <stdio.h>

#include "RSTStdTypes.h"


/* ----- types ----- */

typedef t_RpnintSet  *t_pnS;

        /* abstract data type t_pnS,
           set of Rpnint. */


/* ----- declarations ----- */

t_pnS  PNS_Empty(void);

/* returns an initial set, i.e. NULL. */


void  PNS_Delete(t_pnS *set);

/* deallocates the memory of set, reinitializes set. */


boolean  PNS_Include(t_pnS *set, Rpnint value, boolean *Done);

/* includes value into set;
   returns TRUE if value has been newly included, otherwise FALSE.
   Done:
   Done is intended to maintain a success-variable in the callers
   environment, once set to TRUE at program start. It should keep FALSE
   after an error occurred though passed through multiple functions.
   Hence Done is NEVER set to TRUE. It is set to FALSE if an error
   occurred. Errors additionally produce messages on stderr.
   Done might only be set to FALSE if FALSE is returned. */


boolean  PNS_Exclude(t_pnS *set, Rpnint value);

/* excludes value from set;
   returns TRUE if value has been in set, otherwise FALSE. */


boolean  PNS_Pop(t_pnS *set, Rpnint *value);

/* fast output-then-delete routine (unsorted output),
   picks an element, passes it to value then excludes it;
   returns TRUE if set has not been empty, otherwise
   returns FALSE and leaves value unmodified. */


boolean  PNS_PopMin(t_pnS *set, Rpnint *value);

/* output-then-delete routine (sorted by increasing values),
   picks the minimal element, passes it to value then excludes it;
   returns TRUE if set has not been empty, otherwise
   returns FALSE and leaves value unmodified. */


boolean  PNS_PopMax(t_pnS *set, Rpnint *value);

/* output-then-delete routine (sorted by decreasing values),
   picks the maximal element, passes it to value then excludes it;
   returns TRUE if set has not been empty, otherwise
   returns FALSE and leaves value unmodified. */


boolean  PNS_In(t_pnS set, Rpnint value);

/* returns TRUE if value is in set, otherwise FALSE. */


Rint  PNS_Cardinality(t_pnS set);

/* returns the cardinality of set. */


void PNS_Print(FILE *stream, t_pnS set, Rint epl);

/* prints the elements of a set to stream as a text, in lines of epl elements
   per line.
   Notation: " <value> <value> ...". */


void PNS_PrintTree(FILE *stream, t_pnS set);

/* in a tree like notation prints the elements of a set to stream.
   The output is a long row, consisting of left and right brackets, dots "."
   and data. Subtrees are enclosed in brackets, dots denote NULL. */


#endif /* __RSTRpnintSet_h */
