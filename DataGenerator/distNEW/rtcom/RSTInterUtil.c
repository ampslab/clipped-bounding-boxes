/* ----- RSTInterUtil.c ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//


#include "RSTInterUtil.h"
#include "RSTUtil.h"
#include "RSTPageInOut.h"
#include "RSTFilePageIO.h"
#include "RSTErrors.h"
#include "RSTFileAccess.h"
#include "RSTMemAlloc.h"
#include "RSTSupport.h"


/* types */

typedef union {
              Rint _;
              byte fill[16];
              } typRintCheck;
typedef union {
              Rfloat _;
              byte fill[16];
              } typRfloatCheck;
typedef struct {
               typRintCheck         RintCk;
               typRfloatCheck     RfloatCk;
               } typnumbcheck;


/* constants */

#define iCkConst 42
#define fCkConst 42.42
#define WordSizetypnumbcheck ((int) ((sizeof(typnumbcheck) / sizeof(typword))))

/* ----- declarations ----- */

static Rint MinHeight(Rpnint availpages, Rint fanout);
static void StoreRSTDescFile(RSTREE R);
static void ReConnectDataLink(RSTREE R);
static void ReConnectDirLinks(RSTREE R);
static Rint BlockAlignment(void);
static Rint PageSizeAlignment(void);
static void WriteRSTFileMap(RSTREE R, char *operat);


/************************************************************************/
/* callers CreateRST(), SaveRST() limit length of (*R).mainname */

void CreateRSFiles(RSTREE R)

{
  RSTName SufName;
  
  if (! CreateExclF((*R).mainname,&(*R).DIR.str.f)) {
    (*R).RSTDone= FALSE;
    return;
  }
  strlcpy(SufName,(*R).mainname,sizeof(SufName));
  strlcat(SufName,DATA_SUFF,sizeof(SufName));
  if (! CreateExclF(SufName,&(*R).DATA.str.f)) {
    (*R).RSTDone= FALSE;
    return;
  }
  strlcpy(SufName,(*R).mainname,sizeof(SufName));
  strlcat(SufName,DIR_PD_SUFF,sizeof(SufName));
  if (! CreateExclF(SufName,&(*R).DIR.PA.str.f)) {
    (*R).RSTDone= FALSE;
    return;
  }
  strlcpy(SufName,(*R).mainname,sizeof(SufName));
  strlcat(SufName,DATA_PD_SUFF,sizeof(SufName));
  if (! CreateExclF(SufName,&(*R).DATA.PA.str.f)) {
    (*R).RSTDone= FALSE;
    return;
  }
  StoreRSTDescFile(R);
  WriteRSTFileMap(R,"created");
}

/************************************************************************/
/* single caller InternalOpen() limits length of (*R).mainname */

void OpenRSFiles(RSTREE R)

{
  RSTName SufName;
    
  if (! OpenRdWrF((*R).mainname,&(*R).DIR.str.f)) {
    (*R).RSTDone= FALSE;
    return;
  }
  strlcpy(SufName,(*R).mainname,sizeof(SufName));
  strlcat(SufName,DATA_SUFF,sizeof(SufName));
  if (! OpenRdWrF(SufName,&(*R).DATA.str.f)) {
    (*R).RSTDone= FALSE;
    return;
  }
  strlcpy(SufName,(*R).mainname,sizeof(SufName));
  strlcat(SufName,DIR_PD_SUFF,sizeof(SufName));
  if (! OpenRdWrF(SufName,&(*R).DIR.PA.str.f)) {
    (*R).RSTDone= FALSE;
    return;
  }
  strlcpy(SufName,(*R).mainname,sizeof(SufName));
  strlcat(SufName,DATA_PD_SUFF,sizeof(SufName));
  if (! OpenRdWrF(SufName,&(*R).DATA.PA.str.f)) {
    (*R).RSTDone= FALSE;
    return;
  }
  WriteRSTFileMap(R,"opened");
}

/************************************************************************/

void FastCloseRSFiles(RSTREE R)

{
  CloseF((*R).DIR.str.f);
  CloseF((*R).DATA.str.f);
  CloseF((*R).DIR.PA.str.f);
  CloseF((*R).DATA.PA.str.f);
  WriteRSTFileMap(R,"closed");
}

/************************************************************************/

void CloseRSFiles(RSTREE R)

{
  if (! CloseF((*R).DIR.str.f)) {
    (*R).RSTDone= FALSE;
  }
  if (! CloseF((*R).DATA.str.f)) {
    (*R).RSTDone= FALSE;
  }
  if (! CloseF((*R).DIR.PA.str.f)) {
    (*R).RSTDone= FALSE;
  }
  if (! CloseF((*R).DATA.PA.str.f)) {
    (*R).RSTDone= FALSE;
  }
  WriteRSTFileMap(R,"closed");
}

/************************************************************************/

void InitRamDisc(RSTREE R)

{
  (*R).DIR.str.RAM.ptr= NULL;
  (*R).DATA.str.RAM.ptr= NULL;
  (*R).DIR.PA.str.RAM.ptr= NULL;
  (*R).DATA.PA.str.RAM.ptr= NULL;
  (*R).DIR.str.RAM.size= 0;
  (*R).DATA.str.RAM.size= 0;
  (*R).DIR.PA.str.RAM.size= 0;
  (*R).DATA.PA.str.RAM.size= 0;
}

/************************************************************************/
/* Used for initial allocation and re-allocation.
   ReAllocRamDisc() is NOT self re-initializing when failing.
   Reason: intact RAM disk required to allow e.g. SaveRST(). */
/* Was indeed buggy for the fail case up to version 7.0, but the bug was not
   effective, because the sole solution, failing to have enough main memory,
   is to save to secondary memory, which does not cause additional
   allocation. */

void ReAllocRamDisc(RSTREE R,
                    Rpint dirRAMsize,
                    Rpint dataRAMsize)

{
  void *dirptr, *dirPDptr, *dataptr, *dataPDptr;
  Rpnint lastdirpage, lastdatapage;
  Rpint dirPDsize, dataPDsize;
  
  if (dirRAMsize != (*R).DIR.str.RAM.size) {
    dirptr= reallocM((*R).DIR.str.RAM.ptr,dirRAMsize);
    if (dirptr == NULL) {
      setRSTerr(R,"ReAllocRamDisc (DIR RAM)",reAlloc);
      (*R).RSTDone= FALSE;
      return;
    }
    else {
      if ((*R).DIR.str.RAM.ptr != dirptr) {
        if ((*R).verboseRAMdiskExt && (*R).DIR.str.RAM.ptr != NULL) {
          fprintf(stderr," MOVING LOCATION\n");
        }
        (*R).DIR.str.RAM.ptr= dirptr;
        ReConnectDirLinks(R);
      }
      (*R).DIR.str.RAM.size= dirRAMsize;
    }
    lastdirpage= dirRAMsize / (*R).parameters._.DIR.pagelen - 1;
    dirPDsize= (lastdirpage / MAX_PG_NRS + 1 + 1) * SIZE_FIX_BL;
    /* one block extra for the parameter block */
    if (dirPDsize != (*R).DIR.PA.str.RAM.size) {
      dirPDptr= reallocM((*R).DIR.PA.str.RAM.ptr,dirPDsize);
      if (dirPDptr == NULL) {
        setRSTerr(R,"ReAllocRamDisc (DIR PD)",reAlloc);
        (*R).RSTDone= FALSE;
        return;
      }
      else {
        (*R).DIR.PA.str.RAM.ptr= dirPDptr;
        (*R).DIR.PA.str.RAM.size= dirPDsize;
      }
    }
  }
  
  if (dataRAMsize != (*R).DATA.str.RAM.size) {
    dataptr= reallocM((*R).DATA.str.RAM.ptr,dataRAMsize);
    if (dataptr == NULL) {
      setRSTerr(R,"ReAllocRamDisc (DATA RAM)",reAlloc);
      (*R).RSTDone= FALSE;
      return;
    }
    else {
      if ((*R).DATA.str.RAM.ptr != dataptr) {
        if ((*R).verboseRAMdiskExt && (*R).DATA.str.RAM.ptr != NULL) {
          fprintf(stderr,"MOVING LOCATION\n");
        }
        (*R).DATA.str.RAM.ptr= dataptr;
        ReConnectDataLink(R);
      }
      (*R).DATA.str.RAM.size= dataRAMsize;
    }
    lastdatapage= dataRAMsize / (*R).parameters._.DATA.pagelen - 1;
    dataPDsize= (lastdatapage / MAX_PG_NRS + 1 + 1) * SIZE_FIX_BL;
    /* one block extra for the parameter block */
    if (dataPDsize != (*R).DATA.PA.str.RAM.size) {
      dataPDptr= reallocM((*R).DATA.PA.str.RAM.ptr,dataPDsize);
      if (dataPDptr == NULL) {
        setRSTerr(R,"ReAllocRamDisc (DATA PD)",reAlloc);
        (*R).RSTDone= FALSE;
        return;
      }
      else {
        (*R).DATA.PA.str.RAM.ptr= dataPDptr;
        (*R).DATA.PA.str.RAM.size= dataPDsize;
      }
    }
  }
}

/************************************************************************/

void DeallocRamDisc(RSTREE R)

{
  freeM(&(*R).DIR.str.RAM.ptr);
  freeM(&(*R).DATA.str.RAM.ptr);
  freeM(&(*R).DIR.PA.str.RAM.ptr);
  freeM(&(*R).DATA.PA.str.RAM.ptr);
}

/************************************************************************/

void SetRAMdiscLimits(RSTREE R)

{
  refparameters par;
  refversion ver;
  Rpnint nofdirpages, nofdatapages, ndir, ndata;
  Rint maxdirsplit, maxdatasplit;
  
  par= &(*R).parameters._;
  ver= &(*R).version._;
  
  nofdirpages= (*R).DIR.str.RAM.size / (*par).DIR.pagelen;
  if ((nofdirpages & 1) == 1) {
    ndir= nofdirpages + 1;
  }
  else {
    ndir= nofdirpages;
  }
  maxdirsplit= ((*ver).DIR.reinsertqty + 1) * (MinHeight(nofdirpages,(*par).DIR.M) - 1) + 2;
  /* worst case for each non root level, plus 2 for the root */
  if (nofdirpages < 2 * maxdirsplit) {
    (*R).DIR.str.RAM.pagelimit= ndir / 2;
  }
  else {
    (*R).DIR.str.RAM.pagelimit= nofdirpages - maxdirsplit;
  }
  /* worst case if OverflowTreatment is propagated once per level */
  if ((*R).DIR.PA.pagedir._.number[0] >= (*R).DIR.str.RAM.pagelimit) {
    (*R).DIR.str.RAM.locked= TRUE;
  }
  else {
    (*R).DIR.str.RAM.locked= FALSE;
  }
  
  nofdatapages= (*R).DATA.str.RAM.size / (*par).DATA.pagelen;
  if ((nofdatapages & 1) == 1) {
    ndata= nofdatapages + 1;
  }
  else {
    ndata= nofdatapages;
  }
  maxdatasplit= (*ver).DATA.reinsertqty + 1;
  if (nofdatapages < 2 * maxdatasplit) {
    (*R).DATA.str.RAM.pagelimit= ndata / 2;
  }
  else {
    (*R).DATA.str.RAM.pagelimit= nofdatapages - maxdatasplit;
  }
  /* worst case */
  if ((*R).DATA.PA.pagedir._.number[0] >= (*R).DATA.str.RAM.pagelimit) {
    (*R).DATA.str.RAM.locked= TRUE;
  }
  else {
    (*R).DATA.str.RAM.locked= FALSE;
  }
}

/************************************************************************/

void HandleRamDiscLock(RSTREE R)

{
  char s[80];
  Rpint newRAMsize;
  
  if ((*R).DIR.str.RAM.locked) {
    if ((*R).DIR.str.RAM.size < LO_RAM_SIZE) {
      newRAMsize= (*R).DIR.str.RAM.size * HI_RAM_EXT_FAC;
    }
    else if ((*R).DIR.str.RAM.size > HI_RAM_SIZE) {
      newRAMsize= (*R).DIR.str.RAM.size * LO_RAM_EXT_FAC;
    }
    else {
      newRAMsize= (*R).DIR.str.RAM.size * STD_RAM_EXT_FAC;
    }
    if ((*R).verboseRAMdiskExt) {
      fprintf(stderr," DIR RAM DISK EXTENSION: ");
      fprintf(stderr,strans("%P --> %P\n",s),(*R).DIR.str.RAM.size,newRAMsize);
    }
    ReAllocRamDisc(R,newRAMsize,(*R).DATA.str.RAM.size);
    /* (*R).DATA.str.RAM.size unmodified */
    if (! (*R).RSTDone) {
      fprintf(stderr," DIR RAM DISK EXTENSION: FAILED\n");
      return;
    }
    else {
      if ((*R).verboseRAMdiskExt) {
        fprintf(stderr,"Done\n");
      }
    }
    SetRAMdiscLimits(R);
  }
  if ((*R).DATA.str.RAM.locked) {
    if ((*R).DATA.str.RAM.size < LO_RAM_SIZE) {
      newRAMsize= (*R).DATA.str.RAM.size * HI_RAM_EXT_FAC;
    }
    else if ((*R).DATA.str.RAM.size > HI_RAM_SIZE) {
      newRAMsize= (*R).DATA.str.RAM.size * LO_RAM_EXT_FAC;
    }
    else {
      newRAMsize= (*R).DATA.str.RAM.size * STD_RAM_EXT_FAC;
    }
    if ((*R).verboseRAMdiskExt) {
      fprintf(stderr,"DATA RAM DISK EXTENSION: ");
      fprintf(stderr,strans("%P --> %P\n",s),(*R).DATA.str.RAM.size,newRAMsize);
    }
    ReAllocRamDisc(R,(*R).DIR.str.RAM.size,newRAMsize);
    /* (*R).DIR.str.RAM.size unmodified */
    if (! (*R).RSTDone) {
      fprintf(stderr,"DATA RAM DISK EXTENSION: FAILED\n");
      return;
    }
    else {
      if ((*R).verboseRAMdiskExt) {
        fprintf(stderr,"Done\n");
      }
    }
    SetRAMdiscLimits(R);
  }
}

/************************************************************************/

static void ReConnectDataLink(RSTREE R)

{
  char s[80];
  
  if ((*R).L[0].N == NULL) {
    if ((*R).L[0].P != EMPTY_BL) {
      fprintf(stderr,"%s\n","WARNING");
      fprintf(stderr,"%s\n","ReConnectDataLink 1:");
      fprintf(stderr,"%s\n","(*R).L[0].N == NULL");
      fprintf(stderr,strans("%s %N\n",s),"(*R).L[0].P ==",(*R).L[0].P);
    }
  }
  else {
    GetNode(R,&(*R).L[0].N,(*R).L[0].P,0);
    
    /* COUNTS ADAPTION */
    /* adapt pri counts to lru, sec counts for the case of RAM extension: */
#ifndef COUNTS_OFF
    if ((*R).count.on) {
      (*R).count.DATA.get--;
    }
#endif
  }
}

/************************************************************************/

static void ReConnectDirLinks(RSTREE R)

{
  char s[80];
  Rint lv;
  
  for (lv= 1; lv <= (*R).parameters._.rootlvl; lv++) {
    if ((*R).L[lv].N == NULL) {
      if ((*R).L[lv].P != EMPTY_BL) {
        fprintf(stderr,"%s\n","WARNING");
        fprintf(stderr,"%s\n","ReConnectDirLinks 1:");
        fprintf(stderr,strans("%s%I%s %s\n",s),"(*R).L[",lv,"].N ==","NULL");
        fprintf(stderr,strans("%s%I%s %N\n",s),"(*R).L[",lv,"].P ==",(*R).L[lv].P);
      }
    }
    else {
      GetNode(R,&(*R).L[lv].N,(*R).L[lv].P,lv);
      
      /* COUNTS ADAPTION */
      /* adapt pri counts to lru, sec counts for the case of RAM extension: */
#ifndef COUNTS_OFF
      if ((*R).count.on) {
        (*R).count.DIR.get--;
      }
#endif
    }
  }
}

/************************************************************************/

static Rint MinHeight(Rpnint availpages, Rint fanout)

{
  /* in Rfloat to avoid overflow for availpages near MAX(Rint) */
  Rfloat ap= availpages;
  Rfloat fo= fanout;
  Rfloat nofnodes= 1.0;
  Rfloat nodesperlevel= 1.0;
  Rint height= 1;
  
  while (nofnodes < ap) {
    nodesperlevel*= fo;
    nofnodes+= nodesperlevel;
    height++;
  }
  return height;
}

/************************************************************************/

boolean CreateMMRSTIdentity(RSTREE *r, RSTREE sourceR)

{
  RSTREE R;
  
  if (*r != NULL) {
    /* assured by the caller, else FATAL INTERNAL ERROR there */
    return FALSE;
  }
  *r= allocM(sizeof(rstree));
  R= *r;
  if (R == NULL) {
    setRSTerr(R,"CreateMMRSTIdentity",reAlloc);
    return FALSE;
  }
  
  strlcpy((*R).mainname,MAIN_MEMORY,sizeof(RSTName));
  ResetStorageInfo(R,pri);
  (*R).RSTDone= TRUE;
  
  SetFixParamIOblocks(R);     /* (not really needed) */
  (*R).parameters= (*sourceR).parameters;
  SetOffsets(R,(*R).parameters._.numbofdim,(*R).parameters._.SIZEinfo);
  SetCheck(R,FALSE);
  if (! (*R).RSTDone) {
    freeM(r);
    return FALSE;
  }
  SetVersion(R);
  SetSizesPreAllocArrs(R);
  if (! (*R).RSTDone) {
    freeM(r);
    return FALSE;
  }
  (*R).DIR.str.RAM.locked= (*sourceR).DIR.str.RAM.locked;
  (*R).DATA.str.RAM.locked= (*sourceR).DATA.str.RAM.locked;
  (*R).DIR.str.RAM.ptr= (*sourceR).DIR.str.RAM.ptr;
  (*R).DATA.str.RAM.ptr= (*sourceR).DATA.str.RAM.ptr;
  (*R).DIR.PA.str.RAM.ptr= (*sourceR).DIR.PA.str.RAM.ptr;
  (*R).DATA.PA.str.RAM.ptr= (*sourceR).DATA.PA.str.RAM.ptr;
  
  InitBuffersFlags(R);
  SetVarDirDataIOblocks(R);
  InitRootAndCounting(R,TRUE);
  if (! (*R).RSTDone) {
    DeallocArrs(R);
    freeM(r);
    return FALSE;
  }
  return TRUE;
}

/************************************************************************/

void RemoveMMRSTIdentity(RSTREE *r)

{
  RSTREE R= *r;
  
  DeallocArrs(R);
  freeM(r);
}

/************************************************************************/

boolean OpenBufRSTIdentity(RSTREE *r, RSTREE sourceR)

{
  RSTREE R;
  
  /* Begin replace InternalOpen */
  if (*r != NULL) {
    /* assured by the caller, else FATAL INTERNAL ERROR there */
    return FALSE;
  }
  *r= allocM(sizeof(rstree));
  R= *r;
  if (R == NULL) {
    setRSTerr(R,"OpenBufRSTIdentity",reAlloc);
    return FALSE;
  }
  
  strlcpy((*R).mainname,(*sourceR).mainname,sizeof(RSTName));
  (*R).RSTDone= TRUE;
  
  /* due to compound page numbers, share the file identifiers: */
  (*R).DIR.str.f= (*sourceR).DIR.str.f;
  (*R).DIR.PA.str.f= (*sourceR).DIR.PA.str.f;
  (*R).DATA.str.f= (*sourceR).DATA.str.f;
  (*R).DATA.PA.str.f= (*sourceR).DATA.PA.str.f;
  
  SetFixParamIOblocks(R);
  ReadParamsPDs(R);
  SetOffsets(R,(*R).parameters._.numbofdim,(*R).parameters._.SIZEinfo);
  SetCheck(R,FALSE);
  if (! (*R).RSTDone) {
    freeM(r);
    return FALSE;
  }
  SetVersion(R);
  SetSizesPreAllocArrs(R);
  if (! (*R).RSTDone) {
    freeM(r);
    return FALSE;
  }
  InitBuffersFlags(R);
  SetVarDirDataIOblocks(R);
  /* End   replace InternalOpen */
  
  ResetStorageInfo(R,lru);
  (*R).LRU= (*sourceR).LRU;
  InitRootAndCounting(R,TRUE);
  if (! (*R).RSTDone) {
    DeallocArrs(R);
    freeM(r);
    return FALSE;
  }
  return TRUE;
}

/************************************************************************/

boolean CloseBufRSTIdentity(RSTREE *r)

{
  RSTREE R= *r;
  boolean Done;
  
  /* CloseBufRSTIdentity attempts to continue under error condition! */
  
  PutPath(R);
  
  /*** Begin internal error checks: ***/
  /* General LRU buffer consistency check: May be expensive!
     To be applied after code modifications only! */
  //Done= TRUE;
  //if (! LRUConsistent((*R).LRU,&Done)) {
  //  if (! Done) {
  //    setRSTerr(R,"CloseBufRSTIdentity",LRUconsistenceCheck);
  //  }
  //  else {
  //    (*R).RSTDone= FALSE;
  //  }
  //}
  /*** End   internal error checks ***/
  
  Done= (*R).RSTDone;
  DeallocArrs(R);
  freeM(r);
  return Done;
}

/************************************************************************/

boolean CloseRSTIdentity(RSTREE *r)

{
  RSTREE R= *r;
  boolean Done;
  
  /* CloseRSTIdentity attempts to continue under error condition! */
  
  PutPath(R);
  CloseRSFiles(R);
  if (! (*R).buflinked) { /* should be always TRUE! */
    DeallocBuffers(R);
  }
  
  Done= (*R).RSTDone;
  DeallocArrs(R);
  freeM(r);
  return Done;
}

/************************************************************************/

boolean InternalOpen(RSTREE *r,
                     const char *name,
                     boolean allocBuffers)

{
  RSTREE R= NULL;
  
  BasicCheck();
  
  if (*r != NULL) {
    setRSTerr(R,"InternalOpen",RnotNULL);
    return FALSE;
  }
  *r= allocM(sizeof(rstree));
  R= *r;
  if (R == NULL) {
    setRSTerr(R,"InternalOpen",reAlloc);
    return FALSE;
  }
  
  if (strlcpy((*R).mainname,name,sizeof(RSTName)) >= sizeof(RSTName) - SUFF_LEN) {
    setRSTerr(R,"InternalOpen",nameLenGtr);
    freeM(r);
    return FALSE;
  }
  /* NO ResetStorageInfo here */
  (*R).RSTDone= TRUE;
  
  OpenRSFiles(R);
  if (! (*R).RSTDone) {
    freeM(r);
    return FALSE;
  }
  
  SetFixParamIOblocks(R);
  ReadParamsPDs(R);
  SetOffsets(R,(*R).parameters._.numbofdim,(*R).parameters._.SIZEinfo);
  SetCheck(R,FALSE);
  if (! (*R).RSTDone) {
    FastCloseRSFiles(R);
    freeM(r);
    return FALSE;
  }
  SetVersion(R);
  SetSizesPreAllocArrs(R);
  if (! (*R).RSTDone) {
    FastCloseRSFiles(R);
    freeM(r);
    return FALSE;
  }
  InitBuffersFlags(R);
  if (allocBuffers) {
    AllocBuffers(R);
  }
  SetVarDirDataIOblocks(R);
  if (! (*R).RSTDone) {
    DeallocArrs(R);
    FastCloseRSFiles(R);
    if (allocBuffers) {
      DeallocBuffers(R);
    }
    freeM(r);
    return FALSE;
  }
  return TRUE;
}

/************************************************************************/

void CleanupCloseRST(RSTREE *r) {

  RSTREE R= *r;
  
  CloseRSFiles(R);
  UpdateRSTDescFile(R);
  if (! (*R).buflinked) {
    DeallocBuffers(R);
  }
  DeallocArrs(R);
  freeM(r);
}

/************************************************************************/

void SetBase(RSTREE R,
             Rint dirpagesize,
             Rint datapagesize,
             boolean unique)

{
  refparameters par;
  refpagedir dpd;
  Rint lv;
  
  par= &(*R).parameters._;
  
  (*par).SIZE_ptrtosub= SIZEptrtosub;                  /* const RSTBase      */
  (*par).SIZEinfo= (*R).SIZEinfo;                      /* creation param     */
  (*par).numbofdim= (*R).numbofdim;                    /* creation param     */
  (*par).unique= unique;                               /* creation param     */
  (*par).DIR.pagelen= dirpagesize;                     /* creation param     */
  (*par).DATA.pagelen= datapagesize;                   /* creation param     */
  (*par).DIR.entriesoffset= (*R).Pent0;                /* (creation param)   */
  (*par).DATA.entriesoffset= (*R).Pent0;               /* (creation param)   */
  (*par).DIR.entrylen= (*R).DIR.entLen;                /* creation param     */
  (*par).DATA.entrylen= (*R).DATA.entLen;              /* creation param     */
  
  /* ----- other checks ----- */
  if ((*R).numbofdim < 1) {
    setRSTerr(R,"SetBase",numbDimLss);
    (*R).RSTDone= FALSE;
    return;
  }
  if ((*R).numbofdim > MAX_DIM) {
    setRSTerr(R,"SetBase",numbDimGtr);
    (*R).RSTDone= FALSE;
    return;
  }
  
  if ((*R).SIZEinfo < (int)sizeof(Rpnint)) {
    setRSTerr(R,"SetBase",infoSizeLss);
    (*R).RSTDone= FALSE;
    return;
  }
  
  
  /* ----- check alignment of page lengths ----- */
  /* Main memory residing pages have to be properly aligned owing to pointers
     to the RAM disc. Because secondary memory residing pages may be loaded,
     we demand proper alignment of page lengths in general. */
  if (dirpagesize % PageSizeAlignment() != 0) {
    setRSTerr(R,"SetBase",dirPageSizeAlign);
    (*R).RSTDone= FALSE;
    return;
  }
  if (datapagesize % PageSizeAlignment() != 0) {
    setRSTerr(R,"SetBase",dataPageSizeAlign);
    (*R).RSTDone= FALSE;
    return;
  }
  
  /* ----- set directory level parameters ----- */
  (*par).DIR.M= ((*par).DIR.pagelen-(*par).DIR.entriesoffset) / (*par).DIR.entrylen;
  if ((*par).DIR.M < 3) {
    setRSTerr(R,"SetBase",dirMlss);
    (*R).RSTDone= FALSE;
    return;
  }
  if ((*par).DIR.M > MAX_ENTRIES) {
    setRSTerr(R,"SetBase",dirMgtr);
    (*R).RSTDone= FALSE;
    return;
  }
  (*par).DIR.EqvWords_Mp1= (*par).DIR.M / sizeof(typword) + 1;
  
  /* ----- set data level parameters ----- */
  (*par).DATA.M= ((*par).DATA.pagelen-(*par).DATA.entriesoffset) / (*par).DATA.entrylen;
  if ((*par).DATA.M < 1) {
    setRSTerr(R,"SetBase",dataMlss);
    (*R).RSTDone= FALSE;
    return;
  }
  if ((*par).DATA.M > MAX_ENTRIES) {
    setRSTerr(R,"SetBase",dataMgtr);
    (*R).RSTDone= FALSE;
    return;
  }
  (*par).DATA.EqvWords_Mp1= (*par).DATA.M / sizeof(typword) + 1;
  
  /* ----- set defaults ----- */
  (*par).rootlvl= 0;
  (*par).DIR.pagecount= 1;
  (*par).DATA.pagecount= 0;
  for (lv= 0; lv < PATH_RANGE; lv++) {
    (*par).PageCount[lv]= 0;
  }
  (*par).recordcount= 0;
  
  dpd= &(*R).DIR.PA.pagedir._;
  
  (*dpd).childnr= FIRST_PD_BL_NR;
  (*dpd).nofnumbers= 0;
  (*dpd).number[0]= ROOT_BL_NR;
  
  dpd= &(*R).DATA.PA.pagedir._;
  
  (*dpd).childnr= FIRST_PD_BL_NR;
  (*dpd).nofnumbers= 0;
  (*dpd).number[0]= ROOT_BL_NR;
}

/************************************************************************/

void SetVersion(RSTREE R)

{
  refparameters par;
  refversion ver;
  
  par= &(*R).parameters._;
  ver= &(*R).version._;
  
  (*ver).minfillpercent= MIN_FILL_PERCENT;              /* const InterUtil  */
  (*ver).DELminfillpercent= DEL_MIN_FILL_PERCENT;       /* const InterUtil  */
                                  /* DIR_MIN_ENTRY_QTY    (const InterUtil) */
                                  /* DATA_MIN_ENTRY_QTY   (const InterUtil) */
  (*ver).reinstpercent= REINSERT_PERCENT;               /* const InterUtil  */
  
  /* ----- set directory level parameters ----- */
  (*ver).DIR.m= (MIN_FILL_PERCENT * (*par).DIR.M + 50) / 100;
  if ((*ver).DIR.m < DIR_MIN_ENTRY_QTY) {
    (*ver).DIR.m= DIR_MIN_ENTRY_QTY;
  }
  (*ver).DIR.DELm= (DEL_MIN_FILL_PERCENT * (*par).DIR.M + 50) / 100;
  if ((*ver).DIR.DELm < DIR_MIN_ENTRY_QTY) {
    (*ver).DIR.DELm= DIR_MIN_ENTRY_QTY;
  }
  (*ver).DIR.reinsertqty= ((*ver).reinstpercent * (*par).DIR.M + 50) / 100;
  
  /* ----- set data level parameters ----- */
  (*ver).DATA.m= (MIN_FILL_PERCENT * (*par).DATA.M + 50) / 100;
  if ((*ver).DATA.m < DATA_MIN_ENTRY_QTY) {
    (*ver).DATA.m= DATA_MIN_ENTRY_QTY;
  }
  (*ver).DATA.DELm= (DEL_MIN_FILL_PERCENT * (*par).DATA.M + 50) / 100;
  if ((*ver).DATA.DELm < DATA_MIN_ENTRY_QTY) {
    (*ver).DATA.DELm= DATA_MIN_ENTRY_QTY;
  }
  (*ver).DATA.reinsertqty= ((*ver).reinstpercent * (*par).DATA.M + 50) / 100;
}

/************************************************************************/
/* Exceptionally type int in this function (as cast only).
   The size of size_t, the return value of sizeof(), varies with the size of
   void *. printf("%d",x) needs an int sized x.
*/

void BasicCheck()

{
  boolean AbortExecution= FALSE;
  
  if (sizeof(byte) != 1) {
    fprintf(stderr,"%s\n","RST: ##### FATAL ERROR #####");
    fprintf(stderr,"%s\n","BasicCheck 1:");
    fprintf(stderr,"%s\n","sizeof(byte) != 1");
    fprintf(stderr,"%s %4d\n","sizeof(byte):",(int)sizeof(byte));
    AbortExecution= TRUE;
  }
  if (sizeof(Rshort) != 2) {
    fprintf(stderr,"%s\n","RST: ##### FATAL ERROR #####");
    fprintf(stderr,"%s\n","BasicCheck 2:");
    fprintf(stderr,"%s\n","sizeof(Rshort) != 2");
    fprintf(stderr,"%s %4d\n","sizeof(Rshort):",(int)sizeof(Rshort));
    AbortExecution= TRUE;
  }
  if (sizeof(Rint) < 4) {
    fprintf(stderr,"%s\n","RST: ##### WARNING #####");
    fprintf(stderr,"%s\n","BasicCheck 3:");
    fprintf(stderr,"%s\n","sizeof(Rint) < 4");
    fprintf(stderr,"%s %4d\n","sizeof(Rint):",(int)sizeof(Rint));
  }
  if (sizeof(Rlint) < 6) {
    fprintf(stderr,"%s\n","RST: ##### WARNING #####");
    fprintf(stderr,"%s\n","BasicCheck 4:");
    fprintf(stderr,"%s\n","sizeof(Rlint) < 6");
    fprintf(stderr,"%s %4d\n","sizeof(Rlint):",(int)sizeof(Rlint));
  }
  if (sizeof(Rpint) < sizeof(void *)) {
    fprintf(stderr,"%s\n","RST: ##### WARNING #####");
    fprintf(stderr,"%s\n","BasicCheck 5:");
    fprintf(stderr,"%s\n","sizeof(Rpint) < sizeof(void *)");
    fprintf(stderr,"%s %4d\n"," sizeof(Rpint):",(int)sizeof(Rpint));
    fprintf(stderr,"%s %4d\n","sizeof(void *):",(int)sizeof(void *));
  }
  if (sizeof(Rpnint) < 4) {
    fprintf(stderr,"%s\n","RST: ##### FATAL ERROR #####");
    fprintf(stderr,"%s\n","BasicCheck 6:");
    fprintf(stderr,"%s\n","sizeof(Rpnint) < 4");
    fprintf(stderr,"%s %4d\n","sizeof(Rpnint):",(int)sizeof(Rpnint));
    AbortExecution= TRUE;
  }
  if (sizeof(typpagedir) > sizeof(typfixblock)) {
    fprintf(stderr,"%s\n","RST: ##### FATAL ERROR #####");
    fprintf(stderr,"%s\n","BasicCheck 7:");
    fprintf(stderr,"%s\n","sizeof(typpagedir) > sizeof(typfixblock)");
    fprintf(stderr,"%s %4d\n"," sizeof(typpagedir):",(int)sizeof(typpagedir));
    fprintf(stderr,"%s %4d\n","sizeof(typfixblock):",(int)sizeof(typfixblock));
    AbortExecution= TRUE;
  }
  if (sizeof(typparameters) > sizeof(typfixblock)) {
    fprintf(stderr,"%s\n","RST: ##### FATAL ERROR #####");
    fprintf(stderr,"%s\n","BasicCheck 8:");
    fprintf(stderr,"%s\n","sizeof(typparameters) > sizeof(typfixblock)");
    fprintf(stderr,"%s %4d\n","sizeof(typparameters):",(int)sizeof(typparameters));
    fprintf(stderr,"%s %4d\n","  sizeof(typfixblock):",(int)sizeof(typfixblock));
    AbortExecution= TRUE;
  }
  if (AbortExecution) {
    abort();
  }
}

/************************************************************************/

void SetCheck(RSTREE R, boolean creation)

{
          /* Compatibility checks */
  
  char s[80];
  refparameters par= &(*R).parameters._;
  
  if (creation) {					/* set: */
    /* set for directory nodes: */
    /* set for data nodes: */
  }
  else {						/* check: */
    /* check basics: */
    /* numbofdim, SIZEinfo: simply fetched. */
    /* check directory nodes: */
    if ((*par).DIR.entriesoffset != (*R).Pent0) {
      fprintf(stderr,"%s %s\n","RST: ERROR:","incompatible tree file:");
      fprintf(stderr,strans("%s %4I\n",s),"       entriesoffset:",(*par).DIR.entriesoffset);
      fprintf(stderr,strans("%s %4I\n",s),"           Expecting:",(*R).Pent0);
      (*R).RSTDone= FALSE;
    }
    if ((*par).DIR.entrylen != (*R).DIR.entLen) {
      fprintf(stderr,"%s %s\n","RST: ERROR:","incompatible tree file:");
      fprintf(stderr,strans("%s %4I\n",s),"        DIR.entrylen:",(*par).DIR.entrylen);
      fprintf(stderr,strans("%s %4I\n",s),"           Expecting:",(*R).DIR.entLen);
      (*R).RSTDone= FALSE;
    }
    /* check data nodes: */
    /*** Although the offset is the same in directory and data nodes,
         parameters contain it separately in .DIR and .DATA.
         The check for data nodes is discarded.
    if ((*par).DATA.entriesoffset != (*R).Pent0) {
      fprintf(stderr,"%s %s\n","RST: ERROR:","incompatible tree file:");
      fprintf(stderr,strans("%s %4I\n",s),"  DATA.entriesoffset:",(*par).DATA.entriesoffset);
      fprintf(stderr,strans("%s %4I\n",s),"           Expecting:",(*R).Pent0);
      (*R).RSTDone= FALSE;
    }
    ***/
    if ((*par).DATA.entrylen != (*R).DATA.entLen) {
      fprintf(stderr,"%s %s\n","RST: ERROR:","incompatible tree file:");
      fprintf(stderr,strans("%s %4I\n",s),"       DATA.entrylen:",(*par).DATA.entrylen);
      fprintf(stderr,strans("%s %4I\n",s),"           Expecting:",(*R).DATA.entLen);
      (*R).RSTDone= FALSE;
    }
  }
  							/* set anyway: */
  /* node buffer lengths: */
  (*R).DIR.nodeLen= 2 * (*par).DIR.pagelen;
  (*R).DATA.nodeLen= 2 * (*par).DATA.pagelen;
  /* sufficient for the deletion-by-reinsertion version:
  (*R).DIR.nodeLen= (*par).pagelen + (*par).DIR.entrylen;
  (*R).DATA.nodeLen= (*par).pagelen + (*par).DATA.entrylen;
  */
  if ((*R).DATA.nodeLen > (*R).DIR.nodeLen) {
    (*R).UNInodeLen= (*R).DATA.nodeLen;
  }
  else {
    (*R).UNInodeLen= (*R).DIR.nodeLen;
  }
  /* initial error number: */
  (*R).error= noError;
}

/*** ------------------------ VERSIONCONTROL ------------------------ ***/
/* see "sufficient for the deletion-by-reinsertion version" above       */
/************************************************************************/

static Rint BlockAlignment(void)

{
#define MAX_ALIGN 16
/* Newer standardization (64-bit-pointer-compilation) requires the start of a
   block (newly allocated memory space) to be 16-aligned. Apart from that,
   memory spaces for smaller variables have to be aligned to their size.
   Example:
   sizeof(double) = 8  ==>  double alignment = 8. */
   
  Rint blockAlign= MAX_ALIGN;
  return blockAlign;

#undef MAX_ALIGN
}

/************************************************************************/

static Rint PageSizeAlignment(void)

{
  /* See BlockAlignment! Demands from there may be released if our
     greatest entities are smaller. */
   
  Rint pageSizeAlign= sizeof(Rpint);     /* at least */
  
  if (SIZEatomkey > pageSizeAlign) {
    pageSizeAlign= SIZEatomkey;
  }
  return pageSizeAlign;
}

/************************************************************************/

void InitBuffersFlags(RSTREE R)

{
  Rint lv;
  
  (*R).helpnode= NULL;
  
  for (lv= 0; lv < PATH_RANGE; lv++) {
    (*R).L[lv].N= NULL;
    (*R).L[lv].P= EMPTY_BL;
    (*R).L[lv].E= -1;
    (*R).L[lv].Modif= FALSE;
    (*R).L[lv].ReInsert= FALSE;
    /*   L: initialize the complete struct typlevel */
    
    (*R).LDel[lv].N= NULL;
    /*   LDel: initialize the parts of struct typlevel needed */
    
    (*R).L1[lv].N= NULL;
    (*R).L1[lv].P= EMPTY_BL;
    (*R).L1[lv].Modif= FALSE;
    /*   L1: initialize the parts of struct typlevel needed */
    
    (*R).LRrg[lv].P= EMPTY_BL;
    (*R).LRrg[lv].E= -1;
    /*   LRrg: initialize the complete struct typpagent */
  }
}

/************************************************************************/

void InitFlags(RSTREE R) {

  Rint lv;
  
  for (lv= 0; lv < PATH_RANGE; lv++) {
    (*R).L[lv].P= EMPTY_BL;
    (*R).L[lv].E= -1;
    (*R).L[lv].Modif= FALSE;
    (*R).L[lv].ReInsert= FALSE;
    /*   L: initialize all flags of struct typlevel */
    
    (*R).L1[lv].P= EMPTY_BL;
    (*R).L1[lv].Modif= FALSE;
    /*   L1: initialize the flags of struct typlevel needed */
    
    (*R).LRrg[lv].P= EMPTY_BL;
    (*R).LRrg[lv].E= -1;
    /*   LRrg: initialize the complete struct typpagent */
  }
}

/************************************************************************/

void AllocBuffers(RSTREE R)

{
  Rint lv;
  Rint rootlvl= (*R).parameters._.rootlvl;
  
  /* nodes in (*R).L[0 .. rootlvl] are pre-allocated: */
  (*R).L[0].N= allocM((*R).DATA.nodeLen);
  for (lv= 1; lv <= rootlvl; lv++) {
    (*R).L[lv].N= allocM((*R).DIR.nodeLen);
  }
  
  /* nodes in (*R).L1[0 .. rootlvl-1] are pre-allocated: */
  (*R).L1[0].N= allocM((*R).DATA.nodeLen);
  for (lv= 1; lv < rootlvl; lv++) {
    (*R).L1[lv].N= allocM((*R).DIR.nodeLen);
  }
  
  /* allocate universal nodes with the maximum possible size: */
  if ((*R).DIR.nodeLen > (*R).DATA.nodeLen) {
    (*R).helpnode= allocM((*R).DIR.nodeLen);
  }
  else {
    (*R).helpnode= allocM((*R).DATA.nodeLen);
  }
  
  if ((*R).helpnode == NULL) {
    setRSTerr(R,"AllocBuffers",reAlloc);
    (*R).RSTDone= FALSE;
  }
  //else {
  //  the preceding allocations should have been successful too.
  //}
}

/************************************************************************/

void DeallocBuffers(RSTREE R)

{
  Rint lv;
  Rint rootlvl= (*R).parameters._.rootlvl;
  
  for (lv= 0; lv <= rootlvl; lv++) {
    freeM(&(*R).L[lv].N);
  }
  
  for (lv= 0; lv < rootlvl; lv++) {
    freeM(&(*R).L1[lv].N);
  }
  freeM(&(*R).helpnode);
}

/************************************************************************/

void AdaptPathAlloc(RSTREE R, Rint OLDrootlvl) {

  Rint lv;
  Rint rootlvl= (*R).parameters._.rootlvl;
  
  if (rootlvl == OLDrootlvl) {
    /* nix */
  }
  else {
    if (rootlvl < OLDrootlvl) {
      for (lv= rootlvl + 1; lv <= OLDrootlvl; lv++) {
        freeM(&(*R).L[lv].N);
      }
      for (lv= rootlvl; lv < OLDrootlvl; lv++) {
        freeM(&(*R).L1[lv].N);
      }
    }
    else { // rootlvl > OLDrootlvl
      for (lv= OLDrootlvl + 1; lv <= rootlvl; lv++) {
        (*R).L[lv].N= allocM((*R).DIR.nodeLen);
      }
      for (lv= OLDrootlvl; lv < rootlvl; lv++) {
        (*R).L1[lv].N= allocM((*R).DIR.nodeLen);
      }
      if ((*R).L1[rootlvl-1].N == NULL) {
        setRSTerr(R,"AdaptPathAlloc",reAlloc);
        (*R).RSTDone= FALSE;
      }
      //else {
      //  the preceding allocations should have been successful too.
      //}
    }
  }
}

/************************************************************************/

void WriteParamsPDs(RSTREE R)

{
  WritePage(R,&(*R).DIR.PA.str,PARAM_BL_NR,&(*R).parameters);
  WritePage(R,&(*R).DIR.PA.str,FIRST_PD_BL_NR,&(*R).DIR.PA.pagedir);
  WritePage(R,&(*R).DATA.PA.str,PARAM_BL_NR,&(*R).version);
  // (*R).version not read back: hardcoded
  WritePage(R,&(*R).DATA.PA.str,FIRST_PD_BL_NR,&(*R).DATA.PA.pagedir);
}

/************************************************************************/

void ReadParamsPDs(RSTREE R)

{
  ReadPage(R,&(*R).DIR.PA.str,PARAM_BL_NR,&(*R).parameters);
  ReadPage(R,&(*R).DIR.PA.str,FIRST_PD_BL_NR,&(*R).DIR.PA.pagedir);
  // (*R).version not read back: hardcoded
  ReadPage(R,&(*R).DATA.PA.str,FIRST_PD_BL_NR,&(*R).DATA.PA.pagedir);
}

/************************************************************************/
/* Pure function: does not count (admin. writes). Unused. */

void WriteParamsPDsPure(RSTREE R)

{
  WrPage((*R).DIR.PA.str.f,(*R).DIR.PA.str.psize,PARAM_BL_NR,&(*R).parameters);
  WrPage((*R).DIR.PA.str.f,(*R).DIR.PA.str.psize,FIRST_PD_BL_NR,&(*R).DIR.PA.pagedir);
  WrPage((*R).DATA.PA.str.f,(*R).DATA.PA.str.psize,PARAM_BL_NR,&(*R).version);
  // (*R).version not read back: hardcoded
  WrPage((*R).DATA.PA.str.f,(*R).DATA.PA.str.psize,FIRST_PD_BL_NR,&(*R).DATA.PA.pagedir);
}

/************************************************************************/
/* Pure function: does not count (admin. reads). Unused. */

void ReadParamsPDsPure(RSTREE R)

{
  RdPage((*R).DIR.PA.str.f,(*R).DIR.PA.str.psize,PARAM_BL_NR,&(*R).parameters);
  RdPage((*R).DIR.PA.str.f,(*R).DIR.PA.str.psize,FIRST_PD_BL_NR,&(*R).DIR.PA.pagedir);
  // (*R).version not read back: hardcoded
  RdPage((*R).DATA.PA.str.f,(*R).DATA.PA.str.psize,FIRST_PD_BL_NR,&(*R).DATA.PA.pagedir);
}

/************************************************************************/

void SetFixParamIOblocks(RSTREE R)

{
  (*R).DIR.PA.str.psize= SIZE_FIX_BL;
  (*R).DATA.PA.str.psize= SIZE_FIX_BL;
}

/************************************************************************/

void SetVarDirDataIOblocks(RSTREE R)

{
  (*R).DIR.str.psize= (*R).parameters._.DIR.pagelen;
  (*R).DATA.str.psize= (*R).parameters._.DATA.pagelen;
}

/************************************************************************/
/* SyncPath synchronizes the path with the storage medium but, in case of
   LRU buffering, does not unlink the references!
   SyncPath called: at the entrance of twice open Joins and Distance Queries.
*/
      
void SyncPath(RSTREE R)

{
  Rint lv;
  
  for (lv= 0; lv <= (*R).parameters._.rootlvl; lv++) {
    if ((*R).L[lv].Modif) {
      PutGetNode(R,&(*R).L[lv].N,(*R).L[lv].P,lv);
      (*R).L[lv].Modif= FALSE;
    }
  }
}

/************************************************************************/
/* PutPath synchronizes the path with the storage medium, in case of LRU
   buffering also unlinks the references!
   PutPath called: in CloseRST, CloseRSTIdentity, CloseBufRSTIdentity.
*/
      
void PutPath(RSTREE R)

{
  Rint lv;
  
  for (lv= 0; lv <= (*R).parameters._.rootlvl; lv++) {
    if ((*R).L[lv].Modif) {
      PutNode(R,(*R).L[lv].N,(*R).L[lv].P,lv);
      (*R).L[lv].Modif= FALSE;
    }
    else {
      UnlinkPage(R,(*R).L[lv].P,lv);
    }
  }
}

/************************************************************************/

void InitCount(RSTREE R)

{
  refcount c;
  
  c= &(*R).count;
  (*c).on= FALSE;
  
  /* reading: */
  (*c).DIR.demand= 0; (*c).DATA.demand= 0;
  (*c).DIR.get= 0; (*c).DATA.get= 0;
  (*R).DIR.str.cnt.read= 0; (*R).DATA.str.cnt.read= 0;
  
  /* writing: */
  (*c).DIR.put= 0; (*c).DATA.put= 0;
  (*R).DIR.str.cnt.write= 0; (*R).DATA.str.cnt.write= 0;
  
  /* comparing: */
  (*c).DIR.comp= 0; (*c).DATA.comp= 0;
  
  /* distance query: */
  (*c).DQ_PQelems= 0;
  
  /* LRU buffer counting facility: */
  if ((*R).storkind == lru) {
    LRUCountsOn0((*R).LRU);
    LRUCountsOff((*R).LRU);
  }
  
  /* administrational reading, writing: */
  (*R).DIR.PA.str.cnt.read= 0; (*R).DATA.PA.str.cnt.read= 0;
  (*R).DIR.PA.str.cnt.write= 0; (*R).DATA.PA.str.cnt.write= 0;
  
  /* split, merge, reinsertion: */
  (*c).DIR.overflow= 0; (*c).DATA.overflow= 0;
  (*c).DIR.underflow= 0; (*c).DATA.underflow= 0;
  (*c).DIR.reinst= 0; (*c).DATA.reinst= 0;
  (*c).DIR.split= 0; (*c).DATA.split= 0;
  (*c).DIR.S_Area0= 0; (*c).DATA.S_Area0= 0;
  
  /* choose subtree: */
  (*c).CS_Call= 0;
  (*c).CS_NoFit= 0;
  (*c).CS_UniFit= 0;
  (*c).CS_SomeFit= 0;
  (*c).CS_OvlpEnlOpt= 0;
  (*c).CS_P= 0;
  (*c).CS_MaxP= 0;
  (*c).CS_PminusQ= 0;
  (*c).CS_OvlpEnlComput= 0;
  (*c).CS_P1OvlpEnl0= 0;
  (*c).CS_AfterwOvlpEnl0= 0;
  (*c).CS_Area0= 0;
}

/************************************************************************/

void JoinJoinCounts(RSTREE mainR, RSTREE secR)

{
  refcount mc= &(*mainR).count;
  refcount sc= &(*secR).count;
  
  /* reading: */
  (*mc).DIR.demand+= (*sc).DIR.demand;
  (*mc).DATA.demand+= (*sc).DATA.demand;
  (*mc).DIR.get+= (*sc).DIR.get;
  (*mc).DATA.get+= (*sc).DATA.get;
  (*mainR).DIR.str.cnt.read+= (*secR).DIR.str.cnt.read;
  (*mainR).DATA.str.cnt.read+= (*secR).DATA.str.cnt.read;
  
  /* writing: */
  (*mc).DIR.put+= (*sc).DIR.put;
  (*mc).DATA.put+= (*sc).DATA.put;
  (*mainR).DIR.str.cnt.write+= (*secR).DIR.str.cnt.write;
  (*mainR).DATA.str.cnt.write+= (*secR).DATA.str.cnt.write;
  
  /* comparing: */
  (*mc).DIR.comp+= (*sc).DIR.comp;
  (*mc).DATA.comp+= (*sc).DATA.comp;
}

/************************************************************************/

static void StoreRSTDescFile(RSTREE R)

{
  RSTName name;
  Rint subtreePtrSize, infoSize, numbOfDimensions,
       dirPageSize, dataPageSize, netDirPageSize, netDataPageSize,
       dirEntrySize, dataEntrySize,
       maxDirFanout, maxDataFanout, minDirFanout, minDataFanout,
       minDirDELrest, minDataDELrest;
  Rpnint numbOfDirPages, numbOfDataPages;
  Rlint numbOfRecords;
  Rint rootLevel;
  boolean unique;
  RpnintPath pagesPerLevel;
  boolean success;
  
  RSTName SufName;
  char mode[4]= "a";          /* append */
  /* char mode[4]= "w"; */ /* overwrite */
  FILE *desc;
  Rint clres;
  
  struct timeval tv;
  
  typnumbcheck nc;
  
  char s[80];
  Rint lv;
  
  success= InquireRSTDesc((t_RT)R,
                          name,
                          &subtreePtrSize,
                          &infoSize,
                          &numbOfDimensions,
                          &dirPageSize,
                          &dataPageSize,
                          &netDirPageSize,
                          &netDataPageSize,
                          &dirEntrySize,
                          &dataEntrySize,
                          &maxDirFanout,
                          &maxDataFanout,
                          &minDirFanout,
                          &minDataFanout,
                          &minDirDELrest,
                          &minDataDELrest,
                          &numbOfDirPages,
                          &numbOfDataPages,
                          &numbOfRecords,
                          &rootLevel,
                          &unique,
                          pagesPerLevel);
  if (! success) {
    setRSTerr(R,"StoreRSTDescFile",inquireDesc);
    (*R).RSTDone= FALSE;
    return;
  }
  strlcpy(SufName,name,sizeof(SufName));
  strlcat(SufName,DESC_SUFF,sizeof(SufName));
  
  desc= fopen(SufName,mode);
  if (desc == NULL) {
    perror("RSTree.StoreRSTDescFile: ERROR: OS");
    (*R).RSTDone= FALSE;
    return;
  }
  
  gettimeofday(&tv,NULL);
  
  ClearVector((typword *)&nc,WordSizetypnumbcheck);
  nc.RintCk._=        iCkConst;
  nc.RfloatCk._=      fCkConst;
  
  /* Write nc binary first: */
  fwrite(&nc,sizeof(typnumbcheck),1,desc);
  
  /* Continue with characters: */
  fprintf(desc,"\n");
  fprintf(desc,"-----------------------------------------------------------\n");
  // fprintf(desc,ctime(&tv.tv_sec)); // replaced:
  // For systems like Windows, not maintaining a pointer compatible
  // long, the following substitution is necessary:
  time_t tv_sec= tv.tv_sec;
  fprintf(desc,ctime(&tv_sec));
  fprintf(desc," Document Name:  \"%s\"\n",SufName);
  fprintf(desc,"Description of:  \"%s\"\n",name);
  fprintf(desc,"========================================\n");
  fprintf(desc,
  "This file is created by the functions CreateRST and SaveRST and is\n");
  fprintf(desc,
  "extended with current state data by the function CloseRST.\n");
  fprintf(desc,
  "The file begins with a sequence of non ASCII but plain integer and\n");
  fprintf(desc,
  "floating point numbers, 16 bytes aligned, as follows:\n");
  fprintf(desc,"  Rint representation of %d\n",iCkConst);
  fprintf(desc,"Rfloat representation of %5.2f\n",fCkConst);
  fprintf(desc,
  "See also SYS_BUILD constants and types in RSTTypes.h and RSTStdTypes.h.\n");
  fprintf(desc,
  "The built in function ExamRSTDescFile includes a number representation\n");
  fprintf(desc,
  "compatibility check.\n");
  
  /* Write label "#": */
  fprintf(desc,"#\n");
  
  /* Write reamainder: */
  fprintf(desc,strans("%20s%I\n",s),"subtreePtrSize: ",subtreePtrSize);
  fprintf(desc,strans("%20s%I\n",s),"infoSize: ",infoSize);
  fprintf(desc,strans("%20s%I\n",s),"numbOfDimensions: ",numbOfDimensions);
  fprintf(desc,strans("%20s%I\n",s),"dirPageSize: ",dirPageSize);
  fprintf(desc,strans("%20s%I\n",s),"dataPageSize: ",dataPageSize);
  fprintf(desc,strans("%20s%I\n",s),"netDirPageSize: ",netDirPageSize);
  fprintf(desc,strans("%20s%I\n",s),"netDataPageSize: ",netDataPageSize);
  fprintf(desc,strans("%20s%I\n",s),"dirEntrySize: ",dirEntrySize);
  fprintf(desc,strans("%20s%I\n",s),"dataEntrySize: ",dataEntrySize);
  fprintf(desc,strans("%20s%I\n",s),"maxDirFanout: ",maxDirFanout);
  fprintf(desc,strans("%20s%I\n",s),"maxDataFanout: ",maxDataFanout);
  fprintf(desc,strans("%20s%I\n",s),"minDirFanout: ",minDirFanout);
  fprintf(desc,strans("%20s%I\n",s),"minDataFanout: ",minDataFanout);
  fprintf(desc,strans("%20s%I\n",s),"minDirDELrest: ",minDirDELrest);
  fprintf(desc,strans("%20s%I\n",s),"minDataDELrest: ",minDataDELrest);
  fprintf(desc,strans("%20s%N\n",s),"numbOfDirPages: ",numbOfDirPages);
  fprintf(desc,strans("%20s%N\n",s),"numbOfDataPages: ",numbOfDataPages);
  fprintf(desc,strans("%20s%L\n",s),"numbOfRecords: ",numbOfRecords);
  fprintf(desc,strans("%20s%I\n",s),"rootLevel: ",rootLevel);
  fprintf(desc,"%20s%d\n","unique: ",unique);
  fprintf(desc,"pages per level:\n");
  for(lv= rootLevel; lv >= 0; lv--) {
    fprintf(desc,strans("%10N",s),pagesPerLevel[lv]);
  }
  fprintf(desc,"\n");
  fprintf(desc,"\n");
  clres= fclose(desc);
  if (clres != 0) {
    perror("RSTree.StoreRSTDescFile: ERROR: OS");
    (*R).RSTDone= FALSE;
    return;
  }
}

/************************************************************************/

boolean ReadRSTDescFile(const char *name)

{
  char mode[4]= "r"; /* read only */
  FILE *desc;
  Rint clres, ni;
  
  typnumbcheck nc;
  boolean numbersOK= TRUE;
  boolean sizesOK= TRUE;
  
  char s[80];
  char ch;
  int read_in; /* scanf("%d") reads an int */

  desc= fopen(name,mode);
  if (desc == NULL) {
    perror("RSTree.ReadRSTDescFile: ERROR: OS");
    return FALSE;
  }
  
  /* Read binary written nc first: */
  ni= fread(&nc,sizeof(typnumbcheck),1,desc);
  if (ni != 1) {
    perror("RSTree.ReadRSTDescFile: ERROR: OS");
    return FALSE;
  }
  
  /* Continue reading char by char until label "#": */
  do {
    ch= fgetc(desc);
    putc(ch,stdout);
  } while (ch != '#' && ch != EOF);
  fprintf(stdout,"\n");
  
  /* Check nc: */
  if (nc.RintCk._ != iCkConst) {
    fprintf(stdout,strans("   --> FAILING to read   Rint, reading: %I\n",s),nc.RintCk._);
    fprintf(stdout,"   -->               Originally stored: %d\n\n",iCkConst);
    numbersOK= FALSE;
  }
  if (nc.RfloatCk._ != fCkConst) {
    fprintf(stdout,strans("   --> FAILING to read Rfloat, reading: %f\n",s),nc.RfloatCk._);
    fprintf(stdout,"   -->               Originally stored: %5.2f\n\n",fCkConst);
    numbersOK= FALSE;
  }
  if (numbersOK) {
    fprintf(stdout,"                          %s","Number Representation Check: ok\n");
  }
  
  /* Check or read explicitly further things: */
  /* CAUTION: anonymous read_in compared with fixed values (succession!) */
  ni= fscanf(desc,"%s%d",s,&read_in); fprintf(stdout,"%19s %d",s,read_in);
  if (read_in != SIZEptrtosub) {
    fprintf(stdout,strans("%30s%d",s),"  DIFFERENCE!  Prg: SIZEptrtosub: ",SIZEptrtosub);
    sizesOK= FALSE;
  }
  fprintf(stdout,"\n");
  /* SIZEinfo: */
  ni= fscanf(desc,"%s%d",s,&read_in); fprintf(stdout,"%19s %d",s,read_in);
  fprintf(stdout,"\n");
  fprintf(stdout,"\n");
  /* numbOfDim: */
  ni= fscanf(desc,"%s%d",s,&read_in); fprintf(stdout,"%19s %d",s,read_in);
  fprintf(stdout,"\n");
  
  /* Read remainder char by char: */
  /* First character read/put by fgetc/putc: "\n" */
  for (;;) {
    ch= fgetc(desc);
    if (ch != EOF) {
      putc(ch,stdout);
    }
    else {
      break;
    }
  }
  
  clres= fclose(desc);
  if (clres != 0) {
    perror("RSTree.ReadRSTDescFile: ERROR: OS");
    return FALSE;
  }
  return TRUE;
}

/************************************************************************/

void UpdateRSTDescFile(RSTREE R)

{
  RSTName name;
  Rpnint numbOfDirPages, numbOfDataPages;
  Rlint numbOfRecords;
  Rint rootLevel;
  boolean unique;
  RpnintPath pagesPerLevel;
  boolean success;
  
  RSTName SufName;
  char mode[4]= "a";          /* append */
  /* char mode[4]= "w"; */ /* overwrite */
  FILE *desc;
  Rint clres;
  
  struct timeval tv;
  
  char s[80];
  Rint lv;
  
  success= GetVarRSTDesc((t_RT)R,
                         name,
                         &numbOfDirPages,
                         &numbOfDataPages,
                         &numbOfRecords,
                         &rootLevel,
                         &unique,
                         pagesPerLevel);
  if (! success) {
    setRSTerr(R,"UpdateRSTDescFile",getVarDesc);
    (*R).RSTDone= FALSE;
    return;
  }
  strlcpy(SufName,name,sizeof(SufName));
  strlcat(SufName,DESC_SUFF,sizeof(SufName));
  desc= fopen(SufName,mode);
  if (desc == NULL) {
    perror("RSTree.UpdateRSTDescFile: ERROR: OS");
    (*R).RSTDone= FALSE;
    return;
  }
  
  gettimeofday(&tv,NULL);
  
  fprintf(desc,"-----------------------------------------------------------\n");
  // fprintf(desc,ctime(&tv.tv_sec)); // replaced:
  // For systems like Windows, not maintaining a pointer compatible
  // long, the following substitution is necessary:
  time_t tv_sec= tv.tv_sec;
  fprintf(desc,ctime(&tv_sec));
  fprintf(desc,"%20s%s\n","name: ",name);
  fprintf(desc,strans("%20s%N\n",s),"numbOfDirPages: ",numbOfDirPages);
  fprintf(desc,strans("%20s%N\n",s),"numbOfDataPages: ",numbOfDataPages);
  fprintf(desc,strans("%20s%L\n",s),"numbOfRecords: ",numbOfRecords);
  fprintf(desc,strans("%20s%I\n",s),"rootLevel: ",rootLevel);
  fprintf(desc,"%20s%d\n","unique: ",unique);
  fprintf(desc,"pages per level:\n");
  for(lv= rootLevel; lv >= 0; lv--) {
    fprintf(desc,strans("%10N",s),pagesPerLevel[lv]);
  }
  fprintf(desc,"\n");
  fprintf(desc,"\n");
  clres= fclose(desc);
  if (clres != 0) {
    perror("RSTree.UpdateRSTDescFile: ERROR: OS");
    (*R).RSTDone= FALSE;
    return;
  }
}

/************************************************************************/

static void WriteRSTFileMap(RSTREE R, char *operat) {

  RSTName SufName;
  char mode[4]= "a";          /* append */
  /* char mode[4]= "w"; */ /* overwrite */
  FILE *desc;
  Rint clres;
  
  struct timeval tv;
  
  strlcpy(SufName,(*R).mainname,sizeof(SufName));
  strlcat(SufName,DESC_SUFF,sizeof(SufName));
  desc= fopen(SufName,mode);
  if (desc == NULL) {
    perror("RSTree.StoreRSTFileMap: ERROR: OS");
    (*R).RSTDone= FALSE;
    return;
  }
  
  gettimeofday(&tv,NULL);
  
  fprintf(desc,"-----------------------------------------------------------\n");
  // fprintf(desc,ctime(&tv.tv_sec)); // replaced:
  // For systems like Windows, not maintaining a pointer compatible
  // long, the following substitution is necessary:
  time_t tv_sec= tv.tv_sec;
  fprintf(desc,ctime(&tv_sec));
  fprintf(desc,"%s\n","FileMapping:");
  
  fprintf(desc,"%s:%s:%d\n",operat,(*R).mainname,(*R).DIR.str.f);
  
  strlcpy(SufName,(*R).mainname,sizeof(SufName));
  strlcat(SufName,DATA_SUFF,sizeof(SufName));
  fprintf(desc,"%s:%s:%d\n",operat,SufName,(*R).DATA.str.f);
  
  strlcpy(SufName,(*R).mainname,sizeof(SufName));
  strlcat(SufName,DIR_PD_SUFF,sizeof(SufName));
  fprintf(desc,"%s:%s:%d\n",operat,SufName,(*R).DIR.PA.str.f);
  
  strlcpy(SufName,(*R).mainname,sizeof(SufName));
  strlcat(SufName,DATA_PD_SUFF,sizeof(SufName));
  fprintf(desc,"%s:%s:%d\n",operat,SufName,(*R).DATA.PA.str.f);
  
  fprintf(desc,"\n");
  fprintf(desc,"\n");
  clres= fclose(desc);
  if (clres != 0) {
    perror("RSTree.StoreRSTFileMap: ERROR: OS");
    (*R).RSTDone= FALSE;
    return;
  }
}

/************************************************************************/

void ResetStorageInfo(RSTREE R, StorageKind storage)

{
  (*R).storkind= storage;
  (*R).buflinked= storage == pri || storage == lru;
  (*R).secbound= storage == lru || storage == sec;
}

/************************************************************************/

boolean LRUSatisfies(t_LRU LRU, Rpnint need)

{
  Rpnint cap, use;
  
  GetLRUCapUse(LRU,&cap,&use);
  return cap >= need;
}

/************************************************************************/

void InitRootAndCounting(RSTREE R, boolean rootExists) {

  Rint rootlvl= (*R).parameters._.rootlvl;
  reflevel LR= &(*R).L[rootlvl];
  
  InitCount(R);
  GetNode(R,&(*LR).N,ROOT_BL_NR,rootlvl);
  /* the root alternatively resides in DIR.str.f or DATA.str.f */
  (*LR).P= ROOT_BL_NR;
  if (rootExists && (*R).RSTDone) {
    if (rootlvl == 0) {
      EvalDataNodesMBB(R,(*LR).N,(*R).rootMBB);
    }
    else {
      EvalDirNodesMBB(R,(*LR).N,(*R).rootMBB);
    }
  }
}

/************************************************************************/

boolean CloseLRUBuffering(RSTREE R) {

  boolean Done= TRUE;
  
  LRUCloseFile((*R).LRU,(*R).DIR.str.f,&Done);
  if (Done) {
    LRUCloseFile((*R).LRU,(*R).DATA.str.f,&Done);
  }
  return Done;
}

/************************************************************************/
