/* -----  RSTDistQueryFuncs.h  ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTDistQueryFuncs_h
#define __RSTDistQueryFuncs_h


/**:   RSTree - a C frame for R-tree like structures, MAIN INTERFACE
       =============================================================    **/


/**    Implementation:  Norbert Beckmann
              Version:  2.3
                 Date:  05/14                                           **/
                 
/**    Implemented tree:                                                **/

/**:   Guttman's R-tree, R*-tree, RR*-tree, 1D-key R-tree
                                                                        **/
/**:   INTERFACE (distance query function)
       ===================================                              **/

/**    Level: normal use.                                               **/


/*** *********************************************************************/

#include "RSTTypes.h"
#include "RSTQueryFuncType.h"
#include "RSTDistQueryTypes.h"

/*** ------------------------ distance queries ------------------------- */

/* The distance query is a generalization of the incremental nearest
   neighbor search algorithm which on its part is a generalization of the
   k-nearest neighbors search algorithm.
   See also publication:
   "Distance Browsing in Spatial Databases"
   [G. Hjaltason, H. Samet; 1999].
   The query incrementally provides the objects of the data space, starting
   at the nearest/farthest object, sorted by increasing/decreasing distances
   between the objects and a query point.
   This implementation incrementally provides the rectangles of a region
   (part of the data space of a tree) possibly restricted by two different
   filters.
   The distance query uses a priority queue whose length (maximum number of
   objects) has to be defined at query start. If there is not enough space,
   the priority queue is extended by a predefined factor. */


void     InitDistQueryIdent(t_DQ  *distQ);

         /* Initializes a distance query identifier (at least sets distQ to
            NULL). Each function which requires a distance query identifier
            checks the value of this identifier at its entry.
            NewDistQuery for example demands an initial identifier while
            GetDistQueryRec demands a non initial identifier. */


boolean  NewDistQuery(t_RT               rst,
                      const typcoord     *queryPoint,
                      DistCalcFunc       DistFunc,
                      DistQuerySort      querySort,
                      RectDistType       distType,
                      Rfloat             startDist,
                      const typinterval  *filterRects,
                      Rint               numbOfFilterRects,
                      void               *filterRefAny,
                      QueryFunc          DirFilter,
                      QueryFunc          DataFilter,
                      Rint               priorQueueMax,
                      boolean            verbosePriorQExt,
                      t_DQ               *distQ);

         /* NewDistQuery opens a new distance query on the tree referenced by
            rst, with the query point queryPoint.
            It allocates the associated memory, initializes the priority
            queue and provides the query identifier distQ. 
            After opening the distance query the function GetDistQueryRec,
            call by call, provides the next query record respectively of the
            query referenced by distQ.
            The query, depending on querySort, nominally starts at the
            nearest/farthest rectangle, providing the resulting records
            ordered by increasing/decreasing distances, distance being
            defined by distType.
            Actually the first n nearest/farthest rectangles up to the
            distance startDist, distance again being defined by distType,
            may be filtered out, i.e. the query may start at startDist.
            An additional filter may be implemented by the functions
            DirFilter and DataFilter which take an array of rectangles and
            an arbitrary reference as parameter. For the latter see also
            QueryFunc (in RSTQueryFuncType.h) and the similar application of
            this type of filter function in the function RegionQuery (in
            RSTOtherFuncs.h).
            
            queryPoint:
            The point the distance query relates to.
            
            DistFunc:
            A function to be provided, to calculate the distance.
            See also DistCalcFunc (in RSTDistQueryTypes.h).
            
            querySort:
            Determines the sort order of the distance query.
            See also DistQuerySort (in RSTDistQueryTypes.h).
            
            distType:
            Determines which type of distance is applied in the query.
            See also RectDistType (in RSTDistQueryTypes.h).
            
            startDist:
            Determines the effective start of the query.
            The part of the query, relating to data rectangles with distances
            smaller/greater (querySort = inc/dec) than startDist, is pruned.
            If startDist is set to 0.0, distance pruning is disabled
            independendly of the setting of querySort.
            
            filterRects:
            Used in connection with the filter-query.
            Passed through to the comparison functions DirFilter and
            DataFilter.
            Query rectangles to be compared by DirFilter/DataFilter with
            the directory- and data rectangles encountered during the
            distance query.
            
            numbOfFilterRects:
            Number of rectangles actually passed by filterRects.
            Passed through to the functions DirFilter and DataFilter.
            
            filterRefAny:
            Arbitrary reference passed through to the functions DirFilter
            and DataFilter.
            
            DirFilter, DataFilter:
            Function parameters passing comparison functions of type
            QueryFunc, returning a boolean.   
            The two functions have to perform the filter-query.   
            See also QueryFunc (in RSTQueryFuncType.h) and RegionQuery (in
            RSTOtherFuncs.h).
            
            priorQueueMax:
            Sets the (initial) number of priority queue elements, the distance
            query will allocate memory for.
            If more space is needed, the priority queue will succesively be
            extended by PriorQueueExtensionFactor.
            The query will be faster of course if no extension is necessary.
            If priorQueueMax is set to values below MinPriorQueueInitMax,
            memory for MinPriorQueueInitMax entries will be allocated.
            See also MinPriorQueueInitMax below.
            
            verbosePriorQExt:
            If verbosePriorQExt is set to TRUE, a message is written to
            stderr at each priority queue extension, otherwise a message is
            only emitted if the concerning attempt fails.
            
            distQ:
            query identifier (see GetDistQueryRec). */


/*   The distance query works on a priority queue, whose arrays are initially
     allocated in NewDistQuery. If more space is needed, the priority queue
     is attempted to be extended by the following factor: */

#define PriorQueueExtensionFactor M_SQRT2

/*   The minimum number of entries, memory is reserved for, is: */

#define MinPriorQueueInitMax 128

/*   Primarily depending on the number of neighbors to be found, but also
     depending on other parameters like the height of the tree and the number
     of entries per page, frequently space for much more entries will be
     needed. For an "all purpose" setting, reasonable values start at about
     1024.
     The standard type setting provided, the actual memory consume (MC) of the
     queue is computed MC = priorQueueMax * (ES + 20), where
     ES = Align(numbOfDim * 2 * sizeof(Rfloat) + sizeof(typinfo), 8) and
     Align(v, a) is a function, returning the smallest possible integer
     i >= v, i mod a = 0.
     Example:
     Assuming priorQueueMax = 2048, numbOfDim 3, sizeof(typinfo) = 4
     MC = 2048 * (Align(3 * 2 * 8 + 4, 8) + 20)
        = 2048 * (56 + 20) = 155648 */


boolean  DisposeDistQuery(t_DQ  *distQ);

         /* DisposeDistQuery closes the distance query referenced by distQ.
            It deallocates the associated memory and reinitializes distQ. */


boolean  GetDistQueryRec(t_DQ         distQ,
                         t_RT         rst,
                         typinterval  *rectangle,
                         refinfo      info,
                         Rfloat       *rawDist,
                         boolean      *exists);

         /* Beginning with the first rectangle, sorted by
            increasing/decreasing distances, call by call, GetDistQueryRec
            successively provides the records of the distance query
            referenced by distQ.
            The actual behavior depends on parameters, passed to the
            function NewDistQuery.
            
            distQ:
            query identifier provided by the function NewDistQuery.
            
            rst:
            corresponding tree identifier.
            
            ---------- distance query record ----------
            rectangle:
            contains the data rectangle found.
            
            info:
            contains the associated information part.
            
            rawDist:
            contains the raw distance between the rectangle and the query
            point, as computed by (at most) numbOfDim(*) calls of the applied
            function of type DistCalcFunc (see RSTDistQueryTypes.h).
            --------------------- ---------------------
            
            exists:
            is set to TRUE if distQueryRec is a valid newly found entry,
            otherwise set to FALSE. In the latter case, if the function
            returns TRUE, the complete set of stored records has already
            been given out.

            *) numbOfDim: parameter of CreateRST(), see there. */


#endif /* __RSTDistQueryFuncs_h */
