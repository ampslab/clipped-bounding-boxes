/* -----  RSTRpnintBag.h  ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTRpnintBag_h
#define __RSTRpnintBag_h


/**:   Ordered "open Bag" of Rpnint
       ============================                                     **/


/**    Implementation:  Norbert Beckmann
              Version:  5.0
                 Date:  06/14                                           **/

/** Properties:
    Ordered bag of Rpnint of arbitrary cardinality
    Notation:
    An element is a tuple (value, quantity) where quantity is the number of
    instances of value in the bag.
    value is of type Rpnint, the number of instances is of type Rint.
    The bag is ordered by element.value.
    NOTE:
    Contrary to what is commonly called "bag", this "open bag"
    implementation allows elements with negative numbers of instances to
    be stored. An element is deleted from the bag if an operation leads
    to a number of 0 instances for it, otherwise it is hold with a
    positive or negative number of instances.
    Level: normal use.                                                  **/


#include <stdio.h>

#include "RSTStdTypes.h"


/* ----- types ----- */

typedef t_RpnintBag  *t_pnB;

        /* abstract data type t_pnB,
           open bag of Rpnint. */


/* ----- declarations ----- */

t_pnB  PNB_Empty(void);

/* returns an initial bag, i.e. NULL. */


void  PNB_Delete(t_pnB *bag);

/* deallocates the memory of bag, reinitializes bag. */


Rint  PNB_Include(t_pnB *bag, Rpnint value, Rint quantity, boolean *Done);

/* adds quantity instances of value to the bag;
   returns the new number of instances of the element with value value.
   Done:
   Done is intended to maintain a success-variable in the callers
   environment, once set to TRUE at program start. It should keep FALSE
   after an error occurred though passed through multiple functions.
   Hence Done is NEVER set to TRUE. It is set to FALSE if an error
   occurred. Errors additionally produce messages on stderr.
   Done might only be set to FALSE if 0 is returned. */


Rint  PNB_Exclude(t_pnB *bag, Rpnint value, Rint quantity);

/* subtracts quantity instances of value from the bag;
   if the number of instances of value becomes 0, the element is deleted;
   returns the new number of instances of value. */


Rint  PNB_Pop(t_pnB *bag, Rpnint *value);

/* fast output-then-delete routine (unsorted output);
   picks an element, passes its value to value then deletes it;
   returns the number of instances of the popped element. */


Rint  PNB_PopMin(t_pnB *bag, Rpnint *value);

/* output-then-delete routine (sorted by increasing values);
   picks the minimal element, passes its value to value then deletes it;
   returns the number of instances of the popped element. */


Rint  PNB_PopMax(t_pnB *bag, Rpnint *value);

/* output-then-delete routine (sorted by decreasing values);
   picks the maximal element, passes its value to value then deletes it;
   returns the number of instances of the popped element. */


Rint  PNB_In(t_pnB bag, Rpnint value);

/* returns the number of instances of the element with value value. */


Rint  PNB_NumbElems(t_pnB bag);

/* returns the number of elements in bag. */


Rint  PNB_NumbInsts(t_pnB bag);

/* returns the number of instances in bag.
   See NOTE at the head. Negative numbers of instances may lead to
   curios results! */


void PNB_Print(FILE *stream, t_pnB bag, Rint epl);

/* prints the elements of a bag to stream as a text, in lines of epl elements
   per line.
   Notation: " <quantity>*<value> <quantity>*<value> ...". */


#endif /* __RSTRpnintBag_h */
