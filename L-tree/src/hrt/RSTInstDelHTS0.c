/* ----- RSTInstDel.c ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//


#include "RSTInstDel.h"
#include "RSTInterUtil.h"
#include "RSTUtil.h"
#include "RSTPageInOut.h"
#include "RSTLRUPageIO.h"
#include "RSTMemAlloc.h"


/* types */


/* declarations */

static void AdjustPath(RSTREE R,
                       Rint level,
                       typinterval *instRect);
static void AdjustPathAfterSingDel(RSTREE R,
                                   Rint level,
                                   const typinterval *delRect);
 static boolean DirShrAdjNodesMBB(RSTREE R,
                                  refnode DIRn,
                                  const typinterval *delRect,
                                  typinterval *mbb);
static boolean DataShrAdjNodesMBB(RSTREE R,
                                  refnode DATAn,
                                  const typinterval *delRect,
                                  typinterval *mbb);
static void AdjustKeyPathAfterDeletion(RSTREE R,
                                       Rint level);
static void GetInstPath(RSTREE R,
                        OneDKey newkey,
                        Rint level);
 static Rint NextGtrDirInd(RSTREE R,
                           refnode DIRn,
                           OneDKey newkey);
static Rint NextGtrDataInd(RSTREE R,
                           refnode DATAn,
                           OneDKey newkey);
 static void ShiftRightDirEnts(RSTREE R,
                               refnode DIRn,
                               Rint pos);
static void ShiftRightDataEnts(RSTREE R,
                               refnode DATAn,
                               Rint pos);
 static void ShiftLeftDirEnts(RSTREE R,
                              refnode DIRn,
                              Rint pos);
static void ShiftLeftDataEnts(RSTREE R,
                              refnode DATAn,
                              Rint pos);
static void Split(RSTREE R,
                  void *newentry,
                  Rint level,
                  Rint M,
                  Rint m);
static void HeightenTree(RSTREE R, Rint level);
 static void SplitAndDistributDir(RSTREE R,
                                  Rint level,
                                  refnode NSplit,
                                  refnode DIRNSibl,
                                  boolean reverse,
                                  Rint M,
                                  Rint m);
static void SplitAndDistributData(RSTREE R,
                                  Rint level,
                                  refnode NSplit,
                                  refnode DATANSibl,
                                  boolean reverse,
                                  Rint M,
                                  Rint m);
static void MergeMBBsToMBB(RSTREE R,
                           typinterval *from1,
                           typinterval *from2,
                           typinterval *to);
static void Eval1ShrNodesMBB(RSTREE R,
                             Rint level,
                             const typinterval *delRect,
                             typinterval *reducedrect);
static void Merge(RSTREE R,
                  const typinterval *delRect,
                  Rint level,
                  Rint M,
                  boolean *splerge);
 static Rint MergeDirNodes(RSTREE R,
                           refnode DIRfrom,
                           refnode DIRto);
static Rint MergeDataNodes(RSTREE R,
                           refnode DATAfrom,
                           refnode DATAto);
 static Rint MergeDirNodesReverse(RSTREE R,
                                  refnode DIRfrom,
                                  refnode DIRto);
static Rint MergeDataNodesReverse(RSTREE R,
                                  refnode DATAfrom,
                                  refnode DATAto);
static void SplitMerged(RSTREE R,
                        refnode NSplit,
                        Rint level,
                        boolean reverse,
                        Rint newind,
                        Rpnint oldpagenr,
                        Rint numbentries);
static void CleanUpMerge(RSTREE R,
                         Rint level,
                         typinterval *reducedrect,
                         Rint newind,
                         Rpnint oldpagenr);
static void ShrinkTree(RSTREE R);

/* hilbert specific declarations: */
/* substitute of ChooseSubtree: */
static void DetermSubtree(RSTREE R,
                          OneDKey newkey,
                          Rint level,
                          refnode DIRnode,
                          Rint *nCov,
                          Rint I[]);
static void DetermMerge(RSTREE R,
                        typinterval *unused,
                        Rint pos,
                        refnode DIRnode,
                        boolean *behind,
                        Rint I[]);

/***********************************************************************/

static void GetInstPath(RSTREE R,
                        OneDKey newkey,
                        Rint level)

{
  Rint lv, unused, instind;
  IndexArray use0= (*R).GIP_use0;
  refparameters par= &(*R).parameters._;
  
  lv= (*par).rootlvl;
  while (lv > level) {
    DetermSubtree(R,newkey,lv,(*R).L[lv].N,&unused,use0);
    instind= use0[0];
    ExtendPath(R,instind,lv);
    lv--;
  }
}

/***********************************************************************/

void Insert(RSTREE R, void *newentry, Rint level)

{
  void *ptrNewEntry, *ptrNewKey;
  void *ptrN0, *ptrN0key, *ptrNinst;
  OneDKey newkey;
  Rint M, m, gap;
  refparameters par;
  refversion ver= &(*R).version._;
  refnode n;
  refcount c;
  typinterval *instRect= (*R).I_instRect;
  
  CopyRect((*R).numbofdim,newentry,instRect);
  ptrNewEntry= newentry;
  ptrNewKey= ptrNewEntry + (*R).entPkey;
  newkey= *(refOneDKey)ptrNewKey;
  GetInstPath(R,newkey,level);
  
  par= &(*R).parameters._;
#ifndef COUNTS_OFF
  c= &(*R).count;
#endif
  
  for (;;) {
  
    n= (*R).L[level].N;
    ptrN0= n;
    ptrN0+= (*R).Pent0;
    
    if (level == 0) {
      M= (*par).DATA.M; m= (*ver).DATA.m;
      if ((*n).s.nofentries < M) {
        gap= NextGtrDataInd(R,n,newkey);
        ShiftRightDataEnts(R,n,gap);
        ptrNinst= ptrN0 + gap * (*R).DATA.entLen;
        memcpy(ptrNinst,newentry,(*R).DATA.entLen);
        (*n).s.nofentries++;
        (*R).L[level].Modif= TRUE;
        level++;
        AdjustPath(R,level,instRect);
        break;
      }
    }
    else {
      M= (*par).DIR.M; m= (*ver).DIR.m;
      if ((*n).s.nofentries < M) {
        ptrN0key= ptrN0 + (*R).entPkey;
        if ((*par).DATA.M == 1 && level == 1 && (*R).L[level].E == 0 && newkey < *(refOneDKey)ptrN0key) {
          gap= 0;
        }
        else {
          gap= (*R).L[level].E + 1;
        }
        ShiftRightDirEnts(R,n,gap);
        ptrNinst= ptrN0 + gap * (*R).DIR.entLen;
        memcpy(ptrNinst,newentry,(*R).DIR.entLen);
        (*n).s.nofentries++;
        (*R).L[level].Modif= TRUE;
        level++;
        AdjustPath(R,level,instRect);
        break;
      }
    }
    
    /*** --- for (;;) not broken by direct insert --- ***/
#ifndef COUNTS_OFF
    if ((*c).on) {
      if (level == 0) {
        (*c).DATA.overflow++;
      }
      else {
        (*c).DIR.overflow++;
      }
    }
#endif
    Split(R,newentry,level,M,m);
    level++;
  } /* for(;;) */
}

/***********************************************************************/

 static Rint NextGtrDirInd(RSTREE R,
                           refnode n,
                           OneDKey newkey)

{
  void *ptrKey0, *ptrKeyi;
  Rint l, pos, i, entLen;
  
  ptrKey0= n;
  ptrKey0+= (*R).Pent0 + (*R).entPkey;
  
  entLen= (*R).DIR.entLen;
  l= 0;
  pos= (*n).s.nofentries;
  while (l < pos) {
    i= (l + pos) / 2;
    ptrKeyi= ptrKey0 + i * entLen;
    if (*(refOneDKey)ptrKeyi <= newkey) {
      l= i + 1;
    }
    else {
      pos= i;
    }
  }
  return pos;
}

/***********************************************************************/

static Rint NextGtrDataInd(RSTREE R,
                           refnode n,
                           OneDKey newkey)

{
  void *ptrKey0, *ptrKeyi;
  Rint l, pos, i, entLen;
  
  ptrKey0= n;
  ptrKey0+= (*R).Pent0 + (*R).entPkey;
  
  entLen= (*R).DATA.entLen;
  l= 0;
  pos= (*n).s.nofentries;
  while (l < pos) {
    i= (l + pos) / 2;
    ptrKeyi= ptrKey0 + i * entLen;
    if (*(refOneDKey)ptrKeyi <= newkey) {
      l= i + 1;
    }
    else {
      pos= i;
    }
  }
  return pos;
}

/***********************************************************************/

 static void ShiftRightDirEnts(RSTREE R,
                               refnode n,
                               Rint pos)

{
  void *ptrN0, *ptrGet, *ptrGive;
  Rint i, entLen;
  
  ptrN0= n;
  ptrN0+= (*R).Pent0;
  
  entLen= (*R).DIR.entLen;
  ptrGet= ptrN0 + (*n).s.nofentries * entLen;
  ptrGive= ptrGet - entLen;
  for (i= (*n).s.nofentries; i > pos; i--) {
    memcpy(ptrGet,ptrGive,entLen);
    ptrGet-= entLen;
    ptrGive-= entLen;
  }
}

/***********************************************************************/

static void ShiftRightDataEnts(RSTREE R,
                               refnode n,
                               Rint pos)

{
  void *ptrN0, *ptrGet, *ptrGive;
  Rint i, entLen;
  
  ptrN0= n;
  ptrN0+= (*R).Pent0;
  
  entLen= (*R).DATA.entLen;
  ptrGet= ptrN0 + (*n).s.nofentries * entLen;
  ptrGive= ptrGet - entLen;
  for (i= (*n).s.nofentries; i > pos; i--) {
    memcpy(ptrGet,ptrGive,entLen);
    ptrGet-= entLen;
    ptrGive-= entLen;
  }
}

/***********************************************************************/

 static void ShiftLeftDirEnts(RSTREE R,
                              refnode n,
                              Rint pos)

{
  void *ptrN0, *ptrGet, *ptrGive;
  Rint i, entLen;
  
  ptrN0= n;
  ptrN0+= (*R).Pent0;
  
  entLen= (*R).DIR.entLen;
  ptrGet= ptrN0 + pos * entLen;
  ptrGive= ptrGet + entLen;
  for (i= pos; i < (*n).s.nofentries; i++) {
    memcpy(ptrGet,ptrGive,entLen);
    ptrGet+= entLen;
    ptrGive+= entLen;
  }
}

/***********************************************************************/

static void ShiftLeftDataEnts(RSTREE R,
                              refnode n,
                              Rint pos)

{
  void *ptrN0, *ptrGet, *ptrGive;
  Rint i, entLen;
  
  ptrN0= n;
  ptrN0+= (*R).Pent0;
  
  entLen= (*R).DATA.entLen;
  ptrGet= ptrN0 + pos * entLen;
  ptrGive= ptrGet + entLen;
  for (i= pos; i < (*n).s.nofentries; i++) {
    memcpy(ptrGet,ptrGive,entLen);
    ptrGet+= entLen;
    ptrGive+= entLen;
  }
}

/***********************************************************************/

static void Split(RSTREE R,
                  void *newentry,
                  Rint level,
                  Rint M,
                  Rint m)

{
  void *ptrNewEntry, *ptrNewKey, *ptrNewPtts;
  void *ptrNSibl0, *ptrNSibl0key;
  void *ptrNSplit0, *ptrNSplitGap;
  void *ptrN0, *ptrN0key;
  void *ptrUpN0;
  refparameters par;
  refcount c;
  Rpnint pagenr;
  Rint gap;
  refnode NSibl= (*R).S_NSibl;
  refnode NSplit= (*R).S_NSplit;
  refnode DIN;
  refnode DAN;
  void *upEntry, *upEntryKey;
  OneDKey newkey;
  boolean isdata, newroot;
  
  par= &(*R).parameters._;
#ifndef COUNTS_OFF
  c= &(*R).count;
#endif
  
  ptrNewEntry= newentry;
  ptrNewKey= ptrNewEntry + (*R).entPkey;
  newkey= *(refOneDKey)ptrNewKey;
  
  ptrN0= (*R).L[level].N;
  ptrN0+= (*R).Pent0;
  ptrN0key= ptrN0 + (*R).entPkey;
  
  ptrNSibl0= NSibl;
  ptrNSibl0+= (*R).Pent0;
  ptrNSibl0key= ptrNSibl0 + (*R).entPkey;
  
  ptrNSplit0= NSplit;
  ptrNSplit0+= (*R).Pent0;
  
  if (level == 0) {
    if (M == 1) {
      memcpy(ptrNSibl0,newentry,(*R).DATA.entLen);
      (*NSibl).s.nofentries= 1;
    }
    else {
      memcpy(NSplit,(*R).L[level].N,(*R).DATA.str.psize);
      gap= NextGtrDataInd(R,NSplit,newkey);
      ShiftRightDataEnts(R,NSplit,gap);
      ptrNSplitGap= ptrNSplit0 + gap * (*R).DATA.entLen;
      memcpy(ptrNSplitGap,newentry,(*R).DATA.entLen);
      (*NSplit).s.nofentries++;
      SplitAndDistributData(R,level,NSplit,NSibl,FALSE,M,m);
    }
  }
  else {
    memcpy(NSplit,(*R).L[level].N,(*R).DIR.str.psize);
    if ((*par).DATA.M == 1 && level == 1 && (*R).L[level].E == 0 && newkey < *(refOneDKey)ptrN0key) {
      gap= 0;
    }
    else {
      gap= (*R).L[level].E + 1;
    }
    ShiftRightDirEnts(R,NSplit,gap);
    ptrNSplitGap= ptrNSplit0 + gap * (*R).DIR.entLen;
    memcpy(ptrNSplitGap,newentry,(*R).DIR.entLen);
    (*NSplit).s.nofentries++;
    SplitAndDistributDir(R,level,NSplit,NSibl,FALSE,M,m);
  }
  (*R).L[level].Modif= TRUE;
  
  if (level == (*par).rootlvl) {
    HeightenTree(R,level);
    newroot= TRUE;
  }
  else {
    newroot= FALSE;
  }
  isdata= level == 0;
  GetPageNr(R,&pagenr,level);
  ptrNewPtts= ptrNewEntry + (*R).entPptts;
  *(refptrtosub)ptrNewPtts= pagenr;
  
  ptrUpN0= (*R).L[level+1].N;
  ptrUpN0+= (*R).Pent0;
  upEntry= ptrUpN0 + (*R).L[level+1].E * (*R).DIR.entLen;
  
  if (isdata) {
    EvalDataNodesMBB(R,NSibl,newentry);
    *(refOneDKey)ptrNewKey=  *(refOneDKey)ptrNSibl0key;
    
    DAN= (*R).L[level].N;
    
    EvalDataNodesMBB(R,DAN,upEntry);
    upEntryKey= upEntry + (*R).entPkey;
    *(refOneDKey)upEntryKey= *(refOneDKey)ptrN0key;
    if (newroot) {
    }
#ifndef COUNTS_OFF
    if ((*c).on) {
      (*c).DATA.split++;
    }
#endif
  }
  else {
    EvalDirNodesMBB(R,NSibl,newentry);
    *(refOneDKey)ptrNewKey=  *(refOneDKey)ptrNSibl0key;
    
    DIN= (*R).L[level].N;
    
    EvalDirNodesMBB(R,DIN,upEntry);
    upEntryKey= upEntry + (*R).entPkey;
    *(refOneDKey)upEntryKey= *(refOneDKey)ptrN0key;
    if (newroot) {
    }
#ifndef COUNTS_OFF
    if ((*c).on) {
      (*c).DIR.split++;
    }
#endif
  }
  PutExtNode(R,NSibl,pagenr,level);
}

/***********************************************************************/

static void HeightenTree(RSTREE R, Rint level)

{
  void *ptrRoot0ptts;
  Rpnint pagenr;
  refparameters par= &(*R).parameters._;
  
  GetPageNr(R,&pagenr,level);
  (*R).L[level].P= pagenr;
  
  (*par).rootlvl++;
  
  if ((*R).buflinked) {
    /* copy the old root to a new page, then link the new page: */
    PutGetNode(R,&(*R).L[level].N,pagenr,level);
    /* COUNTS a put */
    (*R).L[level].Modif= FALSE;
    
    /* link the new root: */
    if ((*R).storkind == lru && (*par).rootlvl == 1) {
      /* install, lock (inc lock) a new directory buffer root: */
      EstablishLRUPage(R,&(*R).DIR.str,ROOT_BL_NR);
      /* de-install, unlock (dec lock) the data buffer root: */
      DeleteLRUPage(R,&(*R).DATA.str,ROOT_BL_NR);
    }
    /* lru: unlock (dec lock) the (surely dir-) root: */
    UnlinkPage(R,ROOT_BL_NR,(*par).rootlvl);
    /* link (and lru: lock (inc lock)) the (surely dir-) root: */
    GetNode(R,&(*R).L[(*par).rootlvl].N,ROOT_BL_NR,(*par).rootlvl);
    /* COUNTS a get */
    
    /* COUNTS ADAPTION */
    /* adapt buflinked counts to non buflinked counts: */
#ifndef COUNTS_OFF
    if ((*R).count.on) {
      (*R).count.DIR.get--;
    }
#endif
  }
  else {
    (*R).L[(*par).rootlvl].N= allocM((*R).DIR.nodeLen);
    (*R).L1[(*par).rootlvl].N= allocM((*R).DIR.nodeLen);
  }
  
  /* initiate the root: */
  (*R).L[(*par).rootlvl].P= ROOT_BL_NR;
  (*R).L[(*par).rootlvl].E= 0;
  (*(*R).L[(*par).rootlvl].N).s.nofentries= 1;
  ptrRoot0ptts= (*R).L[(*par).rootlvl].N;
  ptrRoot0ptts+= (*R).Pent0 + (*R).entPptts;
  *(refptrtosub)ptrRoot0ptts= pagenr;
  
  if ((*R).storkind == lru) {
    if (! PathLRUConsistence(R)) {
      (*R).RSTDone= FALSE;
    }
  }
}

/***********************************************************************/

 static void SplitAndDistributDir(RSTREE R,
                                  Rint level,
                                  refnode NSplit,
                                  refnode NSibl,
                                  boolean reverse,
                                  Rint M,
                                  Rint m)

{
  void *ptrNSplit0, *ptrNSplitPart2, *ptrNSibl0, *ptrN0;
  refnode n= (*R).L[level].N;
  
  ptrN0= n;
  ptrN0+= (*R).Pent0;
  ptrNSibl0= NSibl;
  ptrNSibl0+= (*R).Pent0;
  ptrNSplit0= NSplit;
  ptrNSplit0+= (*R).Pent0;
  ptrNSplitPart2= ptrNSplit0 + m * (*R).DIR.entLen;
  
  if (reverse) {
    (*NSibl).s.nofentries= m;
    memcpy(ptrNSibl0,ptrNSplit0,m * (*R).DIR.entLen);
    
    (*n).s.nofentries= M + 1 - m;
    memcpy(ptrN0,ptrNSplitPart2,(M + 1 - m) * (*R).DIR.entLen);
  }
  else {
    (*n).s.nofentries= m;
    memcpy(ptrN0,ptrNSplit0,m * (*R).DIR.entLen);
    
    (*NSibl).s.nofentries= M + 1 - m;
    memcpy(ptrNSibl0,ptrNSplitPart2,(M + 1 - m) * (*R).DIR.entLen);
  }
}

/***********************************************************************/

static void SplitAndDistributData(RSTREE R,
                                  Rint level,
                                  refnode NSplit,
                                  refnode NSibl,
                                  boolean reverse,
                                  Rint M,
                                  Rint m)

{
  void *ptrNSplit0, *ptrNSplitPart2, *ptrNSibl0, *ptrN0;
  refnode n= (*R).L[level].N;
  
  ptrN0= n;
  ptrN0+= (*R).Pent0;
  ptrNSibl0= NSibl;
  ptrNSibl0+= (*R).Pent0;
  ptrNSplit0= NSplit;
  ptrNSplit0+= (*R).Pent0;
  ptrNSplitPart2= ptrNSplit0 + m * (*R).DATA.entLen;
  
  if (reverse) {
    (*NSibl).s.nofentries= m;
    memcpy(ptrNSibl0,ptrNSplit0,m * (*R).DATA.entLen);
    
    (*n).s.nofentries= M + 1 - m;
    memcpy(ptrN0,ptrNSplitPart2,(M + 1 - m) * (*R).DATA.entLen);
  }
  else {
    (*n).s.nofentries= m;
    memcpy(ptrN0,ptrNSplit0,m * (*R).DATA.entLen);
    
    (*NSibl).s.nofentries= M + 1 - m;
    memcpy(ptrNSibl0,ptrNSplitPart2,(M + 1 - m) * (*R).DATA.entLen);
  }
}

/***********************************************************************/

static void MergeMBBsToMBB(RSTREE R,
                           refinterval from1,
                           refinterval from2,
                           refinterval to)

{
  Rint d;
  
  for (d= 0; d < (*R).numbofdim; d++) {
    if ((*from1).l < (*from2).l) {
      (*to).l= (*from1).l;
    }
    else {
      (*to).l= (*from2).l;
    }
    if ((*from1).h > (*from2).h) {
      (*to).h= (*from1).h;
    }
    else {
      (*to).h= (*from2).h;
    }
    from1++; from2++; to++;
  }  
}

/***********************************************************************/

static void Eval1ShrNodesMBB(RSTREE R,
                             Rint level,
                             const typinterval *delRect,
                             typinterval *reducedrect)

{
  void *ptrUpRect;
  if (level == (*R).parameters._.rootlvl) {
    CopyRect((*R).numbofdim,(*R).rootMBB,reducedrect);
  }
  else {
    ptrUpRect= (*R).L[level+1].N;
    ptrUpRect+= (*R).Pent0 + (*R).L[level+1].E * (*R).DIR.entLen;
    CopyRect((*R).numbofdim,ptrUpRect,reducedrect);
  }
  if (level == 0) {
    DataShrAdjNodesMBB(R,(*R).L[level].N,delRect,reducedrect);
  }
  else {
    DirShrAdjNodesMBB(R,(*R).L[level].N,delRect,reducedrect);
  }
}

/***********************************************************************/

void DeleteOneRec(RSTREE R, const typinterval *delRect)

{
  refparameters par;
  refversion ver= &(*R).version._;
  refnode n;
  refcount c;
  Rint level;
  Rint M, m;
  boolean splerge;
  
  par = &(*R).parameters._;
#ifndef COUNTS_OFF
  c= &(*R).count;
#endif
  
  level= 0;
  for (;;) {
  
    n= (*R).L[level].N;
    
    if (level == 0) {
      M= (*par).DATA.M; m= (*ver).DATA.DELm;
      (*n).s.nofentries--;
      ShiftLeftDataEnts(R,n,(*R).L[level].E);
      AdjustKeyPathAfterDeletion(R,level);
      if ((*n).s.nofentries >= m || level == (*par).rootlvl) {
        (*R).L[level].Modif= TRUE;
        AdjustPathAfterSingDel(R,level,delRect);
        break;
      }
    }
    else {
      M= (*par).DIR.M; m= (*ver).DIR.DELm;
      (*n).s.nofentries--;
      ShiftLeftDirEnts(R,n,(*R).L[level].E);
      AdjustKeyPathAfterDeletion(R,level);
      if ((*n).s.nofentries >= m || level == (*par).rootlvl) {
        (*R).L[level].Modif= TRUE;
        AdjustPathAfterSingDel(R,level,delRect);
        break;
      }
    }
    
    /*** --- for (;;) not broken by deletion without underflow --- ***/
#ifndef COUNTS_OFF
    if ((*c).on) {
      if (level == 0) {
        (*c).DATA.underflow++;
      }
      else {
        (*c).DIR.underflow++;
      }
    }
#endif
    Merge(R,delRect,level,M,&splerge);
    if (splerge) {
      AdjustPathAfterSingDel(R,level+1,delRect);
      AdjustKeyPathAfterDeletion(R,level+1);
      break;
    }
    level++;
  } /* for(;;) */
  /*** --- Shrink the tree --- ***/
  if ((*par).rootlvl != 0 && (*(*R).L[(*par).rootlvl].N).s.nofentries == 1) {
    ShrinkTree(R);
  }
}

/***********************************************************************/

static void Merge(RSTREE R,
                  const typinterval *delRect,
                  Rint level,
                  Rint M,
                  boolean *splerge)

{
  void *ptrNMerge0, *ptrN0;
  void *ptrUpDIN0, *ptrUpDINnewindPtts;
  refnode NMerge= (*R).M_NMerge;
  refnode n;
  refnode upDIN;
  refcount c;
  IndexArray use0= (*R).M_use0;
  Rpnint oldpagenr;
  Rint numbentries, entrycount, newind;
  boolean behind;
  typinterval *reducedrect= (*R).M_reducedrect;
  boolean isdata;
  
#ifndef COUNTS_OFF
  c= &(*R).count;
#endif
  upDIN= (*R).L[level+1].N;
  isdata= level == 0;
  
  *splerge= FALSE;
  
  if ((*(*R).L[level].N).s.nofentries != 0) {
    
    n= (*R).L[level].N;
    ptrN0= n;
    ptrN0+= (*R).Pent0;
    
    ptrNMerge0= NMerge;
    ptrNMerge0+= (*R).Pent0;
    
    if (isdata) {
      (*NMerge).s= (*n).s;
      memcpy(ptrNMerge0,ptrN0,(*n).s.nofentries * (*R).DATA.entLen);
    }
    else {
      (*NMerge).s= (*n).s;
      memcpy(ptrNMerge0,ptrN0,(*n).s.nofentries * (*R).DIR.entLen);
    }
    Eval1ShrNodesMBB(R,level,delRect,reducedrect);
    DetermMerge(R,
                reducedrect,
                (*R).L[level+1].E,
                upDIN,
                &behind,
                use0);
    newind= use0[0];
    oldpagenr= (*R).L[level].P;
    ptrUpDIN0= upDIN;
    ptrUpDIN0+= (*R).Pent0;
    ptrUpDINnewindPtts= ptrUpDIN0 + newind * (*R).DIR.entLen + (*R).entPptts;
    (*R).L[level].P= *(refptrtosub)ptrUpDINnewindPtts;
    GetNode(R,&(*R).L[level].N,(*R).L[level].P,level);
#ifndef COUNTS_OFF
    if ((*c).on) {
      /* count the demand belonging to the read counted in GetNode above: */
      if (isdata) {
        (*R).count.DATA.demand++;
      }
      else {
        (*R).count.DIR.demand++;
      }
    }
#endif
    numbentries= (*NMerge).s.nofentries + (*(*R).L[level].N).s.nofentries;
    if (numbentries <= M) {
      /* merge to the NEW N: */
      if (behind) {
        if (isdata) {
          entrycount= MergeDataNodesReverse(R,NMerge,(*R).L[level].N);
        }
        else {
          entrycount= MergeDirNodesReverse(R,NMerge,(*R).L[level].N);
        }
      }
      else {
        if (isdata) {
          entrycount= MergeDataNodes(R,NMerge,(*R).L[level].N);
        }
        else {
          entrycount= MergeDirNodes(R,NMerge,(*R).L[level].N);
        }
      }
      CleanUpMerge(R,level,reducedrect,newind,oldpagenr);
      /* frees the OLD N */
      /* N[level+1].entries[E[level+1]] will be deleted next */
    }
    else {
#ifndef COUNTS_OFF
      if ((*c).on) {
        if (isdata) {
          (*c).DATA.overflow++;
        }
        else {
          (*c).DIR.overflow++;
        }
      }
#endif
      /* merge to NMerge which is the OLD N: */
      if (behind) {
        if (isdata) {
          entrycount= MergeDataNodes(R,(*R).L[level].N,NMerge);
        }
        else {
          entrycount= MergeDirNodes(R,(*R).L[level].N,NMerge);
        }
      }
      else {
        if (isdata) {
          entrycount= MergeDataNodesReverse(R,(*R).L[level].N,NMerge);
        }
        else {
          entrycount= MergeDirNodesReverse(R,(*R).L[level].N,NMerge);
        }
      }
      if (entrycount != numbentries) {
        fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
        fprintf(stderr,"%s\n","Merge 1");
        (*R).RSTDone= FALSE;
      }
      SplitMerged(R,NMerge,level,behind,newind,oldpagenr,numbentries);
      *splerge= TRUE;
    }
  }
  else {
    /* empty page arisen, possible for (level == 0 != rootlvl) if m == 1. */
    PutPageNr(R,(*R).L[level].P,level);
    (*R).L[level].P= EMPTY_BL; /* never a hit! */
    (*R).L[level].Modif= FALSE;
  }
}

/***********************************************************************/

 static Rint MergeDirNodes(RSTREE R,
                           refnode from,
                           refnode to) {

  void *ptrFrom0, *ptrTo0, *ptrToInst;
  
  ptrFrom0= from;
  ptrFrom0+= (*R).Pent0;
  ptrTo0= to;
  ptrTo0+= (*R).Pent0;
  ptrToInst= ptrTo0 + (*to).s.nofentries * (*R).DIR.entLen;
  memcpy(ptrToInst,ptrFrom0,(*from).s.nofentries * (*R).DIR.entLen);
  (*to).s.nofentries+= (*from).s.nofentries;
  return (*to).s.nofentries;
}

/***********************************************************************/

static Rint MergeDataNodes(RSTREE R,
                           refnode from,
                           refnode to) {

  void *ptrFrom0, *ptrTo0, *ptrToInst;
  
  ptrFrom0= from;
  ptrFrom0+= (*R).Pent0;
  ptrTo0= to;
  ptrTo0+= (*R).Pent0;
  ptrToInst= ptrTo0 + (*to).s.nofentries * (*R).DATA.entLen;
  memcpy(ptrToInst,ptrFrom0,(*from).s.nofentries * (*R).DATA.entLen);
  (*to).s.nofentries+= (*from).s.nofentries;
  return (*to).s.nofentries;
}

/***********************************************************************/

 static Rint MergeDirNodesReverse(RSTREE R,
                                  refnode from,
                                  refnode to)

{
  void *ptrFrom0, *ptrTo0;
  void *ptrToGet, *ptrToGive;
  Rint i, lastInd, lastIndShifted;
  
  ptrFrom0= from;
  ptrFrom0+= (*R).Pent0;
  ptrTo0= to;
  ptrTo0+= (*R).Pent0;
  
  lastInd= (*to).s.nofentries - 1;
  lastIndShifted= lastInd + (*from).s.nofentries;
  ptrToGet= ptrTo0 + lastIndShifted * (*R).DIR.entLen;
  ptrToGive= ptrTo0 + lastInd * (*R).DIR.entLen;
  
  for (i= (*to).s.nofentries; i > 0; i--) {
    memcpy(ptrToGet,ptrToGive,(*R).DIR.entLen);
    ptrToGet-= (*R).DIR.entLen;
    ptrToGive-= (*R).DIR.entLen;
  }
  memcpy(ptrTo0,ptrFrom0,(*from).s.nofentries * (*R).DIR.entLen);
  (*to).s.nofentries+= (*from).s.nofentries;
  return (*to).s.nofentries;
}

/***********************************************************************/

static Rint MergeDataNodesReverse(RSTREE R,
                                  refnode from,
                                  refnode to)

{
  void *ptrFrom0, *ptrTo0;
  void *ptrToGet, *ptrToGive;
  Rint i, lastInd, lastIndShifted;
  
  ptrFrom0= from;
  ptrFrom0+= (*R).Pent0;
  ptrTo0= to;
  ptrTo0+= (*R).Pent0;
  
  lastInd= (*to).s.nofentries - 1;
  lastIndShifted= lastInd + (*from).s.nofentries;
  ptrToGet= ptrTo0 + lastIndShifted * (*R).DATA.entLen;
  ptrToGive= ptrTo0 + lastInd * (*R).DATA.entLen;
  
  for (i= (*to).s.nofentries; i > 0; i--) {
    memcpy(ptrToGet,ptrToGive,(*R).DATA.entLen);
    ptrToGet-= (*R).DATA.entLen;
    ptrToGive-= (*R).DATA.entLen;
  }
  memcpy(ptrTo0,ptrFrom0,(*from).s.nofentries * (*R).DATA.entLen);
  (*to).s.nofentries+= (*from).s.nofentries;
  return (*to).s.nofentries;
}

/***********************************************************************/

static void CleanUpMerge(RSTREE R,
                         Rint level,
                         typinterval *reducedrect,
                         Rint newind,
                         Rpnint oldpagenr)

{
  void *ptrN0key;
  void *ptrUpNewRect, *ptrUpNewKey;
  
  ptrN0key= (*R).L[level].N;
  ptrN0key+= (*R).Pent0 + (*R).entPkey;
  ptrUpNewRect= (*R).L[level+1].N;
  ptrUpNewRect+= (*R).Pent0 + newind * (*R).DIR.entLen;
  ptrUpNewKey= ptrUpNewRect + (*R).entPkey;
    
  MergeMBBsToMBB(R,reducedrect,ptrUpNewRect,ptrUpNewRect);
  *(refOneDKey)ptrUpNewKey= *(refOneDKey)ptrN0key;
  PutPageNr(R,oldpagenr,level);
  (*R).L[level].Modif= TRUE;
}

/***********************************************************************/

static void SplitMerged(RSTREE R,
                        refnode NSplit,
                        Rint level,
                        boolean reverse,
                        Rint newind,
                        Rpnint oldpagenr,
                        Rint numbentries)

{
  void *ptrN0key, *ptrNSibl0key;
  void *ptrUp0, *ptrUpRect, *ptrUpKey, *ptrUpNewRect, *ptrUpNewKey;
  refparameters par;
  refversion ver= &(*R).version._;
  refcount c;
  refnode NSibl= (*R).SM_NSibl;
  refnode DN, upDIN;
  Rint splergeM, splergem;
  
  par= &(*R).parameters._;
#ifndef COUNTS_OFF
  c= &(*R).count;
#endif
  
  splergeM= numbentries - 1;
  splergem= ((*ver).minfillpercent * splergeM + 50) / 100;
  /* build the raw m just like for split during insertion */
  
  DN= (*R).L[level].N;
    
  ptrN0key= DN;
  ptrN0key+= (*R).Pent0 + (*R).entPkey;
  
  ptrNSibl0key= NSibl;
  ptrNSibl0key+= (*R).Pent0 + (*R).entPkey;
  
  upDIN= (*R).L[level+1].N;
  
  ptrUp0= upDIN;
  ptrUp0+= (*R).Pent0;
  
  ptrUpRect= ptrUp0 + (*R).L[level+1].E * (*R).DIR.entLen;
  ptrUpKey= ptrUpRect + (*R).entPkey;
  
  ptrUpNewRect= ptrUp0 + newind * (*R).DIR.entLen;
  ptrUpNewKey= ptrUpNewRect + (*R).entPkey;
  
  if (level == 0) {
    if (splergem < DATA_MIN_ENTRY_QTY) {
      splergem= DATA_MIN_ENTRY_QTY;
    }
    if (numbentries - splergem > (*par).DATA.M) {
      splergem= numbentries - (*par).DATA.M;
    }
    /* correct the data level m as necessary */
    
    SplitAndDistributData(R,level,NSplit,NSibl,reverse,splergeM,splergem);
    EvalDataNodesMBB(R,NSibl,ptrUpRect);
    *(refOneDKey)ptrUpKey= *(refOneDKey)ptrNSibl0key;
    EvalDataNodesMBB(R,DN,ptrUpNewRect);
    *(refOneDKey)ptrUpNewKey= *(refOneDKey)ptrN0key;
#ifndef COUNTS_OFF
    if ((*c).on) {
      (*c).DATA.split++;
    }
#endif
  }
  else {
    if (splergem < DIR_MIN_ENTRY_QTY) {
      splergem= DIR_MIN_ENTRY_QTY;
    }
    if (numbentries - splergem > (*par).DIR.M) {
      splergem= numbentries - (*par).DIR.M;
    }
    /* correct the directory level m as necessary */
    
    SplitAndDistributDir(R,level,NSplit,NSibl,reverse,splergeM,splergem);
    EvalDirNodesMBB(R,NSibl,ptrUpRect);
    *(refOneDKey)ptrUpKey= *(refOneDKey)ptrNSibl0key;
    EvalDirNodesMBB(R,DN,ptrUpNewRect);
    *(refOneDKey)ptrUpNewKey= *(refOneDKey)ptrN0key;
#ifndef COUNTS_OFF
    if ((*c).on) {
      (*c).DIR.split++;
    }
#endif
  }
  PutExtNode(R,NSibl,oldpagenr,level);
  (*R).L[level].Modif= TRUE;   /* one of the siblings now in level */
  (*R).L[level+1].Modif= TRUE; /* two rectangles modified in level+1 */
}

/***********************************************************************/

static void ShrinkTree(RSTREE R)

{
  Rpnint oldpagenr;
  refparameters par= &(*R).parameters._;
  
  if ((*par).rootlvl == 1 && (*R).L[0].P == EMPTY_BL) {
    /* generate a valid root transferee (see NtoNDel, Merge) */
    (*R).L[1].E= 0;
    NewNode(R,0);
  }
  if ((*R).storkind == sec) {
    freeM(&(*R).L[(*par).rootlvl].N);
    freeM(&(*R).L1[(*par).rootlvl].N);
  }
  (*R).L[(*par).rootlvl].P= EMPTY_BL;
  (*R).L[(*par).rootlvl].E= -1;
  (*R).L[(*par).rootlvl].Modif= FALSE;
  (*par).PageCount[(*par).rootlvl]= 0;
  
  (*par).rootlvl--;
  
  oldpagenr= (*R).L[(*par).rootlvl].P;
  (*R).L[(*par).rootlvl].P= ROOT_BL_NR;
  if ((*R).buflinked) {
    /* copy the new root from the old page to the root page,
       then link the root page to the root: */
    if ((*R).storkind == lru && (*par).rootlvl == 0) {
      /* install a new data buffer root: */
      EstablishLRUPage(R,&(*R).DATA.str,ROOT_BL_NR);
      /* de-install the directory buffer root: */
      DeleteLRUPage(R,&(*R).DIR.str,ROOT_BL_NR);
    }
    PutGetNode(R,&(*R).L[(*par).rootlvl].N,ROOT_BL_NR,(*par).rootlvl);
    /* COUNTS a put */
    (*R).L[(*par).rootlvl].Modif= FALSE;
    
    /* COUNTS ADAPTION */
    /* adapt buflinked counts to non buflinked counts: */
#ifndef COUNTS_OFF
    if ((*R).count.on) {
      if ((*par).rootlvl == 0) {
        (*R).count.DATA.put--;
      }
      else {
        (*R).count.DIR.put--;
      }
    }
#endif
  }
  else {
    (*R).L[(*par).rootlvl].Modif= TRUE;
  }
  PutPageNr(R,oldpagenr,(*par).rootlvl);
  
  if ((*R).storkind == lru) {
    if (! PathLRUConsistence(R)) {
      (*R).RSTDone= FALSE;
    }
  }
}

/***********************************************************************/
/* called for the first level above insertion, i.e. for the directory only! */

static void AdjustPath(RSTREE R,
                       Rint level,
                       typinterval *instRect)

{
  void *ptrRect;
  refinterval intv, iRintv;
  boolean adjusting;
  Rint d;
  Rint rootlvl= (*R).parameters._.rootlvl;
  
  adjusting= TRUE;
  
  while (level <= rootlvl && adjusting) {
    
    ptrRect= (*R).L[level].N;
    ptrRect+= (*R).Pent0 + (*R).L[level].E * (*R).DIR.entLen;
    intv= ptrRect;
    iRintv= instRect;
    
    adjusting= FALSE;
    for (d= 0; d < (*R).numbofdim; d++) {
      if ((*intv).l > (*iRintv).l) {
        (*intv).l= (*iRintv).l;
        adjusting= TRUE;
      }
      if ((*intv).h < (*iRintv).h) {
        (*intv).h= (*iRintv).h;
        adjusting= TRUE;
      }
      intv++;
      iRintv++;
    }
    if (adjusting) {
      (*R).L[level].Modif= TRUE;
    }
    level++;
  }
  /* Begin maintenance of the rootMBB */
  if (level == rootlvl + 1 && adjusting) {
    if (rootlvl == 0 && (*(*R).L[rootlvl].N).s.nofentries == 1) {
      CopyRect((*R).numbofdim,instRect,(*R).rootMBB);
    }
    else {
      
      intv= &(*R).rootMBB[0];
      iRintv= instRect;
      
      for (d= 0; d < (*R).numbofdim; d++) {
        if ((*intv).l > (*iRintv).l) {
          (*intv).l= (*iRintv).l;
        }
        if ((*intv).h < (*iRintv).h) {
          (*intv).h= (*iRintv).h;
        }
        intv++;
        iRintv++;
      }
    }
  }
  /* End   maintenance of the rootMBB */
}

/***********************************************************************/
/* called for the level of deletion, i.e. for data and directory levels! */

static void AdjustPathAfterSingDel(RSTREE R,
                                   Rint level,
                                   const typinterval *delRect)

{
  void *ptrRect;
  refnode DIN;
  refnode DAN;
  boolean adjusting;
  Rint rootlvl= (*R).parameters._.rootlvl;
  
  adjusting= TRUE;
  if (level == 0) {
    
    DAN= (*R).L[level].N;
    
    if (level == rootlvl) {
      adjusting= FALSE;
      /* the root may be empty if it is a leaf, thus: */
      if ((*DAN).s.nofentries > 0) {
        /* Begin maintenance of the rootMBB */
        DataShrAdjNodesMBB(R,DAN,delRect,(*R).rootMBB);
        /* End   maintenance of the rootMBB */
      }
    }
    else {
      level++;
      
      ptrRect= (*R).L[level].N;
      ptrRect+= (*R).Pent0 + (*R).L[level].E * (*R).DIR.entLen;
      
      adjusting= DataShrAdjNodesMBB(R,DAN,delRect,ptrRect);
      if (adjusting) {
        (*R).L[level].Modif= TRUE;
      }
    }
  }
  while(adjusting) {
    
    DIN= (*R).L[level].N;
    
    if (level == rootlvl) {
      adjusting= FALSE;
      /* the root cannot be empty if it is not a leaf, thus: */
      /* Begin maintenance of the rootMBB */
      DirShrAdjNodesMBB(R,DIN,delRect,(*R).rootMBB);
      /* End   maintenance of the rootMBB */
    }
    else {
      level++;
      
      ptrRect= (*R).L[level].N;
      ptrRect+= (*R).Pent0 + (*R).L[level].E * (*R).DIR.entLen;
      
      adjusting= DirShrAdjNodesMBB(R,DIN,delRect,ptrRect);
      if (adjusting) {
        (*R).L[level].Modif= TRUE;
      }
    }
  }
}

/***********************************************************************/

 static boolean DirShrAdjNodesMBB(RSTREE R,
                                  refnode n,
                                  const typinterval *delReIntv,
                                  refinterval mbbIntv) {

  void *ptrNi;
  refinterval rect;
  Rint d, i;
  boolean changed;
  typatomkey coord;
  
  changed= FALSE;
  for (d= 0; d < (*R).numbofdim; d++) {
    if ((*delReIntv).l == (*mbbIntv).l) {
      /* deleted rectangle touched mbb[d].l --> recomputation of mbb[d].l */
      
      ptrNi= n;
      ptrNi+= (*R).Pent0;
      
      rect= ptrNi;
      rect+= d;
      coord= (*rect).l;
      ptrNi+= (*R).DIR.entLen;
      for (i= 1; i < (*n).s.nofentries; i++) {
        rect= ptrNi;
        rect+= d;
        if ((*rect).l < coord) {
          coord= (*rect).l;
        }
        ptrNi+= (*R).DIR.entLen;
      }
      if ((*mbbIntv).l != coord) {
        (*mbbIntv).l= coord;
        changed= TRUE;
      }
    }
    if ((*delReIntv).h == (*mbbIntv).h) {
      /* deleted rectangle touched mbb[d].h --> recomputation of mbb[d].h */
      
      ptrNi= n;
      ptrNi+= (*R).Pent0;
      
      rect= ptrNi;
      rect+= d;
      coord= (*rect).h;
      ptrNi+= (*R).DIR.entLen;
      for (i= 1; i < (*n).s.nofentries; i++) {
        rect= ptrNi;
        rect+= d;
        if ((*rect).h > coord) {
          coord= (*rect).h;
        }
        ptrNi+= (*R).DIR.entLen;
      }
      if ((*mbbIntv).h != coord) {
        (*mbbIntv).h= coord;
        changed= TRUE;
      }
    }
    delReIntv++; mbbIntv++;
  }
  return changed;
}

/***********************************************************************/

static boolean DataShrAdjNodesMBB(RSTREE R,
                                  refnode n,
                                  const typinterval *delReIntv,
                                  refinterval mbbIntv) {

  void *ptrNi;
  refinterval rect;
  Rint d, i;
  boolean changed;
  typatomkey coord;
  
  changed= FALSE;
  for (d= 0; d < (*R).numbofdim; d++) {
    if ((*delReIntv).l == (*mbbIntv).l) {
      /* deleted rectangle touched mbb[d].l --> recomputation of mbb[d].l */
      
      ptrNi= n;
      ptrNi+= (*R).Pent0;
      
      rect= ptrNi;
      rect+= d;
      coord= (*rect).l;
      ptrNi+= (*R).DATA.entLen;
      for (i= 1; i < (*n).s.nofentries; i++) {
        rect= ptrNi;
        rect+= d;
        if ((*rect).l < coord) {
          coord= (*rect).l;
        }
        ptrNi+= (*R).DATA.entLen;
      }
      if ((*mbbIntv).l != coord) {
        (*mbbIntv).l= coord;
        changed= TRUE;
      }
    }
    if ((*delReIntv).h == (*mbbIntv).h) {
      /* deleted rectangle touched mbb[d].h --> recomputation of mbb[d].h */
      
      ptrNi= n;
      ptrNi+= (*R).Pent0;
      
      rect= ptrNi;
      rect+= d;
      coord= (*rect).h;
      ptrNi+= (*R).DATA.entLen;
      for (i= 1; i < (*n).s.nofentries; i++) {
        rect= ptrNi;
        rect+= d;
        if ((*rect).h > coord) {
          coord= (*rect).h;
        }
        ptrNi+= (*R).DATA.entLen;
      }
      if ((*mbbIntv).h != coord) {
        (*mbbIntv).h= coord;
        changed= TRUE;
      }
    }
    delReIntv++; mbbIntv++;
  }
  return changed;
}

/***********************************************************************/

static void AdjustKeyPathAfterDeletion(RSTREE R,
                                       Rint level)

{
  void *ptrN0key, *ptrNEkey;
  OneDKey newkey;
  Rint rootlvl= (*R).parameters._.rootlvl;
  
  while (level != rootlvl && (*R).L[level].E == 0) {
    ptrN0key= (*R).L[level].N;
    ptrN0key+= (*R).Pent0 + (*R).entPkey;
    newkey= *(refOneDKey)ptrN0key;
    level++;
    ptrNEkey= (*R).L[level].N;
    ptrNEkey+= (*R).Pent0 + (*R).L[level].E * (*R).DIR.entLen + (*R).entPkey;
    *(refOneDKey)ptrNEkey= newkey;
    (*R).L[level].Modif= TRUE;
  }
}

/***********************************************************************/
/* substitute of ChooseSubtree, only applicable to sorted nodes */

static void DetermSubtree(RSTREE R,
                          OneDKey newkey,
                          Rint level,
                          refnode node,
                          Rint *nCov,
                          Rint I[])

{
  void *ptrNode0Key;
  Rint nextpos;
  
  nextpos= NextGtrDirInd(R,node,newkey);
  if (nextpos > 0) {
    I[0]= nextpos - 1;
  }
  else {
    /* newkey smaller than any other key */
    if ((*R).parameters._.DATA.M != 1 || level != 1) {
      ptrNode0Key= node;
      ptrNode0Key+= (*R).Pent0 + (*R).entPkey;
      *(refOneDKey)ptrNode0Key= newkey; /* adjust smallest key */
      (*R).L[level].Modif= TRUE;
    }
    I[0]= 0;
  }
}

/***********************************************************************/
/* substitute of ChooseMerge, only applicable to sorted nodes */

static void DetermMerge(RSTREE R,
                        typinterval *unused,
                        Rint pos,
                        refnode node,
                        boolean *behind,
                        Rint I[])

{
  void *ptrNode0key, *ptrNodePosKey;
  OneDKey leftdiff, rightdiff, key;
  
  ptrNode0key= node;
  ptrNode0key+= (*R).Pent0 + (*R).entPkey;
  
  ptrNodePosKey= ptrNode0key + pos * (*R).DIR.entLen;
  key= *(refOneDKey)ptrNodePosKey;
  
  if (pos == 0) {
    I[0]= 1;
    *behind= TRUE;
  }
  else if (pos == (*node).s.nofentries - 1) {
    I[0]= (*node).s.nofentries - 2;
    *behind= FALSE;
  }
  else {
    ptrNodePosKey= ptrNode0key + (pos - 1) * (*R).DIR.entLen;
    leftdiff= key - *(refOneDKey)ptrNodePosKey;
    ptrNodePosKey= ptrNode0key + (pos + 1) * (*R).DIR.entLen;
    rightdiff= *(refOneDKey)ptrNodePosKey - key;
    if (rightdiff < leftdiff) {
      I[0]= pos + 1;
      *behind= TRUE;
    }
    else {
      I[0]= pos - 1;
      *behind= FALSE;
    }
  }
}

/***********************************************************************/
