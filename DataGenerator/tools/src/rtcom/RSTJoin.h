/* -----  RSTJoin.h  ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTJoin_h
#define __RSTJoin_h


/**:   Join functions
       ==============                                                   **/


/**    Implementation:  Norbert Beckmann
              Version:  3.3
                 Date:  01/14                                           **/

/**    Level: intermediate.
                                                                        **/

#include "RSTOtherFuncs.h"
#include "RSTDistQueryFuncs.h"
#include "RSTBase.h"


/* types */

typedef boolean (*RectCompFunc) (RSTREE,
                                 const typinterval*,
                                 const typinterval*);


/* declarations */

void XJnCnt(RSTREE R1,
            Rint level1,
            const typinterval *R1qRects,
            Rint R1qRectQty,
            void *R1qPtr,
            QueryFunc Dir1Query,
            QueryFunc Data1Query,
            RSTREE R2,
            Rint level2,
            const typinterval *R2qRects,
            Rint R2qRectQty,
            void *R2qPtr,
            QueryFunc Dir2Query,
            QueryFunc Data2Query,
            JoinFunc DirJoin,
            JoinFunc DataJoin,
            Rlint *keysqualifying,
            Rlint *cntlmt);
/* internal comparison counting: does not exist: all comparison functions
   are user implemented */

void XJn(RSTREE R1,
         Rint level1,
         const typinterval *R1qRects,
         Rint R1qRectQty,
         void *R1qPtr,
         QueryFunc Dir1Query,
         QueryFunc Data1Query,
         RSTREE R2,
         Rint level2,
         const typinterval *R2qRects,
         Rint R2qRectQty,
         void *R2qPtr,
         QueryFunc Dir2Query,
         QueryFunc Data2Query,
         JoinFunc DirJoin,
         JoinFunc DataJoin,
         void *mPtr,
         boolean *finish,
         JoinManageFunc Manage);
/* internal comparison counting: does not exist: all comparison functions
   are user implemented */

void XSpJnCnt(RSTREE R1,
              Rint level1,
              const typinterval *R1qRects,
              Rint R1qRectQty,
              void *R1qPtr,
              QueryFunc Dir1Query,
              QueryFunc Data1Query,
              RSTREE R2,
              Rint level2,
              const typinterval *R2qRects,
              Rint R2qRectQty,
              void *R2qPtr,
              QueryFunc Dir2Query,
              QueryFunc Data2Query,
              typinterval *interRect,
              RectCompFunc DirJoin,
              JoinFunc DataJoin,
              Rlint *keysqualifying,
              Rlint *cntlmt);
/* internal comparison counting: is done in R1, and available through
   function GetCountRectComp, whose results must be added to the number of
   comparisons externally counted in the user implemented comparison
   functions */

void XSpJn(RSTREE R1,
           Rint level1,
           const typinterval *R1qRects,
           Rint R1qRectQty,
           void *R1qPtr,
           QueryFunc Dir1Query,
           QueryFunc Data1Query,
           RSTREE R2,
           Rint level2,
           const typinterval *R2qRects,
           Rint R2qRectQty,
           void *R2qPtr,
           QueryFunc Dir2Query,
           QueryFunc Data2Query,
           typinterval *interRect,
           RectCompFunc DirJoin,
           JoinFunc DataJoin,
           void *mPtr,
           boolean *finish,
           JoinManageFunc Manage);
/* internal comparison counting: is done in R1, and available through
   function GetCountRectComp, whose results must be added to the number of
   comparisons externally counted in the user implemented comparison
   functions */

void SpJnCnt(RSTREE R1,
             Rint level1,
             RSTREE R2,
             Rint level2,
             typinterval *interRect,
             Rlint *keysqualifying,
             Rlint *cntlmt);
/* internal comparison counting only: is done in R1, and available
   through function GetCountRectComp */

void SpJn(RSTREE R1,
          Rint level1,
          RSTREE R2,
          Rint level2,
          typinterval *interRect,
          void *mPtr,
          boolean *finish,
          JoinManageFunc Manage);
/* internal comparison counting only: is done in R1, and available
   through function GetCountRectComp */


#endif /* __RSTJoin_h */

