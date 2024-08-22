/* -----  RSTDistQueryTypes.h  ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTDistQueryTypes_h
#define __RSTDistQueryTypes_h


/**:   RSTree - a C frame for R-tree like structures, MAIN INTERFACE
       =============================================================    **/


/**    Implementation:  Norbert Beckmann
              Version:  1.1
                 Date:  01/10                                           **/
                 
/**    Implemented tree:                                                **/

/**:   Guttman's R-tree, R*-tree, RR*-tree, 1D-key R-tree
                                                                        **/
/**:   INTERFACE (distance query specific types)
       =========================================                        **/

/**    Level: normal use.                                               **/


/*** *********************************************************************/

#include "RSTTypes.h"

/*** ---------------- distance query specific types -------------------- */

/* The distance query is a generalization of the incremental nearest
   neighbor search algorithm which on its part is a generalization of the
   k-nearest neighbors search algorithm.
   The query incrementally provides the objects of the data space, starting
   at the nearest/farthest object, sorted by increasing/decreasing distances
   between the objects and a query point.
   RSTree incrementally provides the rectangles of a region (part of the data
   space of a tree) possibly restricted by two different filters. */


typedef void (*DistCalcFunc) (Rfloat   /* coordDist */,
                              Rfloat*  /* cumuDist */);

         /* A function of type DistCalcFunc has to provide the generic
            distance computation.
            Each time the distance query calculates a distance, this function
            is called (at most(*)) numbOfDim(**) times.
            It is responsible, (at most(*)) once called for each dimension,
            successively to compute the (raw(***)) distance in the search
            metrics of choice.
            
            coordDist:
            contains the (possibly negative) distance value along the current
            dimension.
            
            cumuDist:
            initially contains 0.0;
            after (at most(*)) numbOfDim calls contains the calculated
            (raw(***)) distance. This final value is provided in the
            "distance query record" (see GetDistQueryRec in
            RSTDistQueryFuncs.h) as rawDist.
            
            Example:
            for the Euclidian search metrics the applied function should 
            compute:
            *cumuDist+= coordDist * coordDist.
            
            Footnotes:
              *) The call may (or may not) be omitted by the implementation
                 if coordDist == 0.0.
             **) numbOfDim: parameter of CreateRST(), see there.
            ***) In case of the Euclidian search metrics, the real Euclidian
                 distance is the square root of the raw distance, provided
                 in the "distance query record" as rawDist. */
            

typedef enum {
             inc,
             dec
             } DistQuerySort;

         /* determines the sort order of the distance query
            (increasing/decreasing). */


typedef enum {
             minDist,
             cntrDist,
             maxDist
             } RectDistType;

         /* determines how distance is defined.
            
            minDist:
            Distance between the query point and the nearest edge of the
            considered rectangle.
            
            cntrDist:
            Distance between the query point and the center of the
            considered rectangle.
            
            maxDist:
            Distance between the query point and the farthest corner of the
            considered rectangle. */


typedef t_DistQuery  *t_DQ;

        /* abstract data type Distance Query */


#endif /* __RSTDistQueryTypes_h */
