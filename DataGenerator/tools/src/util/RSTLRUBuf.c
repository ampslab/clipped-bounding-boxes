/* ----- RSTLRUBuf.c ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//


#include <stdlib.h>
#include <stdio.h>

#include "RSTLRUBuf.h"
#include "RSTFileAccess.h"
#include "RSTErrors.h"
#include "RSTMemAlloc.h"


/* NOTE: the function parameter Done is NEVER set to TRUE. */


/**    Main principle:
       The LRU buffer consists of lrubuf.hash, a vector of pointers to (lists
       of) LRU entries. hash is allocated with the capacity of the buffer.
       List pointers: first list entry:        (*lrubuf).hash[index],
                      following list entries:  (*typLRUe).other.
       The array of lists is used for buffering and hashing.
       An entry in the list hash[index] knows the anchor of its list, because
       it knows index.
       
       Additionally each LRU entry is member of a priority list.
       List pointers: (*typLRUe).pre, (*typLRUe).suc.
       The priority list is used to maintain the LRU principle.
       
       A new entry is stored in the list hash[index], where index is the hash
       value, calculated from the number of the page the entry contains.
       Additionally the new entry is made member in the priority list as
       head.
       If displacement of an entry is necessary, the tail entry of the
       priority list is cut off, and the entry is excluded from the list
       hash[its Index].
       Available entries only change their position in the priority list.
       They are first excluded from it and then stored as head.
       
       The buffer utilization functions provide a pointer to the struct
       (*typLRUe).pub, which is used for the maintenance of the page and
       the flags.
       
       Peculiarities:
       An unusual facility of this LRU buffer is, that entries may be locked
       in the buffer, i.e. may be prevented from being displaced. This causes
       the possibility that the buffer may become overfilled.
       Since entries are allocated independently of the filling degree, the
       sole impact of an overfilled buffer is, that hashing degenerates due
       to growing lists. However, displacement errors cause warnings in
       CutTailEntry(). **/


/* ----- types ----- */

typedef struct typLRUe {
                       Rpnint          index;
                       struct typLRUe  *other;
                       struct typLRUe  *pre;
                       struct typLRUe  *suc;
                       FPst            fp;
                       LRUentry        pub;
                       } typLRUe, *refLRUe;

typedef struct {
               boolean  on;
               Rlint    read;
               Rlint    new;
               Rlint    write;
               Rlint    avail;
               } typLRUcount, *refLRUcount;

typedef struct {
               Rint         psize;
               refLRUe      *hash;
               Rpnint       qty;
               Rpnint       max;
               refLRUe      head;
               refLRUe      tail;
               typLRUcount  count;
               Rint         error;
               } lrubuf;

typedef lrubuf *LRUBUF;

typedef struct {
               refLRUe pre;
               refLRUe curr;
               } typadrRec;


/* declarations */

static refLRUe NewEntry(LRUBUF LRU,
                        Rpnint index,
                        FPst filePage); /* concerns []-lists */

static void DisposeEntry(LRUBUF LRU,
                         Rpnint index,
                         FPst filePage); /* concerns []-lists */

static refLRUe Curr(refLRUe list,
                    FPst filePage); /* auxiliary concerning []-lists */
/* Called with hash[index], returns the reference to the entry, where
   filePage is stored or NULL. */

static typadrRec PreCurr(refLRUe list,
                         FPst filePage); /* auxiliary concerning []-lists */
/* Called with hash[index], returns a struct, containing the reference to
   the entry, where filePage is stored and the reference to its predecessor.
   Both may be NULL.*/

static void AddHeadEntry(LRUBUF LRU,
                         refLRUe curr); /* concerns head-tail-list */

static boolean CutTailEntry(LRUBUF LRU,
                            refLRUe *tail); /* concerns head-tail-list */

static void ReleaseEntry(LRUBUF LRU,
                         refLRUe curr); /* concerns head-tail-list */
/* Releases curr from the head-tail-list. Does not affect the []-lists.
   The entry thereafter may be e.g. added as head of the head-tail-list
   or disposed ([]-lists). */

static void LRUInitCount(LRUBUF LRU);

static void AllocEntry(LRUBUF LRU,
                       refLRUe *e);

static void DeAllocEntry(LRUBUF LRU,
                         refLRUe *e);

static void setLRUerr(LRUBUF LRU,
                      char *preMessage,
                      typerrors error);

static void setLRUwarn(LRUBUF LRU,
                       char *preMessage,
                       typerrors error);


/*********************************************************************/

void NewLRU(t_LRU *lru,
            Rpnint cap,
            Rint pageSize,
            boolean *Done)

{
  LRUBUF LRU= NULL;
  Rpnint i;
  
  *lru= allocM(sizeof(lrubuf));
  if (*lru == NULL) {
    setLRUerr((LRUBUF)*lru,"LRUBuf.NewLRU",reAlloc);
    *Done= FALSE;
    return;
  }
  LRU= (LRUBUF)*lru;
  
  (*LRU).hash= allocM(cap * sizeof(refLRUe));
  if ((*LRU).hash == NULL) {
    setLRUerr(LRU,"LRUBuf.NewLRU",reAlloc);
    *Done= FALSE;
    freeM(lru);
    return;
  }
  for (i= 0; i < cap; i++) {
    (*LRU).hash[i]= NULL;
  }
  
  (*LRU).psize= pageSize;
  (*LRU).qty= 0;
  (*LRU).max= cap;
  (*LRU).head= NULL;
  (*LRU).tail= NULL;
  (*LRU).error= noError;
  
  LRUInitCount(LRU);
}

/*********************************************************************/

void DisposeLRU(t_LRU *lru)

{
  LRUBUF LRU= (LRUBUF)*lru;
  refLRUe pre, curr;
  Rpnint i;
  
  for (i= 0; i < (*LRU).max; i++) {
    curr= (*LRU).hash[i];
    while (curr != NULL) {
      pre= curr;
      curr= (*curr).other;
      freeM(&(*pre).pub.adr);
      freeM(&pre);
    }
  }
  freeM(&(*LRU).hash);
  freeM(lru);
}

/*********************************************************************/

void SaveLRU(t_LRU lru,
             boolean *Done)

{
  LRUBUF LRU= (LRUBUF)lru;
  
  refLRUe curr= (*LRU).head;
  while(curr != NULL) {
    if ((*curr).pub.write) {
      if (! WrPage((*curr).fp.f,(*LRU).psize,(*curr).fp.p,(*curr).pub.adr)) {
        *Done= FALSE;
        return;
      }
      (*curr).pub.write= FALSE; /* written */
#ifndef COUNTS_OFF
      if ((*LRU).count.on) {
        (*LRU).count.write++;
      }
#endif
    }
    curr= (*curr).suc;
  }
}

/*********************************************************************/

void LRUCloseFile(t_LRU lru,
                  File file,
                  boolean *Done) {

  LRUBUF LRU= (LRUBUF)lru;
  refLRUe curr, suc;
  
  curr= (*LRU).head;
  while(curr != NULL) {
    suc= (*curr).suc;
    if ((*curr).fp.f == file) {
      if ((*curr).pub.write) {
        if (! WrPage((*curr).fp.f,(*LRU).psize,(*curr).fp.p,(*curr).pub.adr)) {
          *Done= FALSE;
          return;
        }
#ifndef COUNTS_OFF
        if ((*LRU).count.on) {
          (*LRU).count.write++;
        }
#endif
      }
      ReleaseEntry(LRU,curr);
      DisposeEntry(LRU,(*curr).index,(*curr).fp);
    }
    curr= suc;
  }
}

/*********************************************************************/

void LRUSyncFile(t_LRU lru,
                 File file,
                 boolean *Done) {

  LRUBUF LRU= (LRUBUF)lru;
  refLRUe curr, suc;
  
  curr= (*LRU).head;
  while(curr != NULL) {
    suc= (*curr).suc;
    if ((*curr).fp.f == file) {
      if ((*curr).pub.write) {
        if (! WrPage((*curr).fp.f,(*LRU).psize,(*curr).fp.p,(*curr).pub.adr)) {
          *Done= FALSE;
          return;
        }
        (*curr).pub.write= FALSE; /* written */
#ifndef COUNTS_OFF
        if ((*LRU).count.on) {
          (*LRU).count.write++;
        }
#endif
      }
    }
    curr= suc;
  }
}

/*********************************************************************/

void LRUSuspendFile(t_LRU lru,
                    File file,
                    boolean *Done) {

  LRUBUF LRU= (LRUBUF)lru;
  refLRUe curr, suc;
  
  curr= (*LRU).head;
  while(curr != NULL) {
    suc= (*curr).suc;
    if ((*curr).fp.f == file) {
      if ((*curr).pub.write) {
        setLRUerr(LRU,"LRUBuf.LRUSuspendFile",unexpectedWriteFlag);
        *Done= FALSE;
        return;
      }
      ReleaseEntry(LRU,curr);
      DisposeEntry(LRU,(*curr).index,(*curr).fp);
    }
    curr= suc;
  }
}

/*********************************************************************/

void ModifLRUCap(t_LRU lru,
                 Rpnint cap,
                 boolean *Done)

{
  LRUBUF LRU= (LRUBUF)lru;
  refLRUe curr, other;
  refLRUe *newhash;
  Rpnint i, index, oldmax;
  
  if (cap < (*LRU).qty) {
    setLRUerr(LRU,"LRUBuf.ModifLRUCap",paramLss);
    *Done= FALSE;
    return;
  }
  
  newhash= allocM(cap * sizeof(refLRUe));
  if (newhash == NULL) {
    setLRUerr(LRU,"LRUBuf.ModifLRUCap",reAlloc);
    *Done= FALSE;
    return;
  }
  
  oldmax= (*LRU).max;
  
  (*LRU).max= cap;
  for (i= 0; i < (*LRU).max; i++) {
    newhash[i]= NULL;
  }
  
  for (i= 0; i < oldmax; i++) {
    curr= (*LRU).hash[i];
    while (curr != NULL) {
      other= (*curr).other;
      
      // index= ((FPun)(*curr).fp).i % (*LRU).max; // gcc
      index= (*((FPun *)&(*curr).fp)).i % (*LRU).max;
      /* insertion into newhash[]-lists: */
      (*curr).other= newhash[index];
      newhash[index]= curr;
      (*newhash[index]).index= index;
      
      curr= other;
    }
  }
  
  freeM(&(*LRU).hash);
  (*LRU).hash= newhash;
  (*LRU).error= noError;
}

/*********************************************************************/

boolean HashLRUEntry(t_LRU lru,
                     FPst filePage,
                     refLRUentry *refEntry,
                     boolean *Done)

{
  LRUBUF LRU= (LRUBUF)lru;
  Rpnint index;
  boolean avail;
  refLRUe curr, tail;
  
  (*LRU).error= noError;
  // index= ((FPun)filePage).i % (*LRU).max; // gcc
  index= (*((FPun *)&filePage)).i % (*LRU).max;
  curr= Curr((*LRU).hash[index],filePage);
  avail= curr != NULL;
  *refEntry= NULL;
  
  if (avail) {
    if (curr != (*LRU).head) {
      ReleaseEntry(LRU,curr);
      AddHeadEntry(LRU,curr);
    }
#ifndef COUNTS_OFF
    if ((*LRU).count.on) {
      (*LRU).count.avail++;
    }
#endif
    *refEntry= &(*curr).pub;
  }
  else {
    curr= NewEntry(LRU,index,filePage);
    if (curr == NULL) {
      *Done= FALSE;
      return avail;
    }
    /* curr in []-lists now */
    if (! RdPage(filePage.f,(*LRU).psize,filePage.p,(*curr).pub.adr)) {
      DisposeEntry(LRU,index,filePage);
      *Done= FALSE;
      return avail;
    }
#ifndef COUNTS_OFF
    if ((*LRU).count.on) {
      (*LRU).count.read++;
    }
#endif
    /* fetched entry establishment: */
    (*curr).pub.locked= 0;
    (*curr).pub.write= FALSE;
    AddHeadEntry(LRU,curr);
    /* curr in head-tail-list too now */
    *refEntry= &(*curr).pub;
    if ((*LRU).qty > (*LRU).max) {
      /* displacement: */
      if (! CutTailEntry(LRU,&tail)) {
        *Done= FALSE;
        return avail;
      }
      DisposeEntry(LRU,(*tail).index,(*tail).fp);
    }
  }
  return avail;
}

/*********************************************************************/

boolean NewLRUEntry(t_LRU lru,
                    FPst filePage,
                    refLRUentry *refEntry,
                    boolean *Done)

{
  LRUBUF LRU= (LRUBUF)lru;
  Rpnint index;
  boolean new;
  refLRUe curr, tail;
  
  (*LRU).error= noError;
  // index= ((FPun)filePage).i % (*LRU).max; // gcc
  index= (*((FPun *)&filePage)).i % (*LRU).max;
  curr= Curr((*LRU).hash[index],filePage);
  new= curr == NULL;
  *refEntry= NULL;
  
  if (! new) {
    if (curr != (*LRU).head) {
      ReleaseEntry(LRU,curr);
      AddHeadEntry(LRU,curr);
    }
#ifndef COUNTS_OFF
    if ((*LRU).count.on) {
      (*LRU).count.avail++;
    }
#endif
    *refEntry= &(*curr).pub;
  }
  else {
    curr= NewEntry(LRU,index,filePage);
    if (curr == NULL) {
      *Done= FALSE;
      return new;
    }
#ifndef COUNTS_OFF
    if ((*LRU).count.on) {
      (*LRU).count.new++;
    }
#endif
    /* curr in []-lists now */
    /* new entry establishment: */
    (*curr).pub.locked= 0;
    (*curr).pub.write= FALSE;
    AddHeadEntry(LRU,curr);
    /* curr in head-tail-list too now */
    *refEntry= &(*curr).pub;
    if ((*LRU).qty > (*LRU).max) {
      /* displacement: */
      if (! CutTailEntry(LRU,&tail)) {
        *Done= FALSE;
        return new;
      }
      DisposeEntry(LRU,(*tail).index,(*tail).fp);
    }
  }
  return new;
}

/*********************************************************************/

boolean DisposeLRUEntry(t_LRU lru,
                        FPst filePage,
                        boolean *Done)

{
  LRUBUF LRU= (LRUBUF)lru;
  Rpnint index;
  boolean avail;
  refLRUe curr;
  
  // index= ((FPun)filePage).i % (*LRU).max; // gcc
  index= (*((FPun *)&filePage)).i % (*LRU).max;
  curr= Curr((*LRU).hash[index],filePage);
  avail= curr != NULL;
  
  if (avail) {
    if ((*curr).pub.locked > 1) {
      setLRUerr(LRU,"LRUBuf.DisposeLRUEntry",multipleLocked);
      *Done= FALSE;
      return avail;
    }
    ReleaseEntry(LRU,curr);
    DisposeEntry(LRU,index,filePage);
  }
  return avail;
}

/*********************************************************************/

void *LRUPgRead(t_LRU lru,
                File file,
                Rpnint pageNr) {

  FPst fp;
  refLRUentry rLRUe;
  boolean done;
  
  fp.f= file;
  fp.p= pageNr;
  HashLRUEntry(lru,fp,&rLRUe,&done);
  if (rLRUe != NULL) {
    return (*rLRUe).adr;
  }
  return NULL;
}

/*********************************************************************/

void *LRUPgWrite(t_LRU lru,
                 File file,
                 Rpnint pageNr) {

  FPst fp;
  refLRUentry rLRUe;
  boolean done;
  
  fp.f= file;
  fp.p= pageNr;
  if (LenOfF(file) < (pageNr + 1) * LRUPageSize(lru)) {
    NewLRUEntry(lru,fp,&rLRUe,&done);
  }
  else {
    HashLRUEntry(lru,fp,&rLRUe,&done);
  }
  if (rLRUe != NULL) {
    (*rLRUe).write= TRUE;
    return (*rLRUe).adr;
  }
  return NULL;
}

/*********************************************************************/

Rint LRUPageSize(t_LRU lru)

{
  return (*(LRUBUF)lru).psize;
}

/*********************************************************************/

void GetLRUCapUse(t_LRU lru,
                  Rpnint *cap,
                  Rpnint *use)

{
  *cap= (*(LRUBUF)lru).max;
  *use= (*(LRUBUF)lru).qty;
}

/*********************************************************************/

void InquireLRUDesc(t_LRU lru,
                    Rint *pageSize,
                    Rpnint *cap,
                    Rpnint *entriesUsed)

{
  *pageSize= (*(LRUBUF)lru).psize;
  *cap= (*(LRUBUF)lru).max;
  *entriesUsed= (*(LRUBUF)lru).qty;
}

/*********************************************************************/

t_FPS LRUPages(t_LRU lru,
               boolean *Done)

{
  LRUBUF LRU= (LRUBUF)lru;
  Rpnint htqty;
  refLRUe curr;
  t_FPS htset= FPS_Empty();
  boolean subErrFree= TRUE;
  
  htqty= 0;
  curr= (*LRU).head;
  while (curr != NULL) {
    if (! FPS_Include(&htset,(*curr).fp,&subErrFree)) {
      if (subErrFree) {
        setLRUerr(LRU,"LRUBuf.LRUPages",redundant);
        *Done= FALSE;
      }
      else {
        *Done= FALSE;
        FPS_Delete(&htset);
        return htset;
      }
    }
    htqty++;
    curr= (*curr).suc;
  }
  if (htqty != (*LRU).qty) {
    setLRUerr(LRU,"LRUBuf.LRUPages",checkSum);
    *Done= FALSE;
  }
  return htset;
}

/*********************************************************************/

t_FPB LRUPagesLocked(t_LRU lru,
                     boolean *Done)

{
  LRUBUF LRU= (LRUBUF)lru;
  Rpnint htqty;
  refLRUe curr;
  t_FPB lockbag= FPB_Empty();
  boolean subErrFree= TRUE;
  
  htqty= 0;
  curr= (*LRU).head;
  while (curr != NULL) {
    if ((*curr).pub.locked != 0) {
      if (FPB_Include(&lockbag,(*curr).fp,(*curr).pub.locked,&subErrFree) != (*curr).pub.locked) {
        if (subErrFree) {
          setLRUerr(LRU,"LRUBuf.LRUPagesLocked",redundant);
          *Done= FALSE;
        }
        else {
          *Done= FALSE;
          FPB_Delete(&lockbag);
          return lockbag;
        }
      }
    }
    htqty++;
    curr= (*curr).suc;
  }
  if (htqty != (*LRU).qty) {
    setLRUerr(LRU,"LRUBuf.LRUPagesLocked",checkSum);
    *Done= FALSE;
  }
  return lockbag;
}

/*********************************************************************/

void PrintLRUPriority(t_LRU lru,
                      FILE *stream,
                      Rint epl) {

  LRUBUF LRU= (LRUBUF)lru;
  char str[80];
  refLRUe last;
  refLRUe curr= (*LRU).head;
  Rint cardcount= 0;
  
  last= curr;
  while (curr != NULL) {
    fprintf(stream,strans(" f%I.p%N",str),(*curr).fp.f,(*curr).fp.p);
    cardcount++;
    if (cardcount % epl == 0) {
      fprintf(stream,"\n");
    }
    last= curr;
    curr= (*curr).suc;
  }
  if (last != (*LRU).tail) {
    fprintf(stderr,"%s\n","LRUBuf: FATAL INTERNAL ERROR");
    fprintf(stderr,"%s\n","PrintLRUPriority 1: inconsistent priority tail.");
  }
  if (cardcount == 0 || cardcount % epl != 0) {
    fprintf(stream,"\n");
  }
}

/*********************************************************************/

boolean LRUConsistent(t_LRU lru,
                      boolean *Done)

{
  LRUBUF LRU= (LRUBUF)lru;
  Rpnint htqty, listsqty, i, numberr;
  FPst numb;
  refLRUe curr;
  t_FPS htset, listsset;
  boolean subErrFree= TRUE;
  
  (*LRU).error= noError;
  
  htset= NULL;
  htqty= 0;
  curr= (*LRU).head;
  while (curr != NULL) {
    if (! FPS_Include(&htset,(*curr).fp,&subErrFree)) {
      if (subErrFree) {
        setLRUerr(LRU,"LRUBuf.LRUConsistent",redundant);
        /* But the check itself could be done! *Done not set! */
      }
      else {
        *Done= FALSE;
      }
      FPS_Delete(&htset);
      return FALSE;  /* on failing to include, regardless of the reason! */
    }
    htqty++;
    curr= (*curr).suc;
  }
  
  listsset= NULL;
  listsqty= 0;
  for (i= 0; i < (*LRU).max; i++) {
    curr= (*LRU).hash[i];
    while (curr != NULL) {
      if (! FPS_Include(&listsset,(*curr).fp,&subErrFree)) {
        if (subErrFree) {
          setLRUerr(LRU,"LRUBuf.LRUConsistent",redundant);
          /* But the check itself could be done! *Done not set! */
        }
        else {
          *Done= FALSE;
        }
        FPS_Delete(&listsset);
        return FALSE;  /* on failing to include, regardless of the reason! */
      }
      listsqty++;
      curr= (*curr).other;
    }
  }
  
  numberr= 0;
  while (FPS_Pop(&htset,&numb)) {
    if (! FPS_Exclude(&listsset,numb)) {
      numberr++;
    }
  }
  if (numberr == 0) {
    numberr= FPS_Cardinality(listsset);
  }
  
  if (htqty != (*LRU).qty || listsqty != (*LRU).qty || numberr != 0) {
    setLRUerr(LRU,"LRUBuf.LRUConsistent",checkSum);
    FPS_Delete(&htset);
    FPS_Delete(&listsset);
    return FALSE;
  }
  
  return TRUE;
}

/*********************************************************************/

void LRUCountsOn0(t_LRU lru)

{
  LRUInitCount((LRUBUF)lru);
  (*(LRUBUF)lru).count.on= TRUE;
}

/************************************************************************/

void LRUCountsOn(t_LRU lru)

{
  (*(LRUBUF)lru).count.on= TRUE;
}

/************************************************************************/

void LRUCountsOff(t_LRU lru)

{
  (*(LRUBUF)lru).count.on= FALSE;
}

/************************************************************************/

void LRUGetCountRead(t_LRU lru,
                     Rlint *readCount)

{
  *readCount= (*(LRUBUF)lru).count.read;
}

/************************************************************************/

void LRUGetCountNew(t_LRU lru,
                    Rlint *newCount)

{
  *newCount= (*(LRUBUF)lru).count.new;
}

/************************************************************************/

void LRUGetCountWrite(t_LRU lru,
                      Rlint *writeCount)

{
  *writeCount= (*(LRUBUF)lru).count.write;
}

/***********************************************************************/

void LRUGetCountAvail(t_LRU lru,
                      Rlint *availCount)

{
  *availCount= (*(LRUBUF)lru).count.avail;
}

/************************************************************************/

int LRUError(t_LRU lru)

{
  if (lru != NULL) {
    return (*(LRUBUF)lru).error;
  }
  else {
    return -1;
  }
}

/************************************************************************/
/* static functions: */
/************************************************************************/

static refLRUe NewEntry(LRUBUF LRU,
                        Rpnint index,
                        FPst filePage)

{
  refLRUe curr;
  
  AllocEntry(LRU,&curr);
  if (curr == NULL) {
    return curr;
  }
  
  /* curr: basic initialization: */
  (*curr).index= index;
  (*curr).fp= filePage;
  /* curr: insertion into []-lists: */
  (*curr).other= (*LRU).hash[index];
  (*LRU).hash[index]= curr;
  /* curr: flags initialization: by callers NewLRUEntry, HashLRUEntry */
  return curr;
}

/*********************************************************************/
/* DisposeEntry is only called when the entry is known to be there */

static void DisposeEntry(LRUBUF LRU,
                         Rpnint index,
                         FPst filePage)

{
  typadrRec r= PreCurr((*LRU).hash[index],filePage);
  
  if (r.curr == NULL) {
    fprintf(stderr,"%s\n","LRUBuf: FATAL INTERNAL ERROR");
    fprintf(stderr,"%s\n","DisposeEntry 1: entry missing.");
  }
  if (r.pre == NULL) {
    (*LRU).hash[index]= (*r.curr).other;
  }
  else {
    (*r.pre).other= (*r.curr).other;
  }
  DeAllocEntry(LRU,&r.curr);
}

/*********************************************************************/

static refLRUe Curr(refLRUe list,
                    FPst filePage)

{
                      // ((FPun)filePage).i != ((FPun)(*list).fp).i // gcc
  while (list != NULL && (*((FPun *)&filePage)).i != (*((FPun *)&(*list).fp)).i) {
    list= (*list).other;
  }
  return list;
}

/*********************************************************************/

static typadrRec PreCurr(refLRUe list,
                         FPst filePage)

{
  typadrRec r;
  
  r.pre= NULL;
  r.curr= list;
                        // ((FPun)filePage).i != ((FPun)(*r.curr).fp).i) // gcc
  while (r.curr != NULL && (*((FPun *)&filePage)).i != (*((FPun *)&(*r.curr).fp)).i) {
    r.pre= r.curr;
    r.curr= (*r.curr).other;
  }
  return r;
}

/*********************************************************************/

static void AddHeadEntry(LRUBUF LRU,
                         refLRUe curr)

{
  if ((*LRU).head == NULL) {
    if ((*LRU).qty == 1) {
      (*LRU).head= curr;
      (*LRU).tail= curr;
      (*curr).pre= NULL;
      (*curr).suc= NULL;
    }
    else {
      fprintf(stderr,"%s\n","LRUBuf: FATAL INTERNAL ERROR");
      fprintf(stderr,"%s\n","AddHeadEntry 1: head missing.");
    }
  }
  else {
    (*curr).pre= NULL;
    (*(*LRU).head).pre= curr;
    (*curr).suc= (*LRU).head;
    (*LRU).head= curr;
  }
}

/*********************************************************************/
/* CutTailEntry is only called immediately after AddHeadEntry */

static boolean CutTailEntry(LRUBUF LRU,
                            refLRUe *tail)

{
  *tail= (*LRU).tail;
  
  while (*tail != NULL && (**tail).pub.locked > 0) {
    *tail= (**tail).pre;
  }
  if (*tail == NULL || *tail == (*LRU).head) {
    /* all entries locked || the sole lock-free is the one just fetched */
    setLRUwarn(LRU,"LRUBuf.CutTailEntry",displacement);
    return FALSE;
  }
  if ((**tail).pub.write) {
    if (! WrPage((**tail).fp.f,(*LRU).psize,(**tail).fp.p,(**tail).pub.adr)) {
      return FALSE;
    }
#ifndef COUNTS_OFF
    if ((*LRU).count.on) {
      (*LRU).count.write++;
    }
#endif
  }
  ReleaseEntry(LRU,*tail);
  return TRUE;
}

/*********************************************************************/

static void ReleaseEntry(LRUBUF LRU,
                         refLRUe curr)

{
  if (curr == (*LRU).head) {
    (*LRU).head= (*curr).suc;
  }
  else {
    (*(*curr).pre).suc= (*curr).suc;
  }
  if (curr == (*LRU).tail) {
    (*LRU).tail= (*curr).pre;
  }
  else {
    (*(*curr).suc).pre= (*curr).pre;
  }
}

/*********************************************************************/

static void LRUInitCount(LRUBUF LRU)

{
  (*LRU).count.on= FALSE;
  
  (*LRU).count.read= 0;
  (*LRU).count.new= 0;
  (*LRU).count.write= 0;
  (*LRU).count.avail= 0;
}

/*********************************************************************/
   
static void AllocEntry(LRUBUF LRU,
                       refLRUe *e)

{
  *e= allocM(sizeof(typLRUe));
  if (*e == NULL) {
    setLRUerr(LRU,"LRUBuf.AllocEntry",reAlloc);
    return;
  }
  (**e).pub.adr= allocM((*LRU).psize);
  if ((**e).pub.adr == NULL) {
    setLRUerr(LRU,"LRUBuf.AllocEntry",reAlloc);
    freeM(e);
    return;
  }
  (*LRU).qty++;
}

/*********************************************************************/

static void DeAllocEntry(LRUBUF LRU,
                         refLRUe *e)

{
  (*LRU).qty--;
  freeM(&(**e).pub.adr);
  freeM(e);
}

/*********************************************************************/

static void setLRUerr(LRUBUF LRU,
                      char *preMessage,
                      typerrors error)

{
  if (LRU != NULL) {
    (*LRU).error= error;
  }
  pRSTerr(preMessage,error);
}

/***********************************************************************/

static void setLRUwarn(LRUBUF LRU,
                       char *preMessage,
                       typerrors error)

{
  if (LRU != NULL) {
    (*LRU).error= error;
  }
  pRSTwarn(preMessage,error);
}

/***********************************************************************/
