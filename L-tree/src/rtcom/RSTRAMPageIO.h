/* -----  RSTRAMPageIO.h  ----- */
#//
#// Copyright (c) 1994 - 2015 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTRAMPageIO_h
#define __RSTRAMPageIO_h


/**:   Pagewise I/O operations from/to RAM-disk
       ========================================                         **/


/**    Implementation:  Norbert Beckmann
              Version:  2.3
                 Date:  07/15                                           **/

/** Notation:
    - "node": entry container, view of the tree (as an example)
    - "page": entry container, view of the RAM-disk, the page, qualified
       by the parameter pagenr. **/

/** Functionality:
    The functions of this headerfile provide pagewise read and write
    from/to RAM-disks.
    Properties:
    A RAM-disk can be thought to consist of a set of equal sized pages.
    A page is qualified by its number.
    A RAM page may be read to a node, or be linked to a node pointer
    (direct access). Nodes may be written to RAM pages.
    Level: intermediate.
    Higher levels: RSTPageInOut.h                                       **/


#include "RSTBase.h"


/* declarations */

void WriteRAMPage(typstorage *str,
                  Rpnint pagenr,
                  void *block);
/* COPY node to page */

void WriteLinkRAMPage(typstorage *str,
                      Rpnint pagenr,
                      refnode *nodeptr);
/* if (nodePtr != Address(page)) {
     COPY node to page,
     LINK page to node
   } */

void LinkRAMPage(typstorage *str,
                 Rpnint pagenr,
                 refnode *nodeptr);
/* Link page to nodeptr */

void ReadRAMPage(typstorage *str,
                 Rpnint pagenr,
                 void *block);
/* COPY page to node */

#endif /* __RSTRAMPageIO_h */
