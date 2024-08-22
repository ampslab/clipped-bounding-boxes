/* -----  RSTLRUPageIO.h  ----- */
#//
#// Copyright (c) 1994 - 2015 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTLRUPageIO_h
#define __RSTLRUPageIO_h


/**:   Pagewise I/O operations from/to LRU buffer
       ==========================================                       **/


/**    Implementation:  Norbert Beckmann
              Version:  2.5
                 Date:  07/15                                           **/

/** Notation:
    - "available": a page, qualified by the parameter pageNr, resides in
      the LRU buffer
    - "node": entry container, view of the tree (as an example)
    - "page": entry container, view of the LRU buffer, the page, qualified
       by the parameter pageNr. **/

/** Functionality:
    The functions of this headerfile provide pagewise read and write
    from/to LRU buffer.
    Properties:
    An LRU buffer provides indirect access to a file, consisting of a set
    of equal sized pages. A page is qualified by its number.
    When the LRU buffer is active (storkind == lru), the nodes of the path
    buffer are links (pointers) into the LRU buffer, i.e. the path buffer
    physically resides in the LRU buffer. Incremental locks assure pages,
    representing a path buffer node, not to be displaced.
    - LRU pages may be linked to a path buffer node (LinkLRUPage)
    - LRU pages, linked to a path buffer node, may be unlinked (UnlinkLRUPage)
    - Arbitrary nodes may be written to certain LRU pages and then be unlinked
      (WriteLRUPage)
    - Arbitrary nodes may be written to certain LRU pages with the link
      persisting or newly being created (WriteLinkLRUPage)
    - LRU pages may be newly established (EstablishLRUPage)
    - LRU pages may be deleted (DeleteLRUPage).
    Level: intermediate.
    Higher levels: RSTPageInOut.h                                       **/


#include "RSTBase.h"


/* declarations */

void UnlinkLRUPage(RSTREE R,
                   typstorage *str,
                   boolean toBeWritten,
                   Rpnint pageNr);

/* A page to be unlinked from the path buffer should be locked in the LRU
   buffer until then, thus:
   Demands available && locked > 0.
   Performs:
   write|= toBeWritten,
   locked--. */
/* Unlink operation */


void WriteLRUPage(RSTREE R,
                  typstorage *str,
                  Rpnint pageNr,
                  refnode nodeptr);

/* The RSTree implementation only writes nodes to pages, which are known to
   be locked in the LRU buffer, thus:
   Demands available && locked > 0.
   Performs:
   COPY node to page,
   write= TRUE,
   locked--. */
/* "Write then unlink" operation */


void WriteLinkLRUPage(RSTREE R,
                      typstorage *str,
                      Rpnint pageNr,
                      refnode *nodeptr);

/* The RSTree implementation only writes nodes to pages, which are known to
   be locked in the LRU buffer, thus:
   Demands available && locked > 0.
   Performs:
   if (*nodePtr != Address(page)) {
     COPY node to page,
     LINK page to node
   },
   write= TRUE.
   // keep lock state. */
/* Write operation */


void LinkLRUPage(RSTREE R,
                 typstorage *str,
                 Rpnint pageNr,
                 refnode *nodeptr);

/* Demands nothing.
   Performs:
   // locate page in the LRU buffer,
   LINK page to node,
   // keep write state,
   locked++.
   if (displacement_Locks error) {
     // extend LRU buffer capacity to the number of used pages (if possible)
   }. */
/* Link operation */


void EstablishLRUPage(RSTREE R,
                      typstorage *str,
                      Rpnint pageNr);

/* The RSTree implementation only establishes pages in the LRU buffer, which
   are known to be unused or beyond EOF in the underlying file, thus:
   Demands "NOT available".
   Performs:
   write= FALSE,
   locked++; // corresponds to locked= 1; (NewLRUEntry(): locked initially 0).
   if (displacement_Locks error) {
     // extend LRU buffer capacity to the number of used pages (if possible)
   }. */
/* Establish operation */
/* Installs an LRU page with the given pageNr for further use. */


void DeleteLRUPage(RSTREE R,
                   typstorage *str,
                   Rpnint pageNr);

/* The LRU buffer implementation only allows pages to be disposed, which are
   known to be available and locked at most once, thus:
   Demands available && locked <= 1.
   Performs:
   // delete page from the LRU buffer. */
/* Forced displace operation without writing (independently of the setting of
   the write flag!) */

#endif /* __RSTLRUPageIO_h */
