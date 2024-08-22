/* -----  RSTQueryFuncType.h  ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTQueryFuncType_h
#define __RSTQueryFuncType_h


/**:   RSTree - a C frame for R-tree like structures, MAIN INTERFACE
       =============================================================    **/


/**    Implementation:  Norbert Beckmann
              Version:  2.1
                 Date:  01/10                                           **/
                 
/**    Implemented tree:                                                **/

/**:   Guttman's R-tree, R*-tree, RR*-tree, 1D-key R-tree
                                                                        **/
/**:   INTERFACE (query function type)
       ===============================                                  **/

/**    Level: normal use.                                               **/


/*** *********************************************************************/

#include "RSTTypes.h"

/*** --------------------- query function type ------------------------- */

typedef boolean (*QueryFunc) (t_RT                /* rst */,
                              Rint                /* numbOfDim */,
                              const typinterval*  /* dRect */,
                              const typinterval*  /* queryRectArray */,
                              Rint                /* numbOfQueryRects */,
                              void*               /* queryRefAny */);

        /* QueryFunc is the type of a filter function, to be provided with
           the call of various query functions in the RSTree interface.
           Basically, it has to provide the boolean result of the comparison
           of a rectangle (dRect), currently considered in the corresponding
           algorithm, with another (array of) rectangle(s) (queryRectArray)
           provided by the caller. The parameter queryRefAny allows arbitrary
           other information to be included into the computation.
           
           rst:
           contains tree identifier of the tree, the query is applied to.
           
           numbOfDim:
           contains the dimensionality of the tree, defined at its creation.
           
           dRect:
           contains the directory or data rectangle respectively, currently
           considered in the respective function.
           
           queryRectArray:
           contains the query rectangles passed to the respective function.
           
           numbOfQueryRects:
           contains the parameter numbOfQueryRects passed to the respective
           function.
           
           queryRefAny:
           contains the parameter queryRefAny passed to the respective
           function. */


#endif /* __RSTQueryFuncType_h */
