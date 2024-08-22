/* ----- RSTSupport.h ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTSupport_h
#define __RSTSupport_h


/**:   Performance Support (Pre-Allocation, )
       ======================================                           **/


/**    Implementation:  Norbert Beckmann
              Version:  1.0
                 Date:  05/14                                           **/

/**    Level: bottom.
                                                                        **/


#include "RSTBase.h"


/* constants */


/* types */


/* declarations */

void SetSizesPreAllocArrs(RSTREE R);

/* SetSizesPreAllocArrs() sets important sizes, and pre-allocates re-usable
   arrays. The function is self re-initializing when failing:
   When consecutively allocating the pointers, one of the allocations fails,
   the procedure stops, DeallocArrs is called, and RSTDone is set to FALSE. */


void DeallocArrs(RSTREE R);

/* DeallocArrs() deallocates re-usable arrays. */

#endif /* __RSTSupport_h */
