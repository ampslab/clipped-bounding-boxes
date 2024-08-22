/* -----  RSTDistHeap.h  ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTDistHeap_h
#define __RSTDistHeap_h


/**:   Distance Heap
       =============                                                    **/


/**    Implementation:  Norbert Beckmann
              Version:  2.2
                 Date:  12/13                                           **/

/**    Level: bottom.
                                                                        **/

#include "RSTBase.h"


/* declarations */

boolean DH_New(DistHeap *h,
               Rint elemLen,
               Rint max,
               Rfloat extFac,
               boolean verbose);

/* demands the address of a variable of type DistHeap,
   initializes the DistHeap h and attempts to allocate the concerning memory,
   returns TRUE if h is ready for use,
   returns FALSE otherwise */


void DH_Dispose(DistHeap *h);

/* deallocates the memory of h */


boolean minDH_Insert(DistHeap *h, refDHelem elem);

/* attempts to insert elem into h ordered by INCREASING dist,
   building up a MinHeap,
   returns TRUE if elem was inserted (memory was (made) available),
   returns FALSE otherwise */


boolean maxDH_Insert(DistHeap *h, refDHelem elem);

/* attempts to insert elem into h ordered by DECREASING dist,
   building up a MaxHeap,
   returns TRUE if elem was inserted (memory was (made) available),
   returns FALSE otherwise */


boolean minDH_Pop(DistHeap *h, refDHelem elem);

/* attempts to remove the first MINIMAL element from h, a MinHeap,
   returns TRUE and provides the element in elem if h is not empty,
   returns FALSE otherwise */


boolean maxDH_Pop(DistHeap *h, refDHelem elem);

/* attempts to remove the first MAXIMAL element from h, a MaxHeap,
   returns TRUE and provides the element in elem if h is not empty,
   returns FALSE otherwise */


Rfloat DH_TopDist(DistHeap *h);

/* returns the distance of the first (MINIMAL/MAXIMAL) element of h,
   if h is empty the result is undefined */


Rint DH_Qty(DistHeap *h);

/* returns the number of elements of h */


Rint DH_MaxQty(DistHeap *h);

/* returns the current maximum number of elements of h */


#endif /* __RSTDistHeap_h */
