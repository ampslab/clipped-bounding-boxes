/* -----  RSTInstDel.h  ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTInstDel_h
#define __RSTInstDel_h


/**:   Insertion and Deletion of records, Split
       ========================================                         **/


/**    Implementation:  Norbert Beckmann
              Version:  9.1
                 Date:  01/10                                           **/

/**    Level: intermediate.
                                                                        **/

#include "RSTBase.h"


/* declarations */

void Insert(RSTREE R,
            void *newentry,
            Rint level);

void DeleteOneRec(RSTREE R, const typinterval *delRect);


#endif /* __RSTInstDel_h */
