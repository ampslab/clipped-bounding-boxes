/* ----- RSTree.c ----- */
#//
#// Copyright (c) 1994 - 2015 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//


#include "RSTOtherFuncs.h"
#include "RSTDistQueryFuncs.h"
#include "RSTDistQueryBase.h"
#include "RSTInterUtil.h"
#include "RSTPageInOut.h"
#include "RSTInstDel.h"
#include "RSTQuery.h"
#include "RSTJoin.h"
#include "RSTDistHeap.h"
#include "RSTUtil.h"
#include "RSTFileAccess.h"
#include "RSTMemAlloc.h"
#include "RSTLRUBuf.h"
#include "RSTSupport.h"

/* constants */


/* types */


/* declarations */

static void ASCdmp(RSTREE R,
                   Rint level,
                   FILE *stream);
static void PrintEntry(RSTREE R,
                       void *entryStart,
                       Rint level,
                       FILE *stream);
static void ChckCnst(RSTREE R,
                     Rint level,
                     boolean *consistency,
                     Rint *upperLevel,
                     typinterval *upperRect,
                     Rpnint *upperPage,
                     Rint *lowerLevel,
                     typinterval *lowerRect,
                     Rpnint *lowerPage,
                     Rpnint countNodes[]);
static void MakeDataEntry(RSTREE R,
                          const typinterval *rect,
                          refinfo info,
                          void *entry);

/************************************************************************/

void InitRSTreeIdent(t_RT *r)

{
  *r= NULL;
}

/************************************************************************/

boolean CreateRST(const char *name,
                  Rint dirpagesize,
                  Rint datapagesize,
                  Rint numbOfDim,
                  Rint infoSize,
                  boolean unique)

{
  RSTREE R= NULL;
  refparameters par;
  
  BasicCheck();
  
  R= allocM(sizeof(rstree));
  if (R == NULL) {
    setRSTerr(R,"CreateRST",reAlloc);
    return FALSE;
  }
  if (strlcpy((*R).mainname,name,sizeof(RSTName)) >= sizeof(RSTName) - SUFF_LEN) {
    setRSTerr(R,"CreateRST",nameLenGtr);
    freeM(&R);
    return FALSE;
  }
  ResetStorageInfo(R,sec);
  (*R).RSTDone= TRUE;
  
  par= &(*R).parameters._;
  
  SetOffsets(R,numbOfDim,infoSize);
  SetBase(R,dirpagesize,datapagesize,unique);
  if (! (*R).RSTDone) {
    freeM(&R);
    return FALSE;
  }
  SetCheck(R,TRUE);
  SetVersion(R);
  /* no SetSizesPreAllocArrs(R) here, because tree not opened */
  CreateRSFiles(R);
  if (! (*R).RSTDone) {
    freeM(&R);
    return FALSE;
  }
  
  SetFixParamIOblocks(R);
  WriteParamsPDs(R);
  
  AllocBuffers(R);
  (*(*R).L[(*par).rootlvl].N).s.nofentries= 0;
  /* (*(*R).L[(*par).rootlvl].N).s.splitnoe= 0; */     /* (1) */
  SetVarDirDataIOblocks(R);
  PutNode(R,(*R).L[(*par).rootlvl].N,ROOT_BL_NR,(*par).rootlvl);
  /* the root alternatively resides in DIR.str.f or DATA.str.f */
  
  DeallocBuffers(R);
  if (! (*R).RSTDone) {
    freeM(&R);
    return FALSE;
  }
  CloseRSFiles(R);
  if (! (*R).RSTDone) {
    freeM(&R);
    return FALSE;
  }
  freeM(&R);
  return TRUE;
}

/*** ------------------------ VERSIONCONTROL ------------------------ ***/
/* Statement (1) initializes a node's splitmid.                        */
/* Simple versions do not support (waste the space for) a splitmid.    */
/************************************************************************/

boolean RemoveRST(const char *name)

{
  RSTName SufName;
  boolean success;

  if (strlen(name) >= sizeof(SufName) - SUFF_LEN) {
    setRSTerr(NULL,"RemoveRST",nameLenGtr);
    return FALSE;
  }
  
  success= TRUE;
  if (! UnlinkF(name)) {
    success= FALSE;
  }
  strlcpy(SufName,name,sizeof(SufName));
  strlcat(SufName,DATA_SUFF,sizeof(SufName));
  if (! UnlinkF(SufName)) {
    success= FALSE;
  }
  strlcpy(SufName,name,sizeof(SufName));
  strlcat(SufName,DIR_PD_SUFF,sizeof(SufName));
  if (! UnlinkF(SufName)) {
    success= FALSE;
  }
  strlcpy(SufName,name,sizeof(SufName));
  strlcat(SufName,DATA_PD_SUFF,sizeof(SufName));
  if (! UnlinkF(SufName)) {
    success= FALSE;
  }
  strlcpy(SufName,name,sizeof(SufName));
  strlcat(SufName,DESC_SUFF,sizeof(SufName));
  if (! UnlinkF(SufName)) {
    success= FALSE;
  }
  return success;
}

/************************************************************************/

boolean TryRemoveRST(const char *name)

{
  RSTName SufName;
  boolean success;

  if (strlen(name) >= sizeof(SufName) - SUFF_LEN) {
    setRSTerr(NULL,"TryRemoveRST",nameLenGtr);
    return FALSE;
  }
  
  success= TRUE;
  if (! SilUnlinkF(name)) {
    success= FALSE;
  }
  strlcpy(SufName,name,sizeof(SufName));
  strlcat(SufName,DATA_SUFF,sizeof(SufName));
  if (! SilUnlinkF(SufName)) {
    success= FALSE;
  }
  strlcpy(SufName,name,sizeof(SufName));
  strlcat(SufName,DIR_PD_SUFF,sizeof(SufName));
  if (! SilUnlinkF(SufName)) {
    success= FALSE;
  }
  strlcpy(SufName,name,sizeof(SufName));
  strlcat(SufName,DATA_PD_SUFF,sizeof(SufName));
  if (! SilUnlinkF(SufName)) {
    success= FALSE;
  }
  strlcpy(SufName,name,sizeof(SufName));
  strlcat(SufName,DESC_SUFF,sizeof(SufName));
  if (! SilUnlinkF(SufName)) {
    success= FALSE;
  }
  return success;
}

/************************************************************************/

boolean OpenRST(t_RT *r,
                const char *name)

{
  RSTREE R= NULL;
  boolean success;

  success= (InternalOpen((RSTREE*)r,name,TRUE));
  if (success) {
    R= (RSTREE)*r;
    
    ResetStorageInfo(R,sec);
    InitRootAndCounting(R,TRUE);
    success= (*R).RSTDone;
  }
  return success;
}

/************************************************************************/

boolean OpenBufferedRST(t_RT *r,
                        const char *name,
                        t_LRU LRU)

{
  RSTREE R= NULL;
  boolean success;
  refparameters par;
  Rint LRUpagesize;
  Rpnint cap, use;

  success= (InternalOpen((RSTREE*)r,name,FALSE));
  if (success) {
    R= (RSTREE)*r;
    
    par= &(*R).parameters._;
    
    InquireLRUDesc(LRU,&LRUpagesize,&cap,&use);
    if ((*par).DIR.pagelen != LRUpagesize) {
      setRSTerr(R,"OpenBufferedRST",dirPageBufMismatch);
      FastCloseRSFiles(R);
      freeM(r);
      return FALSE;
    }
    if ((*par).DATA.pagelen != LRUpagesize) {
      setRSTerr(R,"OpenBufferedRST",dataPageBufMismatch);
      FastCloseRSFiles(R);
      freeM(r);
      return FALSE;
    }
    if (cap < (*par).rootlvl + 1) {
      setRSTerr(R,"OpenBufferedRST",nBufPagesLss);
      FastCloseRSFiles(R);
      freeM(r);
      return FALSE;
    }
    
    ResetStorageInfo(R,lru);
    (*R).LRU= LRU;
    InitRootAndCounting(R,TRUE);
    success= (*R).RSTDone;
  }
  return success;
}

/************************************************************************/

boolean CloseRST(t_RT *r)

{
  RSTREE R= NULL;
  boolean Done;
  
  if (*r == NULL) {
    setRSTerr(R,"CloseRST",RNULL);
    return FALSE;
  }
  R= (RSTREE)*r;
  
  if (! (*R).secbound) {
    setRSTerr(R,"CloseRST",mainMemTree);
    return FALSE;
  }
  (*R).RSTDone= TRUE;
  
  WriteParamsPDs(R);
  
  if ((*R).storkind == lru) {
    /*** Begin internal error checks: ***/
    if (! PathLRUConsistence(R)) {
      fprintf(stderr,"Continuing CloseRST!\n");
    }
    /* General LRU buffer consistency check: May be expensive!
       To be applied after code modifications only! */
    //Done= TRUE;
    //if (! LRUConsistent((*R).LRU,&Done)) {
    //  if (! Done) {
    //    setRSTerr(R,"CloseRST",LRUconsistenceCheck);
    //  }
    //  else {
    //    fprintf(stderr,"Continuing CloseRST!\n");
    //  }
    //}
    /*** End   internal error checks ***/
  }
  
  PutPath(R); /* lru: clear locks */
  Done= TRUE;
  
  if ((*R).storkind == lru) {
    Done= CloseLRUBuffering(R);
  }
  if (Done) {
    CleanupCloseRST((RSTREE*)r);
  }
  return Done;
}

/************************************************************************/

boolean SyncRST(t_RT r) {

  boolean LRUDone= TRUE; // never set to TRUE by LRU implementation!
  RSTREE R= NULL;
  
  if (r == NULL) {
    setRSTerr(R,"SyncRST",RNULL);
    return FALSE;
  }
  R= (RSTREE)r;
  
  if (! (*R).secbound) {
    setRSTerr(R,"SyncRST",mainMemTree);
    return FALSE;
  }
  
  if ((*R).RSTDone) {
    WriteParamsPDs(R);
    SyncPath(R); /* lru: keep locks */
    if ((*R).storkind == lru) {
      /*** Begin internal error checks: ***/
      if (! PathLRUConsistence(R)) {
        return FALSE;
      }
      /*** End   internal error checks ***/
      LRUSyncFile((*R).LRU,(*R).DIR.str.f,&LRUDone);
      LRUSyncFile((*R).LRU,(*R).DATA.str.f,&LRUDone);
    }
  }
  return (*R).RSTDone && LRUDone;
}

/************************************************************************/

boolean GetRSTSync(t_RT r) {

  Rint OLDrootlvl;
  boolean LRUDone= TRUE; // never set to TRUE by LRU implementation!
  RSTREE R= NULL;
  
  if (r == NULL) {
    setRSTerr(R,"GetRSTSync",RNULL);
    return FALSE;
  }
  R= (RSTREE)r;
  
  if (! (*R).secbound) {
    setRSTerr(R,"GetRSTSync",mainMemTree);
    return FALSE;
  }
  
  if ((*R).RSTDone) {
    OLDrootlvl= (*R).parameters._.rootlvl;
    ReadParamsPDs(R);
    InitFlags(R);
    if ((*R).storkind == sec) {
      AdaptPathAlloc(R,OLDrootlvl);
    }
    else if ((*R).storkind == lru) {
      LRUSuspendFile((*R).LRU,(*R).DIR.str.f,&LRUDone);
      LRUSuspendFile((*R).LRU,(*R).DATA.str.f,&LRUDone);
    }
    if ((*R).RSTDone && LRUDone) {
      InitRootAndCounting(R,TRUE);
    }
  }
  return (*R).RSTDone && LRUDone;
}

/************************************************************************/

boolean CreateMainMemRST(t_RT *r,
                         Rint dirpagesize,
                         Rint datapagesize,
                         Rint numbOfDim,
                         Rint infoSize,
                         boolean unique,
                         Rpint dirRAMsize,
                         Rpint dataRAMsize,
                         boolean verboseRAMdiskExt)

{
  RSTREE R= NULL;
  refparameters par;
  
  BasicCheck();
  
  if (*r != NULL) {
    setRSTerr(R,"CreateMainMemRST",RnotNULL);
    return FALSE;
  }
  if (dirRAMsize < 3 * dirpagesize) {
    setRSTerr(R,"CreateMainMemRST",dirRAMsizeLss);
    return FALSE;
  }
  if (dataRAMsize < 3 * datapagesize) {
    setRSTerr(R,"CreateMainMemRST",dataRAMsizeLss);
    return FALSE;
  }
  *r= allocM(sizeof(rstree));
  R= (RSTREE)*r;
  if (R == NULL) {
    setRSTerr(R,"CreateMainMemRST",reAlloc);
    return FALSE;
  }
  
  strlcpy((*R).mainname,MAIN_MEMORY,sizeof(RSTName));
  ResetStorageInfo(R,pri);
  (*R).RSTDone= TRUE;
  
  par= &(*R).parameters._;
  
  SetOffsets(R,numbOfDim,infoSize);
  SetBase(R,dirpagesize,datapagesize,unique);
  if (! (*R).RSTDone) {
    freeM(r);
    return FALSE;
  }
  SetCheck(R,TRUE);
  SetVersion(R);
  SetSizesPreAllocArrs(R);
  if (! (*R).RSTDone) {
    freeM(r);
    return FALSE;
  }
  InitBuffersFlags(R);
  InitRamDisc(R);
  ReAllocRamDisc(R,dirRAMsize,dataRAMsize);
  if (! (*R).RSTDone) {
    DeallocArrs(R);
    DeallocRamDisc(R);
    freeM(r);
    return FALSE;
  }
  SetRAMdiscLimits(R);
  (*R).verboseRAMdiskExt= verboseRAMdiskExt;
  
  SetFixParamIOblocks(R);
  SetVarDirDataIOblocks(R);
  InitRootAndCounting(R,FALSE);
  (*(*R).L[(*par).rootlvl].N).s.nofentries= 0;
  /* (*(*R).L[(*par).rootlvl].N).s.splitnoe= 0; */     /* (1) */
  if (! (*R).RSTDone) {
    DeallocArrs(R);
    DeallocRamDisc(R);
    freeM(r);
    return FALSE;
  }
  return (*R).RSTDone;
}

/*** ------------------------ VERSIONCONTROL ------------------------ ***/
/* Statement (1) initializes a node's splitmid.                        */
/* Simple versions do not support (waste the space for) a splitmid.    */
/************************************************************************/

boolean RemoveMainMemRST(t_RT *r)

{
  RSTREE R= NULL;
  
  if (*r == NULL) {
    setRSTerr(R,"RemoveMainMemRST",RNULL);
    return FALSE;
  }
  R= (RSTREE)*r;
  
  if ((*R).storkind != pri) {
    setRSTerr(R,"RemoveMainMemRST",secMemTree);
    return FALSE;
  }
  DeallocArrs(R);
  DeallocRamDisc(R);
  freeM(r);
  return TRUE;
}

/************************************************************************/

boolean LoadRST(t_RT *r,
                const char *name,
                Rpint dirRAMsize,
                Rpint dataRAMsize)

{
  RSTREE R= NULL;
  refparameters par;
  boolean success;
  void *adr;
  Rpnint extrablocks;
  Rpint buflen, offset;
  Rpint minDirRAMsize, minDataRAMsize;
  
  success= InternalOpen((RSTREE*)r,name,FALSE);
  /* initializes anything except the RAM disc, the root remains to be read */
  if (! success) {
    return FALSE;
  }
  R= (RSTREE)*r;
  
  strlcpy((*R).mainname,MAIN_MEMORY,sizeof(RSTName));
  ResetStorageInfo(R,pri);
  (*R).RSTDone= TRUE;

  par = &(*R).parameters._;
  
  minDirRAMsize= ((*R).DIR.PA.pagedir._.number[0] + 1) * (*par).DIR.pagelen;
  if (dirRAMsize == 0) {
    dirRAMsize= minDirRAMsize;
  }
  else if (dirRAMsize < minDirRAMsize) {
    setRSTerr(R,"LoadRST",dirRAMsizeLss);
    (*R).RSTDone= FALSE;
  }
  minDataRAMsize= ((*R).DATA.PA.pagedir._.number[0] + 1) * (*par).DATA.pagelen;
  if (dataRAMsize == 0) {
    dataRAMsize= minDataRAMsize;
  }
  else if (dataRAMsize < minDataRAMsize) {
    setRSTerr(R,"LoadRST",dataRAMsizeLss);
    (*R).RSTDone= FALSE;
  }
  if (! (*R).RSTDone) {
    FastCloseRSFiles(R);
    freeM(r);
    return FALSE;
  }
  
  InitRamDisc(R);
  ReAllocRamDisc(R,dirRAMsize,dataRAMsize);
  if (! (*R).RSTDone) {
    DeallocRamDisc(R);
    FastCloseRSFiles(R);
    freeM(r);
    return FALSE;
  }
  SetRAMdiscLimits(R);
  
  /* --- read page directories' extra blocks to ramdisc: --- */
  /* Done by InternalOpen:
     DIR.PA.str.f, DATA.PA.str.f newly opened, but first pages already read
     by ReadParamsPDs in pagewise mode. Thus file pointers NOT usefully set
     for the following continuative read. */
  extrablocks= (*R).DIR.PA.pagedir._.childnr - FIRST_PD_BL_NR;
  if (extrablocks != 0) {
    offset= (FIRST_PD_BL_NR + 1) * (*R).DIR.PA.str.psize;
    adr= (void *)((Rpint)(*R).DIR.PA.str.RAM.ptr + offset);
    buflen= extrablocks * (*R).DIR.PA.str.psize;
    if (SetPosF((*R).DIR.PA.str.f,offset)) {
      if (! RdBytes((*R).DIR.PA.str.f,adr,buflen)) {
        (*R).RSTDone= FALSE;
      }
    }
    else {
      (*R).RSTDone= FALSE;
    }
  }
  extrablocks= (*R).DATA.PA.pagedir._.childnr - FIRST_PD_BL_NR;
  if (extrablocks != 0) {
    offset= (FIRST_PD_BL_NR + 1) * (*R).DATA.PA.str.psize;
    adr= (void *)((Rpint)(*R).DATA.PA.str.RAM.ptr + offset);
    buflen= extrablocks * (*R).DATA.PA.str.psize;
    if (SetPosF((*R).DATA.PA.str.f,offset)) {
      if (! RdBytes((*R).DATA.PA.str.f,adr,buflen)) {
        (*R).RSTDone= FALSE;
      }
    }
    else {
      (*R).RSTDone= FALSE;
    }
  }
  
  /* --- read dir and data to ramdisc: --- */
  /* DIR.str.f, DATA.str.f newly opened and not yet touched, thus file
     pointers set to 0 */
  if ((*par).rootlvl > 0) {
    /* DIR.str.f not empty */
    buflen= ((*R).DIR.PA.pagedir._.number[0] + 1) * (*R).DIR.str.psize;
    if (! RdBytes((*R).DIR.str.f,(*R).DIR.str.RAM.ptr,buflen)) {
      (*R).RSTDone= FALSE;
    }
  }
  buflen= ((*R).DATA.PA.pagedir._.number[0] + 1) * (*R).DATA.str.psize;
  if (! RdBytes((*R).DATA.str.f,(*R).DATA.str.RAM.ptr,buflen)) {
    (*R).RSTDone= FALSE;
  }
  
  if (! (*R).RSTDone) {
    FastCloseRSFiles(R);
    DeallocRamDisc(R);
    freeM(r);
    return FALSE;
  }
  
  CloseRSFiles(R);
  if (! (*R).RSTDone) {
    FastCloseRSFiles(R);
    DeallocRamDisc(R);
    freeM(r);
    return FALSE;
  }
  InitRootAndCounting(R,TRUE);
  return (*R).RSTDone;
}

/************************************************************************/

boolean SaveRST(t_RT r,
                const char *name)

{
  RSTREE R= (RSTREE)r;
  refparameters par;
  void *adr;
  Rpnint extrablocks;
  Rpint buflen, offset;
  
  if (R == NULL) {
    setRSTerr(R,"SaveRST",RNULL);
    return FALSE;
  }
  if ((*R).storkind != pri) {
    setRSTerr(R,"SaveRST",secMemTree);
    return FALSE;
  }
  /* set sec. mem. name: */
  if (strlcpy((*R).mainname,name,sizeof(RSTName)) >= sizeof(RSTName) - SUFF_LEN) {
    setRSTerr(R,"SaveRST",nameLenGtr);
    return FALSE;
  }
  (*R).RSTDone= TRUE;
  
  par = &(*R).parameters._;
  
  CreateRSFiles(R);
  /* reset name: */
  strlcpy((*R).mainname,MAIN_MEMORY,sizeof(RSTName));
  if (! (*R).RSTDone) {
    return FALSE;
  }
  
  WriteParamsPDs(R);
  if (! (*R).RSTDone) {
    FastCloseRSFiles(R);
    return FALSE;
  }
  /* --- write ramdisc residing page directories' extra blocks to secondary
         memory --- */
  /* DIR.PA.str.f, DATA.PA.str.f newly created, but first pages already written
     by WriteParamsPDs in pagewise mode. Thus file pointers NOT usefully set
     for the following continuative write. */
  extrablocks= (*R).DIR.PA.pagedir._.childnr - FIRST_PD_BL_NR;
  if (extrablocks != 0) {
    offset= (FIRST_PD_BL_NR + 1) * (*R).DIR.PA.str.psize;
    adr= (void *)((Rpint)(*R).DIR.PA.str.RAM.ptr + offset);
    buflen= extrablocks * (*R).DIR.PA.str.psize;
    if (SetPosF((*R).DIR.PA.str.f,offset)) {
      if (! WrBytes((*R).DIR.PA.str.f,adr,buflen)) {
        (*R).RSTDone= FALSE;
      }
    }
    else {
      (*R).RSTDone= FALSE;
    }
  }
  extrablocks= (*R).DATA.PA.pagedir._.childnr - FIRST_PD_BL_NR;
  if (extrablocks != 0) {
    offset= (FIRST_PD_BL_NR + 1) * (*R).DATA.PA.str.psize;
    adr= (void *)((Rpint)(*R).DATA.PA.str.RAM.ptr + offset);
    buflen= extrablocks * (*R).DATA.PA.str.psize;
    if (SetPosF((*R).DATA.PA.str.f,offset)) {
      if (! WrBytes((*R).DATA.PA.str.f,adr,buflen)) {
        (*R).RSTDone= FALSE;
      }
    }
    else {
      (*R).RSTDone= FALSE;
    }
  }
  
  /* --- write path to RAMdisc: --- */
  /* not necessary since path consists of links to the RAMdisc */
  
  /* --- write ramdisc residing dir and data to secondary memory --- */
  /* DIR.str.f, DATA.str.f newly created and not yet touched, thus file
     pointers set to 0 */
  if ((*par).rootlvl > 0) {
    /* DIR.str.RAM not empty */
    buflen= ((*R).DIR.PA.pagedir._.number[0] + 1) * (*R).DIR.str.psize;
    if (! WrBytes((*R).DIR.str.f,(*R).DIR.str.RAM.ptr,buflen)) {
      (*R).RSTDone= FALSE;
    }
  }
  buflen= ((*R).DATA.PA.pagedir._.number[0] + 1) * (*R).DATA.str.psize;
  if (! WrBytes((*R).DATA.str.f,(*R).DATA.str.RAM.ptr,buflen)) {
    (*R).RSTDone= FALSE;
  }
  
  if (! (*R).RSTDone) {
    FastCloseRSFiles(R);
    return FALSE;
  }
  
  CloseRSFiles(R);
  if (! (*R).RSTDone) {
    return FALSE;
  }
  return (*R).RSTDone;
}

/************************************************************************/

boolean SetUnique(t_RT r,
                  boolean mode)

{
  RSTREE R= (RSTREE)r;
  
  if (R == NULL) {
    setRSTerr(R,"SetUnique",RNULL);
    return FALSE;
  }
  (*R).parameters._.unique= mode;
  return TRUE;
}

/************************************************************************/

boolean InsertRecord(t_RT r,
                     const typinterval *rectangle,
                     refinfo info,
                     boolean *inserted,
                     refinfo infoStored)

{
  RSTREE R= (RSTREE)r;
  refparameters par;
  Rint lv;
  // void *entry;
  
  if (R == NULL) {
    *inserted= FALSE;
    setRSTerr(R,"InsertRecord",RNULL);
    return FALSE;
  }
  
  par= &(*R).parameters._;
  
  if ((*par).DATA.pagecount >= MAX_DATA_PAGES) {
    setRSTerr(R,"InsertRecord",dataNumbPagesExh);
    *inserted= FALSE;
    return FALSE;
  }
  if ((*R).storkind == pri) {
    HandleRamDiscLock(R);
    if (! (*R).RSTDone) {
      *inserted= FALSE;
      return FALSE;
    }
  }
  
  (*R).RSTDone= TRUE;
  
  if ((*par).unique) {
    *inserted= ! RectXsts(R,(*par).rootlvl,rectangle,infoStored);
    if (! (*R).RSTDone) {
      return FALSE;
    }
  }
  else {
    *inserted= TRUE;
  }
  if (*inserted) {
    MakeDataEntry(R,rectangle,info,(*R).IR_entry);
  
    /* for (lv= 1; lv < (*par).rootlvl; lv++) {
      (*R).L[lv].ReInsert= FALSE;
    } */                                        /* (1) */
    
    if ((*par).DATA.M == 1) {
      (*R).L[1].ReInsert= TRUE;                 /* (2.1) */
      Insert(R,(*R).IR_entry,0);
      (*R).L[1].ReInsert= FALSE;                /* (2.2) */
    }
    else {
      (*R).L[0].ReInsert= TRUE;                 /* (2.1) */
      Insert(R,(*R).IR_entry,0);
      (*R).L[0].ReInsert= FALSE;                /* (2.2) */
    }
    (*par).recordcount++;
  }
  return (*R).RSTDone;
}

/*** ------------------------ VERSIONCONTROL ------------------------ ***/
/* Statement (1) must be applied if a non self reinitializing version   */
/* of reinsertion(insertion) is in use (see RSTInstDel).                */
/* Statement (2.1) is a general switch for reinsertion(insertion) and   */
/* must be set FALSE by (2.2) after use.                                */
/* NOTE:                                                                */
/* Reinsertion(insertion) is SUPPORTED here, BUT is ONLY ACTIVE in      */
/* in conjunction WITH an RSTInstDelR..-file!!                          */
/************************************************************************/

boolean DeleteRecord(t_RT r,
                     const typinterval *rectangle,
                     const typinfo *info,
                     InfoCmpFunc Qualified,
                     void *delRefAny,
                     boolean *deleted)

{
  RSTREE R= (RSTREE)r;
  refparameters par;
  Rint lv;
    
  if (R == NULL) {
    *deleted= FALSE;
    setRSTerr(R,"DeleteRecord",RNULL);
    return FALSE;
  }
  if ((*R).storkind == pri) {
    HandleRamDiscLock(R);
    if (! (*R).RSTDone) {
      *deleted= FALSE;
      return FALSE;
    }
  }
  
  (*R).RSTDone= TRUE;
  
  par= &(*R).parameters._;
  
  *deleted= RecordXsts(R,(*par).rootlvl,rectangle,info,Qualified,delRefAny);
  if (! (*R).RSTDone) {
    return FALSE;
  }
  if (*deleted) {
    
    /* for (lv= 0; lv < (*par).rootlvl; lv++) {
      (*R).L[lv].ReInsert= FALSE;
    } */                                        /* (1) */
    
    DeleteOneRec(R,rectangle);
    (*par).recordcount--;
  }
  return (*R).RSTDone;
}

/*** ------------------------ VERSIONCONTROL ------------------------ ***/
/* Statement (1) must be applied if a non self reinitializing version   */
/* of reinsertion(insertion) is in use (see RSTInstDel).                */
/* NOTE:                                                                */
/* Reinsertion(insertion) is SUPPORTED here, BUT is ONLY ACTIVE in      */
/* in conjunction WITH an RSTInstDelR..-file!!                          */
/************************************************************************/

boolean ReorganizeMedium(t_RT r,
                         boolean shortenanyway,
                         boolean *reorganized,
                         boolean *shortened)

{
  RSTREE R= (RSTREE)r;
  
  if (R == NULL) {
    setRSTerr(R,"ReorganizeMedium",RNULL);
    return FALSE;
  }
  (*R).RSTDone= TRUE;
  ReorgMedia(R,shortenanyway,reorganized,shortened);
  return (*R).RSTDone;
}

/************************************************************************/

boolean ExistsRegion(t_RT r,
                     const typinterval *qRects,
                     Rint qRectQty,
                     void *qPtr,
                     QueryFunc DirQuery,
                     QueryFunc DataQuery,
                     boolean *regionfound)

{  
  RSTREE R= (RSTREE)r;
  
  if (R == NULL) {
    *regionfound= FALSE;
    setRSTerr(R,"ExistsRegion",RNULL);
    return FALSE;
  }
  (*R).RSTDone= TRUE;
  *regionfound= FALSE;
  XstsRgn(R,(*R).parameters._.rootlvl,qRects,qRectQty,qPtr,DirQuery,DataQuery,regionfound);
  return (*R).RSTDone;
}

/************************************************************************/

boolean RegionCount(t_RT r,
                    const typinterval *qRects,
                    Rint qRectQty,
                    void *qPtr,
                    QueryFunc DirQuery,
                    QueryFunc DataQuery,
                    Rlint *recordcount)

{  
  RSTREE R= (RSTREE)r;
  
  if (R == NULL) {
    *recordcount= 0;
    setRSTerr(R,"RegionCount",RNULL);
    return FALSE;
  }
  (*R).RSTDone= TRUE;
  *recordcount= 0;
  RgnCnt(R,(*R).parameters._.rootlvl,qRects,qRectQty,qPtr,DirQuery,DataQuery,recordcount);
  return (*R).RSTDone;
}

/************************************************************************/

boolean RegionQuery(t_RT r,
                    const typinterval *qRects,
                    Rint qRectQty,
                    void *qPtr,
                    QueryFunc DirQuery,
                    QueryFunc DataQuery,
                    void *mPtr,
                    QueryManageFunc Manage)

{
  RSTREE R= (RSTREE)r;
  boolean finish;
  
  if (R == NULL) {
    setRSTerr(R,"RegionQuery",RNULL);
    return FALSE;
  }
  (*R).RSTDone= TRUE;
  finish= FALSE;
  RgnQuery(R,(*R).parameters._.rootlvl,qRects,qRectQty,qPtr,DirQuery,DataQuery,mPtr,&finish,Manage);
  return (*R).RSTDone;
}

/************************************************************************/

boolean AllQuery(t_RT r,
                 void *mPtr,
                 QueryManageFunc Manage)

{
  RSTREE R= (RSTREE)r;
  boolean finish;
  
  if (R == NULL) {
    setRSTerr(R,"AllQuery",RNULL);
    return FALSE;
  }
  (*R).RSTDone= TRUE;
  finish= FALSE;
  All(R,(*R).parameters._.rootlvl,mPtr,&finish,Manage);
  return (*R).RSTDone;
}

/************************************************************************/

boolean ASCIIdump(t_RT r,
                  FILE *stream)

{
  RSTREE R= (RSTREE)r;
  char s[80];
  
  if (R == NULL) {
    setRSTerr(R,"ASCIIdump",RNULL);
    return FALSE;
  }
  (*R).RSTDone= TRUE;
  fprintf(stream,"name: ");
  fprintf(stream,"%s\n",(*R).mainname);
  fprintf(stream,"numbOfDimensions: ");
  fprintf(stream,strans("%2I\n",s),(*R).numbofdim);
  fprintf(stream,"dirPageSize: ");
  fprintf(stream,strans("%5I\n",s),(*R).parameters._.DIR.pagelen);
  fprintf(stream,"dataPageSize: ");
  fprintf(stream,strans("%5I\n",s),(*R).parameters._.DATA.pagelen);
  fprintf(stream,"-----\n");
  ASCdmp(R,(*R).parameters._.rootlvl,stream);
  return (*R).RSTDone;
}

/************************************************************************/
/* This function logically belongs to RSTQuery.c. It is located here, to
   avoid various versions of that module. Called by ASCIIdump. */

static void ASCdmp(RSTREE R,
                   Rint level,
                   FILE *stream)

{
  char s[80];
  refnode n;
  void *ptrNi;
  Rint i, lv;
  
  n= (*R).L[level].N;
  ptrNi= n;
  ptrNi+= (*R).Pent0;
  
  fprintf(stream,strans("l: %5I\n",s), level);
  if (level != 0) {
    fprintf(stream,"p: ");
    fprintf(stream,strans("%5N DIR",s),(*R).L[level].P);
    for (lv= level+1; lv <= (*R).parameters._.rootlvl; lv++) {
      fprintf(stream,strans(" <- %5N",s),(*R).L[lv].P);
    }
    fprintf(stream,"\n");
    fprintf(stream,strans("n: %5I\n",s),(Rint)(*n).s.nofentries);
    for (i= 0; i < (*n).s.nofentries; i++) {
      PrintEntry(R,ptrNi,level,stream);
      ptrNi+= (*R).DIR.entLen;
    }

    for (i= 0; i < (*n).s.nofentries; i++) {
      ExtendPath(R,i,level);
      ASCdmp(R,level-1,stream);
    }
  } else {
    fprintf(stream,"p: ");
    fprintf(stream,strans("%5N DAT",s),(*R).L[level].P);
    for (lv= level+1; lv <= (*R).parameters._.rootlvl; lv++) {
      fprintf(stream,strans(" <- %5N",s),(*R).L[lv].P);
    }
    fprintf(stream,"\n");
    fprintf(stream,strans("n: %5I\n",s),(Rint)(*n).s.nofentries);
    for (i= 0; i < (*n).s.nofentries; i++) {
      (*R).L[level].E= i;
      PrintEntry(R,ptrNi,level,stream);
      ptrNi+= (*R).DATA.entLen;
    }
  }
}

/************************************************************************/
/* This function logically belongs to RSTQuery.c. It is located here, to
   avoid various versions of that module. Called by ASCdmp. */

static void PrintEntry(RSTREE R,
                       void *ptr,
                       Rint level,
                       FILE *stream)

{
  char s[80];
  Rint d;
  
  for (d= 0; d < (*R).numbofdim; d++) {
    if (d == 0) {
      fprintf(stream,"e: ");
    }
    else {
      fprintf(stream,"   ");
    }
    fprintf(stream,"% 23.16e",(*(refinterval)ptr).l);
    fprintf(stream," % 23.16e",(*(refinterval)ptr).h);
    ptr+= SIZEinterval;
    if (d < (*R).numbofdim - 1) {
      fprintf(stream,"\n");
    }
    else {
      /* ptr behind rectangle */
      if (level == 0) {
        fprintf(stream,strans(" -> %5N REC\n",s),(*(refinfo)ptr).tag);
      }
      else if (level == 1) {
        fprintf(stream,strans(" -> %5N DAT\n",s),*(refptrtosub)ptr);
      }
      else {
        fprintf(stream,strans(" -> %5N DIR\n",s),*(refptrtosub)ptr);
      }
    }
  }
}

/************************************************************************/

boolean SpJoinCount(t_RT r1,
                    t_RT r2,
                    Rlint *paircount)

{
  RSTREE R1= (RSTREE)r1;
  RSTREE R2= (RSTREE)r2;
  Rlint mark;
  boolean twiceopen, success;
  Rint rootlvl1, rootlvl2, nRecsDummy;
  refinterval rootRectR1, rootRectR2, interRect;
  
  if (R1 == NULL || R2 == NULL) {
    *paircount= 0;
    setRSTerr(R1,"SpJoinCount",RNULL);
    return FALSE;
  }
  if ((*R1).numbofdim != (*R2).numbofdim) {
    *paircount= 0;
    setRSTerr(R1,"SpJoinCount",treeDimMismatch);
    return FALSE;
  }
  rootlvl1= (*R1).parameters._.rootlvl;
  rootlvl2= (*R2).parameters._.rootlvl;
  twiceopen= R1 == R2;
  if (twiceopen) {
    R2= NULL;
    if ((*R1).storkind == pri) {
      success= CreateMMRSTIdentity(&R2,R1);
    }
    else {
      /* (*R).secbound */
      SyncPath(R1);
      WriteParamsPDs(R1);
      if ((*R1).storkind == lru) {
        if (! LRUSatisfies((*R1).LRU,2 * (*R1).parameters._.rootlvl + 1)) {
          setRSTerr(R1,"SpJoinCount",nBufPagesLss);
          return FALSE;
        }
        success= OpenBufRSTIdentity(&R2,R1);
      }
      else {
        success= OpenRST((t_RT*)&R2,(*R1).mainname);
      }
    } /* NEW(R2) */
    if (! success) {
      fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
      fprintf(stderr,"%s\n","SpJoinCount 1");
      (*R2).RSTDone= FALSE;
    }
  }
  if (! ((*R1).RSTDone && (*R2).RSTDone) ) {
    *paircount= 0;
    return FALSE;
  }
  
  /* InitCount should have been called for R2 */
  if ((*R1).count.on) {
    CountsOn((t_RT)R2);
  }
  rootRectR1= allocM((*R1).SIZErect);
  rootRectR2= allocM((*R1).SIZErect);
  interRect= allocM((*R1).SIZErect);
  GetRootMBB((t_RT)R1,&nRecsDummy,rootRectR1);
  GetRootMBB((t_RT)R2,&nRecsDummy,rootRectR2);
  GetOvlpRect((*R1).numbofdim,rootRectR1,rootRectR2,interRect);
  *paircount= 0;
  mark= 0;
  SpJnCnt(R1,rootlvl1,
          R2,rootlvl2,
          interRect,
          paircount,&mark);
  success= (*R1).RSTDone && (*R2).RSTDone;
  if (twiceopen) {
    JoinJoinCounts(R1,R2);
    if ((*R1).storkind == pri) {
      RemoveMMRSTIdentity(&R2);
    }
    else if ((*R1).storkind == lru) {
      success= success && CloseBufRSTIdentity(&R2);
    }
    else {
      success= success && CloseRSTIdentity(&R2);
    }
  }
  freeM(&rootRectR1);
  freeM(&rootRectR2);
  freeM(&interRect);
  return success;
}

/************************************************************************/

boolean SpJoin(t_RT r1,
               t_RT r2,
               void *mPtr,
               JoinManageFunc Manage)

{
  RSTREE R1= (RSTREE)r1;
  RSTREE R2= (RSTREE)r2;
  boolean twiceopen, success;
  boolean finish;
  Rint rootlvl1, rootlvl2, nRecsDummy;
  refinterval rootRectR1, rootRectR2, interRect;
  
  if (R1 == NULL || R2 == NULL) {
    setRSTerr(R1,"SpJoin",RNULL);
    return FALSE;
  }
  if ((*R1).numbofdim != (*R2).numbofdim) {
    setRSTerr(R1,"SpJoin",treeDimMismatch);
    return FALSE;
  }
  rootlvl1= (*R1).parameters._.rootlvl;
  rootlvl2= (*R2).parameters._.rootlvl;
  twiceopen= R1 == R2;
  if (twiceopen) {
    R2= NULL;
    if ((*R1).storkind == pri) {
      success= CreateMMRSTIdentity(&R2,R1);
    }
    else {
      /* (*R).secbound */
      SyncPath(R1);
      WriteParamsPDs(R1);
      if ((*R1).storkind == lru) {
        if (! LRUSatisfies((*R1).LRU,2 * (*R1).parameters._.rootlvl + 1)) {
          setRSTerr(R1,"SpJoin",nBufPagesLss);
          return FALSE;
        }
        success= OpenBufRSTIdentity(&R2,R1);
      }
      else {
        success= OpenRST((t_RT*)&R2,(*R1).mainname);
      }
    } /* NEW(R2) */
    if (! success) {
      fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
      fprintf(stderr,"%s\n","SpJoin 1");
      (*R2).RSTDone= FALSE;
    }
  }
  if (! ((*R1).RSTDone && (*R2).RSTDone) ) {
    return FALSE;
  }
  
  /* InitCount should have been called for R2 */
  if ((*R1).count.on) {
    CountsOn((t_RT)R2);
  }
  rootRectR1= allocM((*R1).SIZErect);
  rootRectR2= allocM((*R1).SIZErect);
  interRect= allocM((*R1).SIZErect);
  GetRootMBB((t_RT)R1,&nRecsDummy,rootRectR1);
  GetRootMBB((t_RT)R2,&nRecsDummy,rootRectR2);
  GetOvlpRect((*R1).numbofdim,rootRectR1,rootRectR2,interRect);
  finish= FALSE;
  SpJn(R1,rootlvl1,
       R2,rootlvl2,
       interRect,
       mPtr,&finish,Manage);
  success= (*R1).RSTDone && (*R2).RSTDone;
  if (twiceopen) {
    JoinJoinCounts(R1,R2);
    if ((*R1).storkind == pri) {
      RemoveMMRSTIdentity(&R2);
    }
    else if ((*R1).storkind == lru) {
      success= success && CloseBufRSTIdentity(&R2);
    }
    else {
      success= success && CloseRSTIdentity(&R2);
    }
  }
  freeM(&rootRectR1);
  freeM(&rootRectR2);
  freeM(&interRect);
  return success;
}

/************************************************************************/

boolean XSpJoinCount(t_RT r1,
                     const typinterval *R1qRects,
                     Rint R1qRectQty,
                     void *R1qPtr,
                     QueryFunc Dir1Query,
                     QueryFunc Data1Query,
                     t_RT r2,
                     const typinterval *R2qRects,
                     Rint R2qRectQty,
                     void *R2qPtr,
                     QueryFunc Dir2Query,
                     QueryFunc Data2Query,
                     /*        DirJoin fixed to DirOvlps */
                     JoinFunc DataJoin,
                     Rlint *paircount)

{
  RSTREE R1= (RSTREE)r1;
  RSTREE R2= (RSTREE)r2;
  Rlint mark;
  boolean twiceopen, success;
  Rint rootlvl1, rootlvl2, nRecsDummy;
  refinterval rootRectR1, rootRectR2, interRect;
  
  if (R1 == NULL || R2 == NULL) {
    *paircount= 0;
    setRSTerr(R1,"XSpJoinCount",RNULL);
    return FALSE;
  }
  if ((*R1).numbofdim != (*R2).numbofdim) {
    *paircount= 0;
    setRSTerr(R1,"XSpJoinCount",treeDimMismatch);
    return FALSE;
  }
  rootlvl1= (*R1).parameters._.rootlvl;
  rootlvl2= (*R2).parameters._.rootlvl;
  twiceopen= R1 == R2;
  if (twiceopen) {
    R2= NULL;
    if ((*R1).storkind == pri) {
      success= CreateMMRSTIdentity(&R2,R1);
    }
    else {
      /* (*R).secbound */
      SyncPath(R1);
      WriteParamsPDs(R1);
      if ((*R1).storkind == lru) {
        if (! LRUSatisfies((*R1).LRU,2 * (*R1).parameters._.rootlvl + 1)) {
          setRSTerr(R1,"XSpJoinCount",nBufPagesLss);
          return FALSE;
        }
        success= OpenBufRSTIdentity(&R2,R1);
      }
      else {
        success= OpenRST((t_RT*)&R2,(*R1).mainname);
      }
    } /* NEW(R2) */
    if (! success) {
      fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
      fprintf(stderr,"%s\n","XSpJoinCount 1");
      (*R2).RSTDone= FALSE;
    }
  }
  if (! ((*R1).RSTDone && (*R2).RSTDone) ) {
    *paircount= 0;
    return FALSE;
  }
  
  /* InitCount should have been called for R2 */
  if ((*R1).count.on) {
    CountsOn((t_RT)R2);
  }
  rootRectR1= allocM((*R1).SIZErect);
  rootRectR2= allocM((*R1).SIZErect);
  interRect= allocM((*R1).SIZErect);
  GetRootMBB((t_RT)R1,&nRecsDummy,rootRectR1);
  GetRootMBB((t_RT)R2,&nRecsDummy,rootRectR2);
  GetOvlpRect((*R1).numbofdim,rootRectR1,rootRectR2,interRect);
  *paircount= 0;
  mark= 0;
  XSpJnCnt(R1,rootlvl1,R1qRects,R1qRectQty,R1qPtr,Dir1Query,Data1Query,
           R2,rootlvl2,R2qRects,R2qRectQty,R2qPtr,Dir2Query,Data2Query,
           interRect,
           DirOvlps,DataJoin,
           paircount,&mark);
  success= (*R1).RSTDone && (*R2).RSTDone;
  if (twiceopen) {
    JoinJoinCounts(R1,R2);
    if ((*R1).storkind == pri) {
      RemoveMMRSTIdentity(&R2);
    }
    else if ((*R1).storkind == lru) {
      success= success && CloseBufRSTIdentity(&R2);
    }
    else {
      success= success && CloseRSTIdentity(&R2);
    }
  }
  freeM(&rootRectR1);
  freeM(&rootRectR2);
  freeM(&interRect);
  return success;
}

/************************************************************************/

boolean XSpJoin(t_RT r1,
                const typinterval *R1qRects,
                Rint R1qRectQty,
                void *R1qPtr,
                QueryFunc Dir1Query,
                QueryFunc Data1Query,
                t_RT r2,
                const typinterval *R2qRects,
                Rint R2qRectQty,
                void *R2qPtr,
                QueryFunc Dir2Query,
                QueryFunc Data2Query,
                /*        DirJoin fixed to DirOvlps */
                JoinFunc DataJoin,
                void *mPtr,
                JoinManageFunc Manage)

{
  RSTREE R1= (RSTREE)r1;
  RSTREE R2= (RSTREE)r2;
  boolean twiceopen, success;
  boolean finish;
  Rint rootlvl1, rootlvl2, nRecsDummy;
  refinterval rootRectR1, rootRectR2, interRect;
  
  if (R1 == NULL || R2 == NULL) {
    setRSTerr(R1,"XSpJoin",RNULL);
    return FALSE;
  }
  if ((*R1).numbofdim != (*R2).numbofdim) {
    setRSTerr(R1,"XSpJoin",treeDimMismatch);
    return FALSE;
  }
  rootlvl1= (*R1).parameters._.rootlvl;
  rootlvl2= (*R2).parameters._.rootlvl;
  twiceopen= R1 == R2;
  if (twiceopen) {
    R2= NULL;
    if ((*R1).storkind == pri) {
      success= CreateMMRSTIdentity(&R2,R1);
    }
    else {
      /* (*R).secbound */
      SyncPath(R1);
      WriteParamsPDs(R1);
      if ((*R1).storkind == lru) {
        if (! LRUSatisfies((*R1).LRU,2 * (*R1).parameters._.rootlvl + 1)) {
          setRSTerr(R1,"XSpJoin",nBufPagesLss);
          return FALSE;
        }
        success= OpenBufRSTIdentity(&R2,R1);
      }
      else {
        success= OpenRST((t_RT*)&R2,(*R1).mainname);
      }
    } /* NEW(R2) */
    if (! success) {
      fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
      fprintf(stderr,"%s\n","XSpJoin 1");
      (*R2).RSTDone= FALSE;
    }
  }
  if (! ((*R1).RSTDone && (*R2).RSTDone) ) {
    return FALSE;
  }
  
  /* InitCount should have been called for R2 */
  if ((*R1).count.on) {
    CountsOn((t_RT)R2);
  }
  rootRectR1= allocM((*R1).SIZErect);
  rootRectR2= allocM((*R1).SIZErect);
  interRect= allocM((*R1).SIZErect);
  GetRootMBB((t_RT)R1,&nRecsDummy,rootRectR1);
  GetRootMBB((t_RT)R2,&nRecsDummy,rootRectR2);
  GetOvlpRect((*R1).numbofdim,rootRectR1,rootRectR2,interRect);
  finish= FALSE;
  XSpJn(R1,rootlvl1,R1qRects,R1qRectQty,R1qPtr,Dir1Query,Data1Query,
        R2,rootlvl2,R2qRects,R2qRectQty,R2qPtr,Dir2Query,Data2Query,
        interRect,
        DirOvlps,DataJoin,
        mPtr,&finish,Manage);
  success= (*R1).RSTDone && (*R2).RSTDone;
  if (twiceopen) {
    JoinJoinCounts(R1,R2);
    if ((*R1).storkind == pri) {
      RemoveMMRSTIdentity(&R2);
    }
    else if ((*R1).storkind == lru) {
      success= success && CloseBufRSTIdentity(&R2);
    }
    else {
      success= success && CloseRSTIdentity(&R2);
    }
  }
  freeM(&rootRectR1);
  freeM(&rootRectR2);
  freeM(&interRect);
  return success;
}

/************************************************************************/

boolean XJoinCount(t_RT r1,
                   const typinterval *R1qRects,
                   Rint R1qRectQty,
                   void *R1qPtr,
                   QueryFunc Dir1Query,
                   QueryFunc Data1Query,
                   t_RT r2,
                   const typinterval *R2qRects,
                   Rint R2qRectQty,
                   void *R2qPtr,
                   QueryFunc Dir2Query,
                   QueryFunc Data2Query,
                   JoinFunc DirJoin,
                   JoinFunc DataJoin,
                   Rlint *paircount)

{
  RSTREE R1= (RSTREE)r1;
  RSTREE R2= (RSTREE)r2;
  Rlint mark;
  boolean twiceopen, success;
  Rint rootlvl1, rootlvl2;
  
  if (R1 == NULL || R2 == NULL) {
    *paircount= 0;
    setRSTerr(R1,"XJoinCount",RNULL);
    return FALSE;
  }
  if ((*R1).numbofdim != (*R2).numbofdim) {
    *paircount= 0;
    setRSTerr(R1,"XJoinCount",treeDimMismatch);
    return FALSE;
  }
  rootlvl1= (*R1).parameters._.rootlvl;
  rootlvl2= (*R2).parameters._.rootlvl;
  twiceopen= R1 == R2;
  if (twiceopen) {
    R2= NULL;
    if ((*R1).storkind == pri) {
      success= CreateMMRSTIdentity(&R2,R1);
    }
    else {
      /* (*R).secbound */
      SyncPath(R1);
      WriteParamsPDs(R1);
      if ((*R1).storkind == lru) {
        if (! LRUSatisfies((*R1).LRU,2 * (*R1).parameters._.rootlvl + 1)) {
          setRSTerr(R1,"XJoinCount",nBufPagesLss);
          return FALSE;
        }
        success= OpenBufRSTIdentity(&R2,R1);
      }
      else {
        success= OpenRST((t_RT*)&R2,(*R1).mainname);
      }
    } /* NEW(R2) */
    if (! success) {
      fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
      fprintf(stderr,"%s\n","XJoinCount 1");
      (*R2).RSTDone= FALSE;
    }
  }
  if (! ((*R1).RSTDone && (*R2).RSTDone) ) {
    *paircount= 0;
    return FALSE;
  }
  
  /* InitCount should have been called for R2 */
  if ((*R1).count.on) {
    CountsOn((t_RT)R2);
  }
  *paircount= 0;
  mark= 0;
  XJnCnt(R1,rootlvl1,R1qRects,R1qRectQty,R1qPtr,Dir1Query,Data1Query,
         R2,rootlvl2,R2qRects,R2qRectQty,R2qPtr,Dir2Query,Data2Query,
         DirJoin,DataJoin,
         paircount,&mark);
  success= (*R1).RSTDone && (*R2).RSTDone;
  if (twiceopen) {
    JoinJoinCounts(R1,R2);
    if ((*R1).storkind == pri) {
      RemoveMMRSTIdentity(&R2);
    }
    else if ((*R1).storkind == lru) {
      success= success && CloseBufRSTIdentity(&R2);
    }
    else {
      success= success && CloseRSTIdentity(&R2);
    }
  }
  return success;
}

/************************************************************************/

boolean XJoin(t_RT r1,
              const typinterval *R1qRects,
              Rint R1qRectQty,
              void *R1qPtr,
              QueryFunc Dir1Query,
              QueryFunc Data1Query,
              t_RT r2,
              const typinterval *R2qRects,
              Rint R2qRectQty,
              void *R2qPtr,
              QueryFunc Dir2Query,
              QueryFunc Data2Query,
              JoinFunc DirJoin,
              JoinFunc DataJoin,
              void *mPtr,
              JoinManageFunc Manage)

{
  RSTREE R1= (RSTREE)r1;
  RSTREE R2= (RSTREE)r2;
  boolean twiceopen, success;
  boolean finish;
  Rint rootlvl1, rootlvl2;
  
  if (R1 == NULL || R2 == NULL) {
    setRSTerr(R1,"XJoin",RNULL);
    return FALSE;
  }
  if ((*R1).numbofdim != (*R2).numbofdim) {
    setRSTerr(R1,"XJoin",treeDimMismatch);
    return FALSE;
  }
  rootlvl1= (*R1).parameters._.rootlvl;
  rootlvl2= (*R2).parameters._.rootlvl;
  twiceopen= R1 == R2;
  if (twiceopen) {
    R2= NULL;
    if ((*R1).storkind == pri) {
      success= CreateMMRSTIdentity(&R2,R1);
    }
    else {
      /* (*R).secbound */
      SyncPath(R1);
      WriteParamsPDs(R1);
      if ((*R1).storkind == lru) {
        if (! LRUSatisfies((*R1).LRU,2 * (*R1).parameters._.rootlvl + 1)) {
          setRSTerr(R1,"XJoin",nBufPagesLss);
          return FALSE;
        }
        success= OpenBufRSTIdentity(&R2,R1);
      }
      else {
        success= OpenRST((t_RT*)&R2,(*R1).mainname);
      }
    } /* NEW(R2) */
    if (! success) {
      fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
      fprintf(stderr,"%s\n","XJoin 1");
      (*R2).RSTDone= FALSE;
    }
  }
  if (! ((*R1).RSTDone && (*R2).RSTDone) ) {
    return FALSE;
  }
  
  /* InitCount should have been called for R2 */
  if ((*R1).count.on) {
    CountsOn((t_RT)R2);
  }
  finish= FALSE;
  XJn(R1,rootlvl1,R1qRects,R1qRectQty,R1qPtr,Dir1Query,Data1Query,
      R2,rootlvl2,R2qRects,R2qRectQty,R2qPtr,Dir2Query,Data2Query,
      DirJoin,DataJoin,
      mPtr,&finish,Manage);
  success= (*R1).RSTDone && (*R2).RSTDone;
  if (twiceopen) {
    JoinJoinCounts(R1,R2);
    if ((*R1).storkind == pri) {
      RemoveMMRSTIdentity(&R2);
    }
    else if ((*R1).storkind == lru) {
      success= success && CloseBufRSTIdentity(&R2);
    }
    else {
      success= success && CloseRSTIdentity(&R2);
    }
  }
  return success;
}

/************************************************************************/

void InitDistQueryIdent(t_DQ *distQ)

{
  *distQ= NULL;
}

/************************************************************************/
/* This comment could also be located in RSTDistQueryFuncs.h, where the
   constants MinPriorQueueInitMax and PriorQueueExtensionFactor are set:
   RSTDistHeap assumes that the priority queue maximum will be actually
   greater after extension by PriorQueueExtensionFactor M_SQRT2 (the result
   is not rounded). This does not hold for 0, 1, 2. Thus a minimum for
   priorQueueMax is anyway required. Setting MinPriorQueueInitMax to 128
   does not consume noteworthy memory, even for a large number of dimensions,
   and is nearly never sufficient in an all purpose setting, but yields a
   reasonable base for the 1st extension. */

boolean NewDistQuery(t_RT r,
                     const typcoord *qPoint,
                     DistCalcFunc DistFunc, 
                     DistQuerySort qSort,
                     RectDistType distType,
                     Rfloat stDist,
                     const typinterval *fRects,
                     Rint fRectsQty,
                     void *fPtr,
                     QueryFunc DirFilter,
                     QueryFunc DataFilter,
                     Rint priorQueueMax,
                     boolean verbosePriorQExt,
                     t_DQ *distQ)
{
  RSTREE R= (RSTREE)r;
  DISTQ DQ;
  Rint rootlvl, d, id;
  
  if (R == NULL) {
    setRSTerr(R,"NewDistQuery",RNULL);
    return FALSE;
  }
  if (*distQ != NULL) {
    setRSTerr(R,"NewDistQuery",distQnotNULL);
    return FALSE;
  }
  if (qSort < 0 || qSort > dec || distType < 0 || distType > maxDist) {
    setRSTerr(R,"NewDistQuery",querySort_distType_range);
    return FALSE;
  }
  
  *distQ= allocM(sizeof(distquery));
  DQ= (DISTQ)*distQ;
  
  if (DQ == NULL) {
    setRSTerr(R,"NewDistQuery",reAlloc);
    return FALSE;
  }
  if (priorQueueMax < MinPriorQueueInitMax) {
    priorQueueMax= MinPriorQueueInitMax;
  }
  if (! DH_New(&(*DQ).H,
        (*R).DHELen,
        priorQueueMax,
        PriorQueueExtensionFactor,
        verbosePriorQExt)) {
    freeM(distQ);
    setRSTerr(R,"NewDistQuery: Priority Queue",reAlloc);
    return FALSE;
  }
  (*DQ).qPoint= allocM((*R).SIZEpoint);
  if ((*DQ).qPoint == NULL) {
    DH_Dispose(&(*DQ).H);
    freeM(distQ);
    setRSTerr(R,"NewDistQuery: queryPoint",reAlloc);
    return FALSE;
  }
  if (fRectsQty != 0) {
    (*DQ).fRects= allocM(fRectsQty * (*R).SIZErect);
    if ((*DQ).fRects == NULL) {
      DH_Dispose(&(*DQ).H);
      freeM(&(*DQ).qPoint);
      freeM(distQ);
      setRSTerr(R,"NewDistQuery: filterRects",reAlloc);
      return FALSE;
    }
  }
  else {
    (*DQ).fRects= NULL;
  }
  (*DQ).R= R;
  (*R).RSTDone= TRUE;
  
  for (d= 0; d < (*R).numbofdim; d++) {
    (*DQ).qPoint[d]= qPoint[d];
  }
  (*DQ).CalcDist= DistFunc;
  (*DQ).sort= qSort;
  if (qSort == inc) {
    (*DQ).DH_Inst= minDH_Insert;
    (*DQ).DH_Pop= minDH_Pop;
    (*DQ).DirDist= PRectDist;
    (*DQ).DirStDist= PRectMaxDist;
  }
  else {
    (*DQ).DH_Inst= maxDH_Insert;
    (*DQ).DH_Pop= maxDH_Pop;
    (*DQ).DirDist= PRectMaxDist;
    (*DQ).DirStDist= PRectDist;
  }
  if (distType == minDist) {
    (*DQ).DataDist= PRectDist;
  }
  else if (distType == cntrDist) {
    (*DQ).DataDist= PRectCntrDist;
  }
  else {
    (*DQ).DataDist= PRectMaxDist;
  }
  (*DQ).stDist= stDist;
  for (id= 0; id < fRectsQty * (*R).numbofdim; id++) {
    (*DQ).fRects[id]= fRects[id];
  }
  (*DQ).fRectsQty= fRectsQty;
  (*DQ).fPtr= fPtr;
  (*DQ).DirFilter= DirFilter;
  (*DQ).DataFilter= DataFilter;
  
  SyncPath(R);
  /* the distance query does not share the standard path */
  rootlvl= (*R).parameters._.rootlvl;
  PushNodePriorQ(DQ,R,(*R).L[rootlvl].N,rootlvl);
  
  if ((*R).RSTDone) {
    /* independent of the Counts-Switch: */
    (*R).count.DQ_PQlen= (*DQ).H.qty;
    (*R).count.DQ_PQmax= (*DQ).H.qty;
  }
  
  return (*R).RSTDone;
}

/************************************************************************/

boolean DisposeDistQuery(t_DQ *distQ)

{
  DISTQ DQ= (DISTQ)*distQ;
  refcount c;
  
  if (DQ == NULL) {
    setRSTerr(NULL,"DisposeDistQuery",distQNULL);
    return FALSE;
  }
  
  c= &(*(*DQ).R).count;
  /* independent of the Counts-Switch: */
  (*c).DQ_PQelems+= (*c).DQ_PQmax;
  
  DH_Dispose(&(*DQ).H);
  freeM(&(*DQ).qPoint);
  freeM(&(*DQ).fRects);
  freeM(distQ);
  return TRUE;
}

/************************************************************************/

boolean GetDistQueryRec(t_DQ distQ,
                        t_RT r,
                        typinterval *rectangle,
                        refinfo info,
                        Rfloat *rawDist,
                        boolean *available)
{
  DISTQ DQ= (DISTQ)distQ;
  RSTREE R= (RSTREE)r;
  refcount c;
  
  if (DQ == NULL) {
    setRSTerr(R,"GetDistQueryRec",distQNULL);
    return FALSE;
  }
  if (R != (*DQ).R) {
    setRSTerr(R,"GetDistQueryRec",RdistQmismatch);
    return FALSE;
  }
  (*R).RSTDone= TRUE;
  *available= DistQueryNext(DQ,R,rectangle,info,rawDist);
  
  if (*available) {
  
    c= &(*R).count;
    /* independent of the Counts-Switch: */
    (*c).DQ_PQlen= (*DQ).H.qty + 1; /* add 1 for the just popped */
    if ((*c).DQ_PQlen > (*c).DQ_PQmax) {
      (*c).DQ_PQmax= (*c).DQ_PQlen;
    } 
  }
  
  return (*R).RSTDone;
}

/************************************************************************/

boolean InquireRSTDesc(t_RT     r,
                         char     *name,
                         Rint     *subtreePtrSize,
                         Rint     *infoSize,
                         Rint     *numbOfDimensions,
                         Rint     *dirPageSize,
                         Rint     *dataPageSize,
                         Rint     *netDirPageSize,
                         Rint     *netDataPageSize,
                         Rint     *dirEntrySize,
                         Rint     *dataEntrySize,
                         Rint     *maxDirFanout,
                         Rint     *maxDataFanout,
                         Rint     *minDirFanout,
                         Rint     *minDataFanout,
                         Rint     *minDirDELrest,
                         Rint     *minDataDELrest,
                         Rpnint   *numbOfDirPages,
                         Rpnint   *numbOfDataPages,
                         Rlint    *numbOfRecords,
                         Rint     *rootLevel,
                         boolean  *unique,
                         Rpnint   pagesPerLevel[])

{
  RSTREE R= (RSTREE)r;
  refparameters par;
  refversion ver;
  Rint lv;
  
  if (R == NULL) {
    setRSTerr(R,"InquireRSTDesc",RNULL);
    return FALSE;
  }
  
  strlcpy(name,(*R).mainname,sizeof(RSTName));
  
  par= &(*R).parameters._;
  ver= &(*R).version._;
  
  *subtreePtrSize= (*par).SIZE_ptrtosub;
  *infoSize= (*par).SIZEinfo;
  *numbOfDimensions= (*par).numbofdim;
  *dirPageSize= (*par).DIR.pagelen;
  *dataPageSize= (*par).DATA.pagelen;
  *netDirPageSize= (*par).DIR.pagelen - (*par).DIR.entriesoffset;
  *netDataPageSize= (*par).DATA.pagelen - (*par).DATA.entriesoffset;
  *dirEntrySize= (*par).DIR.entrylen;
  *dataEntrySize= (*par).DATA.entrylen;
  *maxDirFanout= (*par).DIR.M;
  *maxDataFanout= (*par).DATA.M;
  *minDirFanout= (*ver).DIR.m;
  *minDataFanout= (*ver).DATA.m;
  *minDirDELrest= (*ver).DIR.DELm;
  *minDataDELrest= (*ver).DATA.DELm;
  if ((*par).rootlvl == 0) {
    *numbOfDirPages= (*par).DATA.pagecount;
    *numbOfDataPages= (*par).DIR.pagecount;
  }
  else {
    *numbOfDirPages= (*par).DIR.pagecount;
    *numbOfDataPages= (*par).DATA.pagecount;
  }
  *numbOfRecords= (*par).recordcount;
  *rootLevel= (*par).rootlvl;
  *unique= (*par).unique;
  for (lv= 0; lv < (*par).rootlvl; lv++) {
    pagesPerLevel[lv]= (*par).PageCount[lv];
  }
  pagesPerLevel[(*par).rootlvl]= 1;
  return TRUE;
}

/************************************************************************/

boolean  GetCreatRSTDesc(t_RT   r,
                         char     *name,
                         Rint     *dirPageSize,
                         Rint     *dataPageSize,
                         Rint     *numbOfDimensions,
                         Rint     *infoSize,
                         boolean  *unique)
                         
{
  RSTREE R= (RSTREE)r;
  refparameters par= &(*R).parameters._;
  
  if (R == NULL) {
    setRSTerr(R,"GetCreatRSTDesc",RNULL);
    return FALSE;
  }
  
  strlcpy(name,(*R).mainname,sizeof(RSTName));
  *dirPageSize= (*par).DIR.pagelen;
  *dataPageSize= (*par).DATA.pagelen;
  *numbOfDimensions= (*par).numbofdim;
  *infoSize= (*par).SIZEinfo;
  *unique= (*par).unique;
  return TRUE;
}

/************************************************************************/

boolean GetVarRSTDesc(t_RT   r,
                      char     *name,
                      Rpnint   *numbOfDirPages,
                      Rpnint   *numbOfDataPages,
                      Rlint    *numbOfRecords,
                      Rint     *rootLevel,
                      boolean  *unique,
                      Rpnint   pagesPerLevel[])

{
  RSTREE R= (RSTREE)r;
  refparameters par= &(*R).parameters._;
  Rint lv;
  
  if (R == NULL) {
    setRSTerr(R,"GetVarRSTDesc",RNULL);
    return FALSE;
  }
  
  strlcpy(name,(*R).mainname,sizeof(RSTName));
  *numbOfDirPages= (*par).DIR.pagecount;
  *numbOfDataPages= (*par).DATA.pagecount;
  *numbOfRecords= (*par).recordcount;
  *rootLevel= (*par).rootlvl;
  *unique= (*par).unique;
  for (lv= 0; lv < (*par).rootlvl; lv++) {
    pagesPerLevel[lv]= (*par).PageCount[lv];
  }
  pagesPerLevel[(*par).rootlvl]= 1;
  return TRUE;
}

/************************************************************************/

boolean GetCapParams(t_RT  r,
                     Rint    *subtreePtrSize,
                     Rint    *infoSize,
                     Rint    *numbOfDimensions,
                     Rint    *dirPageSize,
                     Rint    *dataPageSize,
                     Rint    *netDirPageSize,
                     Rint    *netDataPageSize,
                     Rint    *dirEntrySize,
                     Rint    *dataEntrySize) {

  RSTREE R= (RSTREE)r;
  
  if (R == NULL) {
    setRSTerr(R,"GetCapParams",RNULL);
    return FALSE;
  }
  
  refparameters par= &(*R).parameters._;
  
  *subtreePtrSize= (*par).SIZE_ptrtosub;
  *infoSize= (*par).SIZEinfo;
  *numbOfDimensions= (*par).numbofdim;
  *dirPageSize= (*par).DIR.pagelen;
  *dataPageSize= (*par).DATA.pagelen;
  *netDirPageSize= (*par).DIR.pagelen - (*par).DIR.entriesoffset;
  *netDataPageSize= (*par).DATA.pagelen - (*par).DATA.entriesoffset;
  *dirEntrySize= (*par).DIR.entrylen;
  *dataEntrySize= (*par).DATA.entrylen;
  return TRUE;
}

/************************************************************************/

boolean GetPageSizes(t_RT  r,
                     Rint    *dirPageSize,
                     Rint    *dataPageSize) {

  RSTREE R= (RSTREE)r;
  
  if (R == NULL) {
    setRSTerr(R,"GetPageSizes",RNULL);
    return FALSE;
  }
  
  refparameters par= &(*R).parameters._;
  
  *dirPageSize= (*par).DIR.pagelen;
  *dataPageSize= (*par).DATA.pagelen;
  return TRUE;
}

/************************************************************************/

boolean ExamRSTDescFile(const char *name)

{
  return ReadRSTDescFile(name);
}

/************************************************************************/

void PrintRSTImplLimits(void) {

  char s[160];
  
  fprintf(stdout,"+++  RSTree implementation dependent limits:\n");
  fprintf(stdout,"\n");
  fprintf(stdout,"INSTANCE                                  MAXIMUM\n");
  fprintf(stdout,"-------------------------------------------------\n");
  fprintf(stdout,"Number of dimensions       ");
  fprintf(stdout,strans("%22L\n",s),(Rlint)MAX_DIM);
  /* const of RSTBase */
  
  fprintf(stdout,"Number of entries per page ");
  fprintf(stdout,strans("%22L\n",s),(Rlint)MAX_ENTRIES);
  /* const of RSTBase */
  
  fprintf(stdout,"Number of data pages       ");
  fprintf(stdout,strans("%22L\n",s),(Rlint)MAX_DATA_PAGES);
  /* const of RSTBase */
  
  fprintf(stdout,"Height of a tree           ");
  fprintf(stdout,strans("%22L\n",s),(Rlint)PATH_RANGE);
  /* const of RSTBase */
  
  fprintf(stdout,"Length of a name (path)    ");
  fprintf(stdout,strans("%22L\n",s),(Rlint)MaxNameLength);
  /* const of RSTStdTypes */
}

/************************************************************************/

boolean CountsOn0(t_RT r)

{
  RSTREE R= (RSTREE)r;
  
  if (R == NULL) {
    setRSTerr(R,"CountsOn0",RNULL);
    return FALSE;
  }
  InitCount(R);
  (*R).count.on= TRUE;
  if ((*R).storkind == lru) {
    LRUCountsOn0((*R).LRU);
  }
  return TRUE;
}

/************************************************************************/

boolean CountsOn(t_RT r)

{
  RSTREE R= (RSTREE)r;
  
  if (R == NULL) {
    setRSTerr(R,"CountsOn",RNULL);
    return FALSE;
  }
  (*R).count.on= TRUE;
  if ((*R).storkind == lru) {
    LRUCountsOn((*R).LRU);
  }
  return TRUE;
}

/************************************************************************/

boolean CountsOff(t_RT r)

{
  RSTREE R= (RSTREE)r;
  
  if (R == NULL) {
    setRSTerr(R,"CountsOff",RNULL);
    return FALSE;
  }
  (*R).count.on= FALSE;
  if ((*R).storkind == lru) {
    LRUCountsOff((*R).LRU);
  }
  return TRUE;
}

/************************************************************************/

boolean GetCountRead(t_RT r,
                     Rlint *dirdemand, Rlint *datademand,
                     Rlint *dirget, Rlint *dataget,
                     Rlint *dirread, Rlint *dataread, Rlint *dataoptimal)

{
  RSTREE R= (RSTREE)r;
  refcount c;
  
  if (R == NULL) {
    *dirdemand= 0;
    *datademand= 0;
    *dirget= 0;
    *dataget= 0;
    *dirread= 0;
    *dataread= 0;
    setRSTerr(R,"GetCountRead",RNULL);
    return FALSE;
  }
  c= &(*R).count;
  *dirdemand= (*c).DIR.demand;
  *datademand= (*c).DATA.demand;
  *dirget= (*c).DIR.get;
  *dataget= (*c).DATA.get;
  *dataoptimal= (*c).useful_leaf_accesses;
  if ((*R).storkind == lru) {
    *dirread= 0; /* single buffer only, all counts done in the data level */
    LRUGetCountRead((*R).LRU,dataread);
  }
  else {
    *dirread= (*R).DIR.str.cnt.read;
    *dataread= (*R).DATA.str.cnt.read;
  }
  return TRUE;
}

/************************************************************************/

boolean GetCountAdminRead(t_RT r,
                          Rint *adminPageSize,
                          Rlint *dirread,
                          Rlint *dataread)

{
  RSTREE R= (RSTREE)r;
  
  if (R == NULL) {
    *dirread= 0;
    *dataread= 0;
    setRSTerr(R,"GetCountAdminRead",RNULL);
    return FALSE;
  }
  *adminPageSize= SIZE_FIX_BL;
  *dirread= (*R).DIR.PA.str.cnt.read;
  *dataread= (*R).DATA.PA.str.cnt.read;
  return TRUE;
}

/************************************************************************/

boolean GetCountWrite(t_RT r,
                      Rlint *dirput, Rlint *dataput,
                      Rlint *dirwrite, Rlint *datawrite)

{
  RSTREE R= (RSTREE)r;
  refcount c;
  
  if (R == NULL) {
    *dirput= 0;
    *dataput= 0;
    *dirwrite= 0;
    *datawrite= 0;
    setRSTerr(R,"GetCountWrite",RNULL);
    return FALSE;
  }
  c= &(*R).count;
  *dirput= (*c).DIR.put;
  *dataput= (*c).DATA.put;
  if ((*R).storkind == lru) {
    *dirwrite= 0; /* single buffer only, all counts done in the data level */
    LRUGetCountWrite((*R).LRU,datawrite);
  }
  else {
    *dirwrite= (*R).DIR.str.cnt.write;
    *datawrite= (*R).DATA.str.cnt.write;
  }
  return TRUE;
}

/************************************************************************/

boolean GetCountAdminWrite(t_RT r,
                           Rint *adminPageSize,
                           Rlint *dirwrite,
                           Rlint *datawrite)

{
  RSTREE R= (RSTREE)r;
  
  if (R == NULL) {
    *dirwrite= 0;
    *datawrite= 0;
    setRSTerr(R,"GetCountAdminWrite",RNULL);
    return FALSE;
  }
  *adminPageSize= SIZE_FIX_BL;
  *dirwrite= (*R).DIR.PA.str.cnt.write;
  *datawrite= (*R).DATA.PA.str.cnt.write;
  return TRUE;
}

/************************************************************************/

boolean GetCountRectComp(t_RT r,
                         Rlint *dircomp, Rlint *datacomp)

{
  RSTREE R= (RSTREE)r;
  refcount c;
  
  if (R == NULL) {
    *dircomp= 0;
    *datacomp= 0;
    setRSTerr(R,"GetCountRectComp",RNULL);
    return FALSE;
  }
  c= &(*R).count;
  *dircomp= (*c).DIR.comp;
  *datacomp= (*c).DATA.comp;
  return TRUE;
}

/************************************************************************/

boolean GetCountPriorQ(t_RT r,
                       Rint *PriorQlen,
                       Rint *PriorQmax,
                       Rlint *PriorQelems)

{
  RSTREE R= (RSTREE)r;
  refcount c;
  
  if (R == NULL) {
    *PriorQlen= 0;
    *PriorQmax= 0;
    *PriorQelems= 0;
    setRSTerr(R,"GetCountPriorQ",RNULL);
    return FALSE;
  }
  c= &(*R).count;
  *PriorQlen= (*c).DQ_PQlen;
  *PriorQmax= (*c).DQ_PQmax;
  *PriorQelems= (*c).DQ_PQelems;
  return TRUE;
}

/************************************************************************/

boolean GetCountOvUndFlw(t_RT r,
                         Rlint *diroverflow, Rlint *dataoverflow,
                         Rlint *dirunderflow, Rlint *dataunderflow,
                         Rlint *dirreinst, Rlint *datareinst,
                         Rlint *dirsplit, Rlint *datasplit,
                         Rlint *dirS_Area0, Rlint *dataS_Area0)

{
  RSTREE R= (RSTREE)r;
  refcount c;
  
  if (R == NULL) {
    *diroverflow= 0; *dataoverflow= 0;
    *dirreinst= 0; *datareinst= 0;
    *dirsplit= 0; *datasplit= 0;
    *dirS_Area0= 0; *dataS_Area0= 0;
    setRSTerr(R,"GetCountOvUndFlw",RNULL);
    return FALSE;
  }
  c= &(*R).count;
  *diroverflow= (*c).DIR.overflow;
  *dataoverflow= (*c).DATA.overflow;
  *dirunderflow= (*c).DIR.underflow;
  *dataunderflow= (*c).DATA.underflow;
  *dirreinst= (*c).DIR.reinst;
  *datareinst= (*c).DATA.reinst;
  *dirsplit= (*c).DIR.split;
  *datasplit= (*c).DATA.split;
  *dirS_Area0= (*c).DIR.S_Area0;
  *dataS_Area0= (*c).DATA.S_Area0;
  return TRUE;
}

/************************************************************************/

boolean GetCountChsSbtr(t_RT r,
                        Rlint *CS_Call,
                        Rlint *CS_NoFit,
                        Rlint *CS_UniFit,
                        Rlint *CS_SomeFit,
                        Rlint *CS_OvlpEnlOpt,
                        Rlint *CS_P,
                        Rlint *CS_MaxP,
                        Rlint *CS_PminusQ,
                        Rlint *CS_OvlpEnlComput,
                        Rlint *CS_P1OvlpEnl0,
                        Rlint *CS_AfterwOvlpEnl0,
                        Rlint *CS_Area0)

{
  RSTREE R= (RSTREE)r;
  refcount c;
  
  if (R == NULL) {
    *CS_Call= 0;
    *CS_NoFit= 0;
    *CS_UniFit= 0;
    *CS_SomeFit= 0;
    *CS_OvlpEnlOpt= 0;
    *CS_P= 0;
    *CS_MaxP= 0;
    *CS_PminusQ= 0;
    *CS_OvlpEnlComput= 0;
    *CS_P1OvlpEnl0= 0;
    *CS_AfterwOvlpEnl0= 0;
    *CS_Area0= 0;
    setRSTerr(R,"GetCountChsSbtr",RNULL);
    return FALSE;
  }
  c= &(*R).count;
    *CS_Call= (*c).CS_Call;
    *CS_NoFit= (*c).CS_NoFit;
    *CS_UniFit= (*c).CS_UniFit;
    *CS_SomeFit= (*c).CS_SomeFit;
    *CS_OvlpEnlOpt= (*c).CS_OvlpEnlOpt;
    *CS_P= (*c).CS_P;
    *CS_MaxP= (*c).CS_MaxP;
    *CS_PminusQ= (*c).CS_PminusQ;
    *CS_OvlpEnlComput= (*c).CS_OvlpEnlComput;
    *CS_P1OvlpEnl0= (*c).CS_P1OvlpEnl0;
    *CS_AfterwOvlpEnl0= (*c).CS_AfterwOvlpEnl0;
    *CS_Area0= (*c).CS_Area0;
  return TRUE;
}

/************************************************************************/

boolean GetPagesRecords(t_RT r,
                        Rpnint *dirpages,
                        Rpnint *datapages,
                        Rlint *records)

{
  RSTREE R= (RSTREE)r;
  refparameters par;
  
  if (R == NULL) {
    *dirpages= 0;
    *datapages= 0;
    *records= 0;
    setRSTerr(R,"GetPagesRecords",RNULL);
    return FALSE;
  }
  
  par= &(*R).parameters._;
  
  *dirpages= (*par).DIR.pagecount;
  *datapages= (*par).DATA.pagecount;
  *records= (*par).recordcount;
  return TRUE;
}

/************************************************************************/

boolean GetMaxFanout(t_RT r,
                     Rint *maxDirFanout,
                     Rint *maxDataFanout)

{
  RSTREE R= (RSTREE)r;
  refparameters par;

  if (R == NULL) {
    *maxDirFanout= 0;
    *maxDataFanout= 0;
    setRSTerr(R,"GetMaxFanout",RNULL);
    return FALSE;
  }
  
  par= &(*R).parameters._;
  
  *maxDirFanout= (*par).DIR.M;
  *maxDataFanout= (*par).DATA.M;
  return TRUE;
}

/************************************************************************/

boolean GetRootLevel(t_RT r,
                     Rint *rootlevel)

{
  RSTREE R= (RSTREE)r;
  
  if (R == NULL) {
    *rootlevel= 0;
    setRSTerr(R,"GetRootLevel",RNULL);
    return FALSE;
  }
  *rootlevel= (*R).parameters._.rootlvl;
  return TRUE;
}

/************************************************************************/

boolean GetHeight(t_RT r,
                  Rint *height)

{
  RSTREE R= (RSTREE)r;
  
  if (R == NULL) {
    *height= 0;
    setRSTerr(R,"GetHeight",RNULL);
    return FALSE;
  }
  *height= (*R).parameters._.rootlvl + 1;
  return TRUE;
}

/************************************************************************/

boolean GetName(t_RT r,
                char *name)

{
  RSTREE R= (RSTREE)r;
  
  if (R == NULL) {
    setRSTerr(R,"GetName",RNULL);
    return FALSE;
  }
  
  strlcpy(name,(*R).mainname,sizeof(RSTName));
  return TRUE;
}

/************************************************************************/

boolean GetGlobalMBB(t_RT r,
                     Rint *numbEnt,
                     typinterval *mbb)

{
  RSTREE R= (RSTREE)r;
  Rint rootlvl;
  refnode n;
  
  if (R == NULL) {
    setRSTerr(R,"GetGlobalMBB",RNULL);
    return FALSE;
  }
  
  rootlvl= (*R).parameters._.rootlvl;
  n= (*R).L[rootlvl].N;
  
  *numbEnt= (*n).s.nofentries;
  if (*numbEnt == 0) {
    return TRUE;
  }
  
  if (rootlvl == 0) {
    EvalDataNodesMBB(R,n,mbb);
  }
  else {
    EvalDirNodesMBB(R,n,mbb);
  }
  return TRUE;
}

/************************************************************************/
/* If GetRootMBB is not implemented, GetGlobalMBB can do the job. */

boolean GetRootMBB(t_RT r,
                   Rint *numbEnt,
                   typinterval *mbb)

{
  RSTREE R= (RSTREE)r;
  
  if (R == NULL) {
    setRSTerr(R,"GetRootMBB",RNULL);
    return FALSE;
  }
  *numbEnt= (*(*R).L[(*R).parameters._.rootlvl].N).s.nofentries;
  CopyRect((*R).numbofdim,(*R).rootMBB,mbb);
  return TRUE;
}

/************************************************************************/

boolean GetRAMdiskLimits(t_RT   r,
                         Rpint    *dirRAMsize,
                         Rpint    *dataRAMsize,
                         Rpint    *dirRAMutil,
                         Rpint    *dataRAMutil,
                         boolean  *dirLocked,
                         boolean  *dataLocked)

{
  RSTREE R= (RSTREE)r;
  refparameters par= &(*R).parameters._;
  
  if (R == NULL) {
    *dirRAMsize= 0;
    *dataRAMsize= 0;
    *dirRAMutil= 0;
    *dataRAMutil= 0;
    *dirLocked= FALSE;
    *dataLocked= FALSE;
    setRSTerr(R,"GetRAMdiskLimits",RNULL);
    return FALSE;
  }
  if ((*R).storkind != pri) {
    *dirRAMsize= 0;
    *dataRAMsize= 0;
    *dirRAMutil= 0;
    *dataRAMutil= 0;
    *dirLocked= FALSE;
    *dataLocked= FALSE;
    setRSTerr(R,"GetRAMdiskLimits",secMemTree);
    return FALSE;
  }
  *dirRAMsize= (*R).DIR.str.RAM.size;
  *dataRAMsize= (*R).DATA.str.RAM.size;
  if ((*par).rootlvl != 0) {
    *dirRAMutil= ((*R).DIR.PA.pagedir._.number[0] + 1) * (*par).DIR.pagelen;
  }
  else {
    *dirRAMutil= 0;
  }
  *dataRAMutil= ((*R).DATA.PA.pagedir._.number[0] + 1) * (*par).DATA.pagelen;
  /* Concerns both, (*R).DIR.PA.pagedir._. and (*R).DATA.PA.pagedir._.:
     number[0], initially 0, contains the number of the last page currently in
     use. It is incremented, during call-up for an additional page, before
     being assigned. The 0. page, kept free for the root, is the first page
     number. */
  *dirLocked= (*R).DIR.str.RAM.locked;
  *dataLocked= (*R).DATA.str.RAM.locked;
  return TRUE;
}

/************************************************************************/

boolean GetStorageKind(t_RT r,
                       char *token)

{
  RSTREE R= (RSTREE)r;
  
  if (R == NULL) {
    setRSTerr(R,"GetStorageKind",RNULL);
    strcpy(token,"");
    return FALSE;
  }
  switch ((*R).storkind) {
    case pri:
      strcpy(token,"pri");
      break;
    case lru:
      strcpy(token,"LRU");
      break;
    case sec:
      strcpy(token,"sec");
      break;
  }
  return TRUE;
}

/************************************************************************/

boolean TryGetStorageKind(t_RT r,
                          char *token)

{
  RSTREE R= (RSTREE)r;
  
  if (R == NULL) {
    strcpy(token,"");
    return FALSE;
  }
  switch ((*R).storkind) {
    case pri:
      strcpy(token,"pri");
      break;
    case lru:
      strcpy(token,"LRU");
      break;
    case sec:
      strcpy(token,"sec");
      break;
  }
  return TRUE;
}

/************************************************************************/

boolean ClearPath(t_RT r)

{
  RSTREE R= (RSTREE)r;
  Rint lv;
  
  if (R == NULL) {
    setRSTerr(R,"ClearPath",RNULL);
    return FALSE;
  }
  for (lv= 0; lv < (*R).parameters._.rootlvl; lv++) {
    if ((*R).L[lv].Modif) {
      PutNode(R,(*R).L[lv].N,(*R).L[lv].P,lv);
      (*R).L[lv].Modif= FALSE;
    }
    else {
      UnlinkPage(R,(*R).L[lv].P,lv);
    }
    (*R).L[lv].P= EMPTY_BL;
  }
  return TRUE;
}

/************************************************************************/

boolean CheckConsistency(t_RT r,
                         boolean *consistent,
                         boolean *rootMBBok,
                         boolean *otherMBBsOk,
                         typinterval *storedRootMBB,
                         typinterval *rootsMBB,
                         Rint entryNumberPath[],
                         Rint *parentLevel,
                         typinterval *storedMBB,
                         Rpnint *parentPageNr,
                         Rint *childLevel,
                         typinterval *childsMBB,
                         Rpnint *childPageNr,
                         Rpnint pagesPerLevel[])

{
  RSTREE R= (RSTREE)r;
  Rint lv, rootlvl, n;
  
  if (R == NULL) {
    setRSTerr(R,"CheckConsistency",RNULL);
    return FALSE;
  }
  
  GetRootMBB((t_RT)R,&n,storedRootMBB);
  GetGlobalMBB((t_RT)R,&n,rootsMBB);
  *rootMBBok= n == 0 || RectsEql((*R).numbofdim,storedRootMBB,rootsMBB);
  
  rootlvl= (*R).parameters._.rootlvl;
  *parentLevel= 1;     /* the lowest possible parentLevel */
  for (lv= 0; lv <= rootlvl; lv++) {
    entryNumberPath[lv]= -1;
    pagesPerLevel[lv]= 0;
  }
  
  (*R).RSTDone= TRUE;
  
  *otherMBBsOk= TRUE;
  ChckCnst(R,
           rootlvl,
           otherMBBsOk,
           parentLevel,
           storedMBB,
           parentPageNr,
           childLevel,
           childsMBB,
           childPageNr,
           pagesPerLevel);
  for (lv= *parentLevel; lv <= rootlvl; lv++) {
    entryNumberPath[lv]= (*R).L[lv].E;
  }
  *consistent= *rootMBBok && *otherMBBsOk;
  return TRUE;
}

/************************************************************************/
/* This function logically belongs to RSTQuery.c. It is located here, to
   avoid various versions of that module. Called by CheckConsistency. */

static void ChckCnst(RSTREE R,
                     Rint level,
                     boolean *consistency,
                     Rint *upperLevel,
                     typinterval *upperRect,
                     Rpnint *upperPage,
                     Rint *lowerLevel,
                     typinterval *lowerRect,
                     Rpnint *lowerPage,
                     Rpnint countNodes[])

{
  Rint i, next;
  refnode n;
  void *ptrNi, *ptrNiRect;
  typinterval *mbb= allocM((*R).SIZErect);
  
  if (level != 0) {
  
    n= (*R).L[level].N;
    ptrNi= n;
    ptrNi+= (*R).Pent0;
    
    next= level - 1;
    i= 0;
    do {
      ExtendPath(R,i,level);
      if (next > 0) {
        EvalDirNodesMBB(R,(*R).L[next].N,mbb);
      }
      else {
        EvalDataNodesMBB(R,(*R).L[next].N,mbb);
      }
      
      ptrNiRect= ptrNi + i * (*R).DIR.entLen;
      if (RectsEql((*R).numbofdim,ptrNiRect,mbb)) {
        ChckCnst(R,
                 next,
                 consistency,
                 upperLevel,
                 upperRect,
                 upperPage,
                 lowerLevel,
                 lowerRect,
                 lowerPage,
                 countNodes);
      }
      else {
        *consistency= FALSE;
        *upperLevel= level;
        CopyRect((*R).numbofdim,ptrNiRect,upperRect);
        *upperPage= (*R).L[level].P;
        *lowerLevel= next;
        CopyRect((*R).numbofdim,mbb,lowerRect);
        *lowerPage= (*R).L[next].P;
      }
      i++;
    } while (*consistency && i < (*n).s.nofentries);
    countNodes[level]++;
  }
  else {
    countNodes[level]++;
  }
  freeM(&mbb);
}

/************************************************************************/


boolean PathsDump(t_RT r)

{
  RSTREE R= (RSTREE)r;
  char s[80];
  Rint lv;
  refparameters par;
  
  if (R == NULL) {
    setRSTerr(R,"PathsDump",RNULL);
    return FALSE;
  }
  
  par= &(*R).parameters._;
  
  fprintf(stdout,"Level:     ");
  for (lv= (*par).rootlvl; lv >= 0; lv--) {
    fprintf(stdout,strans("%19I",s),lv);
  }
  fprintf(stdout,"\n");
  
  fprintf(stdout,"L.N:       ");
  for (lv= (*par).rootlvl; lv >= 0; lv--) {
    fprintf(stdout,strans("%#19Q",s),(Rpint)(*R).L[lv].N);
  }
  fprintf(stdout,"\n");
  
  fprintf(stdout,"L.P:       ");
  for (lv= (*par).rootlvl; lv >= 0; lv--) {
    fprintf(stdout,strans("%19N",s),(*R).L[lv].P);
  }
  fprintf(stdout,"\n");
  
  fprintf(stdout,"L.E:       ");
  for (lv= (*par).rootlvl; lv >= 0; lv--) {
    fprintf(stdout,strans("%19I",s),(*R).L[lv].E);
  }
  fprintf(stdout,"\n");
  
  fprintf(stdout,"L.Modif:   ");
  for (lv= (*par).rootlvl; lv >= 0; lv--) {
    fprintf(stdout,strans("%19I",s),(*R).L[lv].Modif);
  }
  fprintf(stdout,"\n");
  
  fprintf(stdout,"L.ReInsert:");
  for (lv= (*par).rootlvl; lv >= 0; lv--) {
    fprintf(stdout,strans("%19I",s),(*R).L[lv].ReInsert);
  }
  fprintf(stdout,"\n");
  
  fprintf(stdout,"LDel.N:    ");
  for (lv= (*par).rootlvl; lv >= 0; lv--) {
    fprintf(stdout,strans("%#19Q",s),(Rpint)(*R).LDel[lv].N);
  }
  fprintf(stdout,"\n");
  
  fprintf(stdout,"L1.N:      ");
  for (lv= (*par).rootlvl; lv >= 0; lv--) {
    fprintf(stdout,strans("%#19Q",s),(Rpint)(*R).L1[lv].N);
  }
  fprintf(stdout,"\n");
  
  fprintf(stdout,"L1.P:      ");
  for (lv= (*par).rootlvl; lv >= 0; lv--) {
    fprintf(stdout,strans("%19N",s),(*R).L1[lv].P);
  }
  fprintf(stdout,"\n");
  
  fprintf(stdout,"L1.Modif:  ");
  for (lv= (*par).rootlvl; lv >= 0; lv--) {
    fprintf(stdout,strans("%19I",s),(*R).L1[lv].Modif);
  }
  fprintf(stdout,"\n");
  
  fprintf(stdout,"LRrg.P:    ");
  for (lv= (*par).rootlvl; lv >= 0; lv--) {
    fprintf(stdout,strans("%19N",s),(*R).LRrg[lv].P);
  }
  fprintf(stdout,"\n");
  
  fprintf(stdout,"LRrg.E:    ");
  for (lv= (*par).rootlvl; lv >= 0; lv--) {
    fprintf(stdout,strans("%19I",s),(*R).LRrg[lv].E);
  }
  fprintf(stdout,"\n");
  
  fprintf(stdout,"#pages:    ");
  for (lv= (*par).rootlvl; lv >= 0; lv--) {
    fprintf(stdout,strans("%19N",s),(*par).PageCount[lv]);
  }
  fprintf(stdout,"\n");
  
  fprintf(stdout,"Level:     ");
  for (lv= (*par).rootlvl; lv >= 0; lv--) {
    fprintf(stdout,strans("%19I",s),lv);
  }
  fprintf(stdout,"\n");
  
  return TRUE;
}


/************************************************************************/

boolean DirLevelDump(t_RT r,
                     void *bufAdr,
                     Rint bufSize,
                     RectConvFunc Convert)

{
# define MX_NMB_STR_LN 2
# define NMB_RCTS_BFFRD 200
  
  RSTREE R= (RSTREE)r;
  char s[80];
  NamePath names;
  FilePath files;
  PtrPath buffers;
  char numberBuf[MX_NMB_STR_LN + 1];
  Rint rootlvl, lv;
  
  if (R == NULL) {
    setRSTerr(R,"DirLevelDump",RNULL);
    return FALSE;
  }
  
  (*R).RSTDone= TRUE;
  rootlvl= (*R).parameters._.rootlvl;
  if (rootlvl > 0) {
    for (lv= 1; lv <= rootlvl; lv++) {
      strlcpy(names[lv],(*R).mainname,sizeof(RSTName));
      strlcat(names[lv],".lv",sizeof(RSTName));
      sprintf(numberBuf,strans("%02I",s),lv);
      strlcat(names[lv],numberBuf,sizeof(RSTName));
      CreateTruncF(names[lv],&files[lv]);
      CreateFileBuf((t_FB *)&buffers[lv],bufSize,NMB_RCTS_BFFRD,files[lv]);
    }
    LvlDmp(R,rootlvl,bufAdr,bufSize,Convert,names,buffers);
    for (lv= 1; lv <= rootlvl; lv++) {
      if (! DisposeFileBuf((t_FB *)&buffers[lv])) {
        (*R).RSTDone= FALSE;
      }
      if (! CloseF(files[lv])) {
        (*R).RSTDone= FALSE;
      }
    }
  }
  return (*R).RSTDone;
  
# undef NMB_RCTS_BFFRD
# undef MX_NMB_STR_LN
}

/************************************************************************/

int RSTError(t_RT r)

{
  RSTREE R= (RSTREE)r;
  
  if (R != NULL) {
    return (*R).error;
  }
  else {
    return -1;
  }
}

/************************************************************************/
/* This function logically belongs to an RSTBase.c which does not exist(!).
   It is here due to differences in versions of its body, whereas its
   header is (currently) always the same, and located in RSTBase.h.
   RSTree.c and RSTBase.h follow the same version alteration. */
/***
      This function exactly has to conform with the type structure set in
      RSTBase.h!! Different for various versions!
***/

void SetOffsets(RSTREE R,
                Rint numbOfDim,
                Rint infoSize) {
  
  char s[256];
  Rint entOffset;
  
  /* dimensionality: */
  (*R).numbofdim= numbOfDim;
  
  /* sizes: */
  (*R).SIZEinfo= infoSize;
  (*R).SIZEpoint= numbOfDim * SIZEatomkey;
  (*R).SIZErect= numbOfDim * SIZEinterval;
  
  /* offsets in nodes: */
  entOffset= sizeof(typnodeinfo);
  
  /* Does the compiler behave as expected?: */
  if ((entOffset & SIZEatomkey - 1) != 0 || entOffset > SIZEatomkey) {
    fprintf(stderr,
    "RST: SetOffsets: WARNING: UNEXPECTED ALIGNMENT:\n");
    fprintf(stderr,
    strans("sizeof(typnodeinfo) = %I, should be %I\n",s),entOffset,SIZEatomkey);
  }
  
  AlignInt(&entOffset,SIZEatomkey);
  (*R).Pent0= entOffset;
  
  /* aligned lengths: */
  (*R).DIR.entLen= (*R).SIZErect + SIZEptrtosub;
  AlignInt(&(*R).DIR.entLen,SIZEatomkey);
  (*R).DATA.entLen= (*R).SIZErect + infoSize;
  AlignInt(&(*R).DATA.entLen,SIZEatomkey);
  
  (*R).pttsLen= (*R).DIR.entLen - (*R).SIZErect;
  (*R).infoLen= (*R).DATA.entLen - (*R).SIZErect;
  
  /* offsets in entries: */
  /*** (*R).entPrect= 0 ***/
  (*R).entPptts= (*R).SIZErect;
  (*R).entPinfo= (*R).SIZErect;
  
  /* offsets in DistHeapElem's (which again contain entries): */
  (*R).DHEPent0= sizeof(typDHEinfo);
  AlignInt(&(*R).DHEPent0,SIZEatomkey);
  
  /* aligned lengths: */
  (*R).DHELen= (*R).DHEPent0 + (*R).DATA.entLen;
  /*** There is only one DHELen, the possibly greater (DATA)!! ***/
}

/************************************************************************/

static void MakeDataEntry(RSTREE R,
                          const typinterval *rect,
                          refinfo info,
                          void *entry) {

  void *ptr;
  
  CopyRect((*R).numbofdim,rect,entry);
  ptr= entry;
  ptr+= (*R).SIZErect;
  memcpy(ptr,info,(*R).SIZEinfo);
}

/************************************************************************/