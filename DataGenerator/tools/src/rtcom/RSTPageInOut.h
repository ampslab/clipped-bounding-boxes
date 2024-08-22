/* -----  RSTPageInOut.h  ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTPageInOut_h
#define __RSTPageInOut_h


/**:   Pagewise I/O operations independently of the storage type
       =========================================================        **/


/**    Implementation:  Norbert Beckmann
              Version:  3.3
                 Date:  07/14                                           **/

/** Properties / Functionality:
    Pagewise I/O independently of the storage type,
    page-number management,
    media reorganization (after deletion).
    Level: normal use.                                                  **/


#include "RSTBase.h"


/***
     Special notes concerning the following I/O operations for pages:
     UnlinkPage, PutNode, PutExtNode, PutGetNode, GetNode.

Preliminary remark:
This header file belongs to an RSTree implementation which provides three
different storage types. Independently of the storage type the implementation
internally buffers one single path (two paths during reorganization after
deletion).

Notation:
node: entry container, view of the tree.
page: entry container, view of the storage medium.

The storage types (storkind):
1. "secondary memory" (sec): Nodes of the path are copies of pages from
                             secondary memory; I/Os from/to secondary memory.
2. "RAM disk" (pri): Nodes of the path are links (pointers) into the RAM
                     disk; I/Os from/to RAM disk.
3. "LRU buffer" (lru): Nodes of the path are links (pointers) into the LRU
                       buffer, corresponding locks assure that linked pages
                       are not displaced; I/Os from/to LRU buffer.
                 
The I/O operations considered in this section serve for the transfer of single
nodes (tree) to pages (storage medium) and vice versa. They work independently
of the three storage types provided by the RSTree implementation.
Nodes (tree) are identified by a parameter "nodeptr" whereas pages (storage
medium) are identified by a parameter "pagenr".

UnlinkPage
  is a pure "LRU buffer" operation, a no op otherwise, and necessary, because
  in contrast to the situation when the two other storage methods are used,
  locks have to be freed in the LRU buffer if a page does not longer belong to
  the path buffer (see also PutNode).

PutNode
  is subject to an unavoidable semantics change if applied to storage type
  "LRU buffer"!
  If the storage type is "secondary memory", it is a simple write of a node,
  thereby not giving up the buffered information (of course).
  If the storage type is "RAM disk" it is a no op and hence follows the same
  semantics.
  If the storage type is "LRU buffer" however, PutNode DOES give up the link
  to the concerning page in that it frees its lock (*) as UnlinkPage does;
  actually PutNode is called if the node has to be written to the
  corresponding file finally, whereas UnlinkPage is called if it should not.
  A closer view to the real implementation however reveals that the semantics
  change is of rather theoretical relevance: Since for storage type "secondary
  memory" PutNode is a true write to a file, it should anyway (apart from the
  special case of a "sync" operation) only be called if the call is
  immediately followed by a GetNode of another page, this way in fact giving
  up the old node. Thus the semantics difference can be accepted.
  The special case, that a node shall be written, but not unlinked (i.e. the
  "normal" secondary memory semantics of a PutNode), is covered by the
  function PutGetNode, which is more powerful however.
  *) The unlock operation is necessary finally, and there is no other instance
     which could do it.

Remark:  The functions UnlinkPage and PutNode do not really remove the link
between node and page (nodeptr unchanged). They merely allow the concerning
page to be displaced at some time, whereby nodeptr will become illegal or even
point to another page. The RSTree implementation itself guarantees not to
access pages whose locks have been freed.

PutExtNode
  is needed since the path may be linked. It is designed for writing an
  external, i.e. not path residing node to some page, and hence is not (as
  PutNode) a no op for storage type "RAM disk". It writes a node to the
  designated page.
  In case of storage type "LRU buffer" it assumes the page being available and
  locked in the buffer. After writing the page, the page is unlocked.

PutGetNode
  is needed since the path may be linked. It is designed for writing an
  arbitrary node, i.e. path residing or not, to some page, and then (for the
  linked buffer types "RAM disk" or "LRU buffer") linking the page to the
  node.
  In case of storage type "secondary memory" the node is simply written to the
  designated page, as for all other Put operations. In case of the linked
  buffer types, it is first tested, whether the addressed page is already
  linked to the concerning node. If not, the node is copied to the page, and
  then the page is linked to the node.
  In case of storage type "LRU buffer", the page is additionally marked to be
  written back if being displaced at some time, whereas the lock state is left
  as is.

GetNode
  is a read, link respectively, of a page to a node.
***/


/* declarations */

void UnlinkPage(RSTREE R,
                Rpnint pagenr,
                Rint level);

/* pri: no action.
   lru: UnlinkLRUPage(FALSE):
        write|= FALSE (* write unmodified *),
        locked--.
   sec: no action.
   --> Counts: none.
*/

void PutNode(RSTREE R,
             refnode nodeptr,
             Rpnint pagenr,
             Rint level);

/* pri: no action.
   lru: UnlinkLRUPage(TRUE):
        write|= TRUE (* write= TRUE *),
        locked--.
   sec: WritePage.
   --> Counts: put++.
*/

void PutExtNode(RSTREE R,
                refnode nodeptr,
                Rpnint pagenr,
                Rint level);

/* pri: WriteRAMPage.
   lru: WriteLRUPage():
        COPY node to page,
        write= TRUE,
        locked--.
   sec: WritePage.
   --> Counts: put++.
*/

void PutGetNode(RSTREE R,
                refnode *nodeptr,
                Rpnint pagenr,
                Rint level);

/* pri: WriteLinkRAMPage.
   lru: WriteLinkLRUPage():
        if (nodePtr != Address(page)) {
          COPY node to page,
          LINK page to node
        },
        write= TRUE,
        (* locked unmodified *).
   sec: WritePage.
   --> Counts: put++.
*/

void GetNode(RSTREE R,
             refnode *nodeptr,
             Rpnint pagenr,
             Rint level);

/* pri: LinkRAMPage.
   lru: LinkLRUPage():
        LINK page to node,
        (* write unmodified *),
        locked++.
   sec: ReadPage.
   --> Counts: get++.
*/

boolean PathLRUConsistence(RSTREE R);

/* Compares the numbers of the pages in the standard path with the locks in
   the LRU buffer.
   Returns TRUE if path and LRU buffer are consistent concerning locks,
   FALSE otherwise. */

void NewNode(RSTREE R,
             Rint level);
/* Input information: level of the node to be fetched, E[level+1].
   If necessary writes back N, sets Modif off.
   Sets P, fetches the corresponding page into N.
   --> Does not check if N already contains the right page. */

void ExtendPath(RSTREE R,
                Rint ind,
                Rint level);
/* Input information: current level, index.
   Does all necessary checks and work in level and level-1.
   At the end N[level-1] contains the corresponding page. */
   
void ExtendReorgPath(RSTREE R,
                     typlevel Path[],
                     Rint ind,
                     Rint level,
                     typlevel *nextPath[]);
/* This procedure uses the NReorg-path !! */
/* Like ExtendPath. */

void GetPageNr(RSTREE R,
               Rpnint *pagenr,
               Rint level);

void PutPageNr(RSTREE R,
               Rpnint pagenr,
               Rint level);

void ReorgMedia(RSTREE R,
                boolean shortest,
                boolean *reorgactiv,
                boolean *truncactiv);


#endif /* __RSTPageInOut_h */
