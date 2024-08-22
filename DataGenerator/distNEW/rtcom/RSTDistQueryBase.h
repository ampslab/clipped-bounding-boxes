/* -----  RSTDistQueryBase.h  ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTDistQueryBase_h
#define __RSTDistQueryBase_h


/**:   (Private) Distance Query Definition
       ===================================                              **/


/**    Implementation:  Norbert Beckmann
              Version:  1.1
                 Date:  01/10                                           **/

/**    Level: bottom.
                                                                        **/

#include "RSTTypes.h"
#include "RSTQueryFuncType.h"
#include "RSTDistQueryTypes.h"
#include "RSTBase.h"


/* ----- types ----- */

typedef boolean (*DistHeapInstFunc) (DistHeap*  /* */,
                                     refDHelem  /* */);

typedef boolean (*DistHeapPopFunc) (DistHeap*  /* */,
                                    refDHelem  /* */);

typedef Rfloat (*DistFunc) (RSTREE        /* RSTREE */,
                            DistCalcFunc  /* DistCalcFunc */,
                            typcoord*     /* point */,
                            typinterval*  /* intv */);

typedef struct {
               RSTREE            R;
               DistHeap          H;
               typcoord          *qPoint;
               DistCalcFunc      CalcDist;
               DistQuerySort     sort;
               DistHeapInstFunc  DH_Inst;
               DistHeapPopFunc   DH_Pop;
               DistFunc          DirDist, DataDist;
               Rfloat            stDist;
               DistFunc          DirStDist;
               typinterval       *fRects;
               Rint              fRectsQty;
               void              *fPtr;
               QueryFunc         DirFilter, DataFilter;
               } distquery;

typedef distquery  *DISTQ; /* distance query identifier */


#endif /* __RSTDistQueryBase_h */

