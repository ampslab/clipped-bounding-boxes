/* -----  RSTChooseSub.h  ----- */
#//
#// Copyright (c) 1994 - 2013 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTChooseSub_h
#define __RSTChooseSub_h


/**:   ChooseSubtree, ChooseMerge
       ==========================                                       **/


/**    Implementation:  Norbert Beckmann
              Version:  2.1
                 Date:  01/10                                           **/

/**    Level: intermediate.
                                                                        **/

#include "RSTBase.h"


/* declarations */

void     ChooseSubtree(RSTREE R,
                       typinterval *newrect,
                       Rint level,
                       refnode DIRnode,
                       Rint *ncov,
                       Rint I[]);
/* ChooseSubtree provides the insertion index in I[0]. If coverage by more
   than one rectangle of the current node is detected, the indices of the
   remaining rectangles (entries) are provided in I[1] .. I[ncov-1]. */


void     ChooseMerge(RSTREE R,
                     typinterval *newrect,
                     Rint skip,
                     refnode DIRnode,
                     Rint *ncov,
                     Rint I[]);
/* ChooseMerge provides the merge index in I[0]. Implementation derived
   from ChooseSubtree (see there). */

#endif /* __RSTChooseSub_h */

