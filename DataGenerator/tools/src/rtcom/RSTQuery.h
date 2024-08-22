/* -----  RSTQuery.h  ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTQuery_h
#define __RSTQuery_h


/**:   Query functions
       ===============                                                  **/


/**    Implementation:  Norbert Beckmann
              Version:  5.0
                 Date:  06/14                                           **/

/**    Level: intermediate.
                                                                        **/

#include "RSTOtherFuncs.h"
#include "RSTDistQueryFuncs.h"
#include "RSTDistQueryBase.h"


/* declarations */

boolean RectXsts(RSTREE R,
                 Rint level,
                 const typinterval *rectangle,
                 refinfo info);

/* Unique case pre insertion rectangle EMQ */


boolean RecordXsts(RSTREE R,
                   Rint level,
                   const typinterval *rectangle,
                   const typinfo *info,
                   InfoCmpFunc Qualified,
                   void *dPtr);

/* Pre deletion record EMQ */


void XstsRgn(RSTREE R,
             Rint level,
             const typinterval *qRects,
             Rint qRectQty,
             void *qPtr,
             QueryFunc DirQuery,
             QueryFunc DataQuery,
             boolean *found);

/* Implements RSTree.ExistsRegion */


void RgnCnt(RSTREE R,
            Rint level,
            const typinterval *qRects,
            Rint qRectQty,
            void *qPtr,
            QueryFunc DirQuery,
            QueryFunc DataQuery,
            Rlint *keysqualifying);

/* Implements RSTree.RegionCount */


void RgnQuery(RSTREE R,
              Rint level,
              const typinterval *qRects,
              Rint qRectQty,
              void *qPtr,
              QueryFunc DirQuery,
              QueryFunc DataQuery,
              void *ptr,
              boolean *finish,
              QueryManageFunc Manage);

/* Implements RSTree.RegionQuery */


void All(RSTREE R,
         Rint level,
         void *ptr,
         boolean *finish,
         QueryManageFunc Manage);

/* Implements RSTree.AllQuery */


void PushNodePriorQ(DISTQ DQ,
                    RSTREE R,
                    refnode n,
                    Rint level);

/**/


boolean DistQueryNext(DISTQ DQ,
                      RSTREE R,
                      typinterval *rectangle,
                      refinfo info,
                      Rfloat *rawDist);

/**/


void RectInLevelCnt(RSTREE R,
                    Rint level,
                    Rint targetlevel,
                    typinterval *rectangle,
                    Rint *keysqualifying);

/* Counting rectangle EMQ for rectangles in targetlevel */


void XstsPageNrInLevel(RSTREE R,
                       Rint level,
                       Rint targetlevel,
                       typinterval *rectangle,
                       Rpnint pagenr,
                       boolean *found);

/* Queries for an entry in targetlevel whose entry.ptrtosub matches pagenr,
   i.e. points to an existing page residing in (targetlevel - 1).
   In the levels root-level .. (targetlevel + 1) the parameter rectangle
   restricts the search as in exact match queries.
   NOTE:
   - that in targetlevel XstsPageNrInLevel queries for entry.ptrtosub only.
   - that XstsPageNrInLevel is intended for directory levels only, but
   - that the query is implemented for targetlevel == 0 (the data-level) too,
     such that the first Rpnint-sized field of the info-part must match
     pagenr. */


void XstsDirPageNr(RSTREE R,
                   typlevel Path[],
                   typlevel *nextPath[],
                   Rint level,
                   typinterval *rectangle,
                   Rpnint pagenr,
                   boolean pagelvl_known,
                   Rint known_pagelvl,
                   boolean *found,
                   Rint *levelfound);
                   
/* This function uses the L- and L1-path, and the LRrg-path (P,E) !! */
/* Queries for a directory page number, i.e. queries for an entry in the
   levels root-level .. 2, whose entry.ptrtosub matches pagenr (similar to
   XstsPageNrInLevel). Implemented for the post deletion file
   reorganization. */


void XstsDataPageNr(RSTREE R,
                    typlevel Path[],
                    typlevel *NextPath[],
                    Rint level,
                    typinterval *rectangle,
                    Rpnint pagenr,
                    boolean *found,
                    Rint *levelfound);

/* This function uses the L- and L1-path, and the LRrg-path (P,E) !! */
/* Queries for a data page number, i.e. queries for an entry in level 1,
   whose entry.ptrtosub matches pagenr (similar to XstsPageNrInLevel).
   Implemented for the post deletion file reorganization. */


void LvlDmp(RSTREE R,
            Rint lv,
            void *buf,
            Rint bufSz,
            RectConvFunc Convert,
            RSTName nm[],
            void* buffers[]);

/**/


#endif /* __RSTQuery_h */

