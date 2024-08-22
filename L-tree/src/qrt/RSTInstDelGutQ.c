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
#include "RSTChooseSub.h"


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
static void GetInstPath(RSTREE R,
                        typinterval *newrect,
                        Rint level);
static void Split(RSTREE R,
                  void *newentry,
                  typinterval *instRect,
                  Rint level,
                  Rint M,
                  Rint m);
static void HeightenTree(RSTREE R, Rint level);
 static void SplitAndDistributDir(RSTREE R,
                                  Rint level,
                                  refnode NSplit,
                                  refnode DIRNSibl,
                                  typinterval *allrect,
                                  Rint M,
                                  Rint m);
static void SplitAndDistributData(RSTREE R,
                                  Rint level,
                                  refnode NSplit,
                                  refnode DATANSibl,
                                  typinterval *allrect,
                                  Rint M,
                                  Rint m);
static void Eval1ExtNodesMBB(RSTREE R,
                             Rint level,
                             typinterval *newrect,
                             typinterval *allrect);
static void MergeMBBsToMBB(RSTREE R,
                           typinterval *from1,
                           typinterval *from2,
                           typinterval *to);
static void ShrinkTree(RSTREE R);
static void NtoNDel(RSTREE R,
                    Rint level);
static void ReInsertPath(RSTREE R);

/* Function declarations for Guttman's R-Tree: */
 static void DirPickSeeds(RSTREE R,
                          refnode DIRn,
                          Rint M,
                          Rint *seed1,
                          Rint *seed2,
                          Rfloat *area1,
                          Rfloat *area2);
static void DataPickSeeds(RSTREE R,
                          refnode DATAn,
                          Rint M,
                          Rint *seed1,
                          Rint *seed2,
                          Rfloat *area1,
                          Rfloat *area2);
 static void DirPickNext(RSTREE R,
                         Rint level,
                         refnode DIRNSplit,
                         refnode DIRNSibl,
                         Rint M,
                         Rint m,
                         Rint seedN,
                         Rint seedNSib,
                         Rfloat areaN,
                         Rfloat areaNSib);
static void DataPickNext(RSTREE R,
                         Rint level,
                         refnode DATANSplit,
                         refnode DATANSibl,
                         Rint M,
                         Rint m,
                         Rint seedN,
                         Rint seedNSib,
                         Rfloat areaN,
                         Rfloat areaNSib);
static Rfloat MBBArea(RSTREE R,
                      typinterval *rect1,
                      typinterval *rect2);
static void GetMBBAndArea(RSTREE R,
                          typinterval *mbb,
                          Rfloat *area,
                          typinterval *otherRect);
 static void DirAssignActOrRest(RSTREE R,
                                refnode DIRNAss,
                                Rint *numAss,
                                Rint found,
                                typinterval *rectAss,
                                Rfloat *areaAss,
                                Rint *numgone,
                                byte *gone,
                                Rint M,
                                Rint m,
                                refnode DIRNSplit,
                                refnode DIRNOther,
                                Rint *numOther);
static void DataAssignActOrRest(RSTREE R,
                                refnode DATANAss,
                                Rint *numAss,
                                Rint found,
                                typinterval *rectAss,
                                Rfloat *areaAss,
                                Rint *numgone,
                                byte *gone,
                                Rint M,
                                Rint m,
                                refnode DATANSplit,
                                refnode DATANOther,
                                Rint *numOther);
/* Dir|DataAssignActOrRest:
   assigns the actual entry of NSplit to DIR|DATANAss;
   if necessary assigns the rest of DIR|DATANSplit to DIR|DATANOther */
/* ------------------------------------------------------------------- */
/* functions repeated here to be available for inlining: */
/* ------------------------------------------------------------------- */
/* identical to ClearVector() in RSTUtil: */
static void ID_ClearVector(typword *ptr, Rint wordsqty);

/***********************************************************************/

static void GetInstPath(RSTREE R,
                        typinterval *newrect,
                        Rint level)

{
  Rint lv, unused, instind;
  IndexArray use0= (*R).GIP_use0;
  refparameters par= &(*R).parameters._;
  
  lv= (*par).rootlvl;
  while (lv > level) {
    ChooseSubtree(R,newrect,lv,(*R).L[lv].N,&unused,use0);
    instind= use0[0];
    ExtendPath(R,instind,lv);
    lv--;
  }
}

/***********************************************************************/

void Insert(RSTREE R, void *newentry, Rint level)

{
  void *ptrN0, *ptrNInst;
  Rint M, m;
  refparameters par;
  refversion ver= &(*R).version._;
  refnode n;
  refcount c;
  typinterval *instRect= (*R).I_instRect;
  
  CopyRect((*R).numbofdim,newentry,instRect);
  GetInstPath(R,instRect,level);
  
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
        ptrNInst= ptrN0 + (*n).s.nofentries * (*R).DATA.entLen;
        memcpy(ptrNInst,newentry,(*R).DATA.entLen);
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
        ptrNInst= ptrN0 + (*n).s.nofentries * (*R).DIR.entLen;
        memcpy(ptrNInst,newentry,(*R).DIR.entLen);
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
    Split(R,newentry,instRect,level,M,m);
    level++;
  } /* for(;;) */
}

/***********************************************************************/

static void Split(RSTREE R,
                  void *newentry,
                  typinterval *instRect,
                  Rint level,
                  Rint M,
                  Rint m)

{
  void *ptrNSibl0, *ptrNSplit0, *ptrNSplitInst;
  void *ptrNewentPtts;
  void *ptrUpperN0;
  refparameters par;
  refcount c;
  Rpnint pagenr;
  boolean isdata, newroot;
  refnode NSibl= (*R).S_NSibl;
  refnode NSplit= (*R).S_NSplit;
  void *upRect;
  typinterval *allrect= (*R).S_allrect;
  
  par= &(*R).parameters._;
#ifndef COUNTS_OFF
  c= &(*R).count;
#endif
  
  ptrNSibl0= NSibl;
  ptrNSibl0+= (*R).Pent0;
  
  ptrNSplit0= NSplit;
  ptrNSplit0+= (*R).Pent0;
  
  if (level == 0) {
    if (M == 1) {
      memcpy(ptrNSibl0,newentry,(*R).DATA.entLen);
      (*NSibl).s.nofentries= 1;
    }
    else {
      memcpy(NSplit,(*R).L[level].N,(*R).DATA.str.psize);
      ptrNSplitInst= ptrNSplit0 + M * (*R).DATA.entLen;
      memcpy(ptrNSplitInst,newentry,(*R).DATA.entLen);
      (*NSplit).s.nofentries++;
      Eval1ExtNodesMBB(R,level,instRect,allrect);
      SplitAndDistributData(R,level,NSplit,NSibl,allrect,M,m);
    }
  }
  else {
    memcpy(NSplit,(*R).L[level].N,(*R).DIR.str.psize);
    ptrNSplitInst= ptrNSplit0 + M * (*R).DIR.entLen;
    memcpy(ptrNSplitInst,newentry,(*R).DIR.entLen);
    (*NSplit).s.nofentries++;
    Eval1ExtNodesMBB(R,level,instRect,allrect);
    SplitAndDistributDir(R,level,NSplit,NSibl,allrect,M,m);
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
  ptrNewentPtts= newentry;
  ptrNewentPtts+= (*R).entPptts;
  *(refptrtosub)ptrNewentPtts= pagenr;

  ptrUpperN0= (*R).L[level+1].N;
  ptrUpperN0+= (*R).Pent0;
  upRect= ptrUpperN0 + (*R).L[level+1].E * (*R).DIR.entLen;
  
  if (isdata) {
    EvalDataNodesMBB(R,NSibl,newentry);
    EvalDataNodesMBB(R,(*R).L[level].N,upRect);
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
    EvalDirNodesMBB(R,(*R).L[level].N,upRect);
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
                                  typinterval *allrect,
                                  Rint M,
                                  Rint m)

{
  Rint seed1, seed2;
  Rfloat area1, area2;
  
  DirPickSeeds(R,NSplit,M,&seed1,&seed2,&area1,&area2);
  DirPickNext(R,level,NSplit,NSibl,M,m,seed1,seed2,area1,area2);
}

/***********************************************************************/

static void SplitAndDistributData(RSTREE R,
                                  Rint level,
                                  refnode NSplit,
                                  refnode NSibl,
                                  typinterval *allrect,
                                  Rint M,
                                  Rint m)

{
  Rint seed1, seed2;
  Rfloat area1, area2;
  
  DataPickSeeds(R,NSplit,M,&seed1,&seed2,&area1,&area2);
  DataPickNext(R,level,NSplit,NSibl,M,m,seed1,seed2,area1,area2);
}

/***********************************************************************/

 static void DirPickSeeds(RSTREE R,
                          refnode n,
                          Rint M,
                          Rint *seed1,
                          Rint *seed2,
                          Rfloat *area1,
                          Rfloat *area2)

{
  void *ptrN0, *ptrNi, *ptrNj;
  Rint i, j, d;
  refinterval intv;
  ValueArray Areas= (*R).DDPS_Areas;
  Rfloat *Ai, *Aj;
  Rfloat mbbArea, areaWaste, validAreaWaste;
  
  ptrN0= n;
  ptrN0+= (*R).Pent0;
  
  ptrNi= ptrN0;
  for (i= 0; i <= M; i++) {
    
    intv= ptrNi;
    
    Areas[i]= 1.0;
    for (d= 0; d < (*R).numbofdim; d++) {
      Areas[i]*= (*intv).h - (*intv).l;
      intv++;
    }
    ptrNi+= (*R).DIR.entLen;
  }
  ptrNi= ptrN0;
  Ai= Areas;
  for (i= 0; i <= M; i++) {
    ptrNj= ptrN0 + (i + 1) * (*R).DIR.entLen;
    Aj= Ai + 1;
    for (j= i + 1; j <= M; j++) {
      mbbArea= MBBArea(R,ptrNi,ptrNj);
      areaWaste= mbbArea - *Ai - *Aj;
      if (i == 0 && j == 1) {
        validAreaWaste= areaWaste;
        *seed1= i; *seed2= j;
      }
      else if (areaWaste > validAreaWaste) {
        validAreaWaste= areaWaste;
        *seed1= i; *seed2= j;
      }
      ptrNj+= (*R).DIR.entLen;
      Aj++;
    }
    ptrNi+= (*R).DIR.entLen;
    Ai++;
  }
  *area1= Areas[*seed1]; *area2= Areas[*seed2];
}

/***********************************************************************/

static void DataPickSeeds(RSTREE R,
                          refnode n,
                          Rint M,
                          Rint *seed1,
                          Rint *seed2,
                          Rfloat *area1,
                          Rfloat *area2)

{
  void *ptrN0, *ptrNi, *ptrNj;
  Rint i, j, d;
  refinterval intv;
  ValueArray Areas= (*R).DDPS_Areas;
  Rfloat *Ai, *Aj;
  Rfloat mbbArea, areaWaste, validAreaWaste;
  
  ptrN0= n;
  ptrN0+= (*R).Pent0;
  
  ptrNi= ptrN0;
  for (i= 0; i <= M; i++) {
    
    intv= ptrNi;
    
    Areas[i]= 1.0;
    for (d= 0; d < (*R).numbofdim; d++) {
      Areas[i]*= (*intv).h - (*intv).l;
      intv++;
    }
    ptrNi+= (*R).DATA.entLen;
  }
  ptrNi= ptrN0;
  Ai= Areas;
  for (i= 0; i <= M; i++) {
    ptrNj= ptrN0 + (i + 1) * (*R).DATA.entLen;
    Aj= Ai + 1;
    for (j= i + 1; j <= M; j++) {
      mbbArea= MBBArea(R,ptrNi,ptrNj);
      areaWaste= mbbArea - *Ai - *Aj;
      if (i == 0 && j == 1) {
        validAreaWaste= areaWaste;
        *seed1= i; *seed2= j;
      }
      else if (areaWaste > validAreaWaste) {
        validAreaWaste= areaWaste;
        *seed1= i; *seed2= j;
      }
      ptrNj+= (*R).DATA.entLen;
      Aj++;
    }
    ptrNi+= (*R).DATA.entLen;
    Ai++;
  }
  *area1= Areas[*seed1]; *area2= Areas[*seed2];
}

/***********************************************************************/

 static void DirPickNext(RSTREE R,
                         Rint level,
                         refnode NSplit,
                         refnode NSibl,
                         Rint M,
                         Rint m,
                         Rint seedN,
                         Rint seedNSib,
                         Rfloat areaN,
                         Rfloat areaNSib)

{
  void *ptrN0, *ptrNSibl0, *ptrNSplit0;
  void *ptrNSplitSeedN, *ptrNSplitSeedNSib;
  void *ptrNSplitj;
  Rint numN, numNSib, numgone, j, found;
  boolean first;
  Rfloat diff, validdiff, new_areaN, new_areaNSib, enlN, enlNSib;
  typinterval *rectN= (*R).DDPN_rectN;
  typinterval *rectNSib= (*R).DDPN_rectNSib;
  uWordsBytes *gone= (*R).DDPN_gone;
  enum {cN, cNSib, none} chosen, validchoice;
  refnode N= (*R).L[level].N;
  
  ID_ClearVector((*gone).words,(*R).DIReqvWords_Mp1);
  numgone= 0;
  
  ptrN0= N;
  ptrN0+= (*R).Pent0;
  
  ptrNSibl0= NSibl;
  ptrNSibl0+= (*R).Pent0;
  
  ptrNSplit0= NSplit;
  ptrNSplit0+= (*R).Pent0;
  
  ptrNSplitSeedN= ptrNSplit0 + seedN * (*R).DIR.entLen;
  memcpy(ptrN0,ptrNSplitSeedN,(*R).DIR.entLen);
  CopyRect((*R).numbofdim,ptrNSplitSeedN,rectN);
  (*gone)._[seedN]= 1;
  numN= 1;
  numgone++;
  
  ptrNSplitSeedNSib= ptrNSplit0 + seedNSib * (*R).DIR.entLen;
  memcpy(ptrNSibl0,ptrNSplitSeedNSib,(*R).DIR.entLen);
  CopyRect((*R).numbofdim,ptrNSplitSeedNSib,rectNSib);
  (*gone)._[seedNSib]= 1;
  numNSib= 1;
  numgone++;
  
  do {
    validdiff= 0.0; validchoice= none; first= TRUE;
    
    ptrNSplitj= ptrNSplit0;
    for (j= 0; j <= M; j++) {
      if ((*gone)._[j] == 0) {
        if (first) {
          found= j;
          first= FALSE;
        }
        new_areaN= MBBArea(R,rectN,ptrNSplitj);
        new_areaNSib= MBBArea(R,rectNSib,ptrNSplitj);
        enlN= new_areaN - areaN; enlNSib= new_areaNSib - areaNSib;
        if (enlN < enlNSib) {
          chosen= cN;
          diff= enlNSib - enlN;
        }
        else {
          chosen= cNSib;
          diff= enlN - enlNSib;
        }
        if (diff > validdiff) {
          validdiff= diff;
          validchoice= chosen;
          found= j;
        }
      }
      ptrNSplitj+= (*R).DIR.entLen;
    }
    if (validchoice == cN) {
      DirAssignActOrRest(R,N,&numN,found,rectN,&areaN,
                         &numgone,(*gone)._,M,m,NSplit,
                         NSibl,&numNSib);
    }
    else if (validchoice == cNSib) {
      DirAssignActOrRest(R,NSibl,&numNSib,found,rectNSib,&areaNSib,
                         &numgone,(*gone)._,M,m,NSplit,
                         N,&numN);
    }
    else {
      if (areaN < areaNSib) {
        DirAssignActOrRest(R,N,&numN,found,rectN,&areaN,
                           &numgone,(*gone)._,M,m,NSplit,
                           NSibl,&numNSib);
      }
      else if (areaNSib < areaN) {
        DirAssignActOrRest(R,NSibl,&numNSib,found,rectNSib,&areaNSib,
                           &numgone,(*gone)._,M,m,NSplit,
                           N,&numN);
      }
      else {
        if (numN < numNSib) {
          DirAssignActOrRest(R,N,&numN,found,rectN,&areaN,
                             &numgone,(*gone)._,M,m,NSplit,
                             NSibl,&numNSib);
        }
        else {
          DirAssignActOrRest(R,NSibl,&numNSib,found,rectNSib,&areaNSib,
                             &numgone,(*gone)._,M,m,NSplit,
                             N,&numN);
        }
      }
    }
  } while (numgone < M + 1);
  (*N).s.nofentries= numN;
  (*NSibl).s.nofentries= numNSib;
  
  if (numgone != M + 1) {
    fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
    fprintf(stderr,"%s\n","DirPickNext 1");
    abort();
  }
  if (numN + numNSib != numgone) {
    fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
    fprintf(stderr,"%s\n","DirPickNext 2");
    abort();
  }
  if (numN < m || numNSib < m) {
    fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
    fprintf(stderr,"%s\n","DirPickNext 3");
    abort();
  }
}

/***********************************************************************/

static void DataPickNext(RSTREE R,
                         Rint level,
                         refnode NSplit,
                         refnode NSibl,
                         Rint M,
                         Rint m,
                         Rint seedN,
                         Rint seedNSib,
                         Rfloat areaN,
                         Rfloat areaNSib)

{
  void *ptrN0, *ptrNSibl0, *ptrNSplit0;
  void *ptrNSplitSeedN, *ptrNSplitSeedNSib;
  void *ptrNSplitj;
  Rint numN, numNSib, numgone, j, found;
  boolean first;
  Rfloat diff, validdiff, new_areaN, new_areaNSib, enlN, enlNSib;
  typinterval *rectN= (*R).DDPN_rectN;
  typinterval *rectNSib= (*R).DDPN_rectNSib;
  uWordsBytes *gone= (*R).DDPN_gone;
  enum {cN, cNSib, none} chosen, validchoice;
  refnode N= (*R).L[level].N;
  
  ID_ClearVector((*gone).words,(*R).DATAeqvWords_Mp1);
  numgone= 0;
  
  ptrN0= N;
  ptrN0+= (*R).Pent0;
  
  ptrNSibl0= NSibl;
  ptrNSibl0+= (*R).Pent0;
  
  ptrNSplit0= NSplit;
  ptrNSplit0+= (*R).Pent0;
  
  ptrNSplitSeedN= ptrNSplit0 + seedN * (*R).DATA.entLen;
  memcpy(ptrN0,ptrNSplitSeedN,(*R).DATA.entLen);
  CopyRect((*R).numbofdim,ptrNSplitSeedN,rectN);
  (*gone)._[seedN]= 1;
  numN= 1;
  numgone++;
  
  ptrNSplitSeedNSib= ptrNSplit0 + seedNSib * (*R).DATA.entLen;
  memcpy(ptrNSibl0,ptrNSplitSeedNSib,(*R).DATA.entLen);
  CopyRect((*R).numbofdim,ptrNSplitSeedNSib,rectNSib);
  (*gone)._[seedNSib]= 1;
  numNSib= 1;
  numgone++;
  
  do {
    validdiff= 0.0; validchoice= none; first= TRUE;
    
    ptrNSplitj= ptrNSplit0;
    for (j= 0; j <= M; j++) {
      if ((*gone)._[j] == 0) {
        if (first) {
          found= j;
          first= FALSE;
        }
        new_areaN= MBBArea(R,rectN,ptrNSplitj);
        new_areaNSib= MBBArea(R,rectNSib,ptrNSplitj);
        enlN= new_areaN - areaN; enlNSib= new_areaNSib - areaNSib;
        if (enlN < enlNSib) {
          chosen= cN;
          diff= enlNSib - enlN;
        }
        else {
          chosen= cNSib;
          diff= enlN - enlNSib;
        }
        if (diff > validdiff) {
          validdiff= diff;
          validchoice= chosen;
          found= j;
        }
      }
      ptrNSplitj+= (*R).DATA.entLen;
    }
    if (validchoice == cN) {
      DataAssignActOrRest(R,N,&numN,found,rectN,&areaN,
                          &numgone,(*gone)._,M,m,NSplit,
                          NSibl,&numNSib);
    }
    else if (validchoice == cNSib) {
      DataAssignActOrRest(R,NSibl,&numNSib,found,rectNSib,&areaNSib,
                          &numgone,(*gone)._,M,m,NSplit,
                          N,&numN);
    }
    else {
      if (areaN < areaNSib) {
        DataAssignActOrRest(R,N,&numN,found,rectN,&areaN,
                            &numgone,(*gone)._,M,m,NSplit,
                            NSibl,&numNSib);
      }
      else if (areaNSib < areaN) {
        DataAssignActOrRest(R,NSibl,&numNSib,found,rectNSib,&areaNSib,
                            &numgone,(*gone)._,M,m,NSplit,
                            N,&numN);
      }
      else {
        if (numN < numNSib) {
          DataAssignActOrRest(R,N,&numN,found,rectN,&areaN,
                              &numgone,(*gone)._,M,m,NSplit,
                              NSibl,&numNSib);
        }
        else {
          DataAssignActOrRest(R,NSibl,&numNSib,found,rectNSib,&areaNSib,
                              &numgone,(*gone)._,M,m,NSplit,
                              N,&numN);
        }
      }
    }
  } while (numgone < M + 1);
  (*N).s.nofentries= numN;
  (*NSibl).s.nofentries= numNSib;
  
  if (numgone != M + 1) {
    fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
    fprintf(stderr,"%s\n","DataPickNext 1");
    abort();
  }
  if (numN + numNSib != numgone) {
    fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
    fprintf(stderr,"%s\n","DataPickNext 2");
    abort();
  }
  if (numN < m || numNSib < m) {
    fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
    fprintf(stderr,"%s\n","DataPickNext 3");
    abort();
  }
}

/***********************************************************************/

static Rfloat MBBArea(RSTREE R,
                      typinterval *rect1,
                      typinterval *rect2)

{
  Rint d;
  typatomkey low, high;
  Rfloat area;
  
  area= 1.0;
  for (d= 0; d < (*R).numbofdim; d++) {
    if ((*rect2).l < (*rect1).l) {
      low= (*rect2).l;
    }
    else {
      low= (*rect1).l;
    }
    if ((*rect2).h > (*rect1).h) {
      high= (*rect2).h;
    }
    else {
      high= (*rect1).h;
    }
    area*= high - low;
    rect1++; rect2++;
  }
  return area;
}

/***********************************************************************/

static void GetMBBAndArea(RSTREE R,
                          typinterval *mbb,
                          Rfloat *area,
                          typinterval *otherRect)

{
  Rint d;
  
  *area= 1.0;
  for (d= 0; d < (*R).numbofdim; d++) {
    if (otherRect[d].l < mbb[d].l) {
      mbb[d].l= otherRect[d].l;
    }
    if (otherRect[d].h > mbb[d].h) {
      mbb[d].h= otherRect[d].h;
    }
    *area*= mbb[d].h - mbb[d].l;
  }
}

/***********************************************************************/

 static void DirAssignActOrRest(RSTREE R,
                                refnode NAss,
                                Rint *numAss,
                                Rint found,
                                typinterval *rectAss,
                                Rfloat *areaAss,
                                Rint *numgone,
                                byte *gone,
                                Rint M,
                                Rint m,
                                refnode NSplit,
                                refnode NOther,
                                Rint *numOther)
{
  void *ptrNAss0, *ptrNAssNumAss;
  void *ptrNOther0, *ptrNOtherNumOther;
  void *ptrNSplit0, *ptrNSplitFound, *ptrNSpliti;
  Rint i;
  
  ptrNAss0= NAss;
  ptrNAss0+= (*R).Pent0;
  
  ptrNOther0= NOther;
  ptrNOther0+= (*R).Pent0;
  
  ptrNSplit0= NSplit;
  ptrNSplit0+= (*R).Pent0;
  
  ptrNAssNumAss= ptrNAss0 + *numAss * (*R).DIR.entLen;
  ptrNSplitFound= ptrNSplit0 + found * (*R).DIR.entLen;
  memcpy(ptrNAssNumAss,ptrNSplitFound,(*R).DIR.entLen);
  gone[found]= 1;
  (*numAss)++; (*numgone)++;
  if (*numAss == M + 1 - m) {
    /* assign the rest to NOther */
    ptrNOtherNumOther= ptrNOther0 + *numOther * (*R).DIR.entLen;
    for (i= 0; i <= M; i++) {
      if (gone[i] == 0) {
        ptrNSpliti= ptrNSplit0 + i * (*R).DIR.entLen;
        memcpy(ptrNOtherNumOther,ptrNSpliti,(*R).DIR.entLen);
        (*numOther)++; (*numgone)++;
        ptrNOtherNumOther+= (*R).DIR.entLen;
      }
    }
  }
  else {
    /* adapt geometrics of NAss */
    GetMBBAndArea(R,rectAss,areaAss,ptrNSplitFound);
  }
}

/***********************************************************************/

static void DataAssignActOrRest(RSTREE R,
                                refnode NAss,
                                Rint *numAss,
                                Rint found,
                                typinterval *rectAss,
                                Rfloat *areaAss,
                                Rint *numgone,
                                byte *gone,
                                Rint M,
                                Rint m,
                                refnode NSplit,
                                refnode NOther,
                                Rint *numOther)
{
  void *ptrNAss0, *ptrNAssNumAss;
  void *ptrNOther0, *ptrNOtherNumOther;
  void *ptrNSplit0, *ptrNSplitFound, *ptrNSpliti;
  Rint i;
  
  ptrNAss0= NAss;
  ptrNAss0+= (*R).Pent0;
  
  ptrNOther0= NOther;
  ptrNOther0+= (*R).Pent0;
  
  ptrNSplit0= NSplit;
  ptrNSplit0+= (*R).Pent0;
  
  ptrNAssNumAss= ptrNAss0 + *numAss * (*R).DATA.entLen;
  ptrNSplitFound= ptrNSplit0 + found * (*R).DATA.entLen;
  memcpy(ptrNAssNumAss,ptrNSplitFound,(*R).DATA.entLen);
  gone[found]= 1;
  (*numAss)++; (*numgone)++;
  if (*numAss == M + 1 - m) {
    /* assign the rest to NOther */
    ptrNOtherNumOther= ptrNOther0 + *numOther * (*R).DATA.entLen;
    for (i= 0; i <= M; i++) {
      if (gone[i] == 0) {
        ptrNSpliti= ptrNSplit0 + i * (*R).DATA.entLen;
        memcpy(ptrNOtherNumOther,ptrNSpliti,(*R).DATA.entLen);
        (*numOther)++; (*numgone)++;
        ptrNOtherNumOther+= (*R).DATA.entLen;
      }
    }
  }
  else {
    /* adapt geometrics of NAss */
    GetMBBAndArea(R,rectAss,areaAss,ptrNSplitFound);
  }
}

/***********************************************************************/

static void Eval1ExtNodesMBB(RSTREE R,
                             Rint level,
                             typinterval *newrect,
                             typinterval *allrect)

{
  void *ptrUpRect;
  
  if (level == (*R).parameters._.rootlvl) {
    ptrUpRect= (*R).rootMBB;
  }
  else {
    ptrUpRect= (*R).L[level+1].N;
    ptrUpRect+= (*R).Pent0 + (*R).L[level+1].E * (*R).DIR.entLen;
  }
  MergeMBBsToMBB(R,ptrUpRect,newrect,allrect);
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
/* This deletion by reinsertion version would not need a delRect! --> .E! */

void DeleteOneRec(RSTREE R, const typinterval *delRect)

{
  void *ptrN0, *ptrNlast, *ptrNdel;
  refparameters par;
  refversion ver= &(*R).version._;
  refnode n;
  refcount c;
  Rint level;
  Rint M, m;
  typinterval *effDelRect= (*R).DOR_effDelRect;
  
  par = &(*R).parameters._;
#ifndef COUNTS_OFF
  c= &(*R).count;
#endif
  
  level= 0;
  for (;;) {
  
    n= (*R).L[level].N;
    ptrN0= n;
    ptrN0+= (*R).Pent0;
    
    if (level == 0) {
      M= (*par).DATA.M; m= (*ver).DATA.DELm;
      (*n).s.nofentries--;
      ptrNlast= ptrN0 + (*n).s.nofentries * (*R).DATA.entLen;
      ptrNdel= ptrN0 + (*R).L[level].E * (*R).DATA.entLen;
      memcpy(ptrNdel,ptrNlast,(*R).DATA.entLen);
      if ((*n).s.nofentries >= m || level == (*par).rootlvl) {
        (*R).L[level].Modif= TRUE;
        AdjustPathAfterSingDel(R,level,delRect);
        break;
      }
    }
    else {
      M= (*par).DIR.M; m= (*ver).DIR.DELm;
      (*n).s.nofentries--;
      ptrNlast= ptrN0 + (*n).s.nofentries * (*R).DIR.entLen;
      ptrNdel= ptrN0 + (*R).L[level].E * (*R).DIR.entLen;
      CopyRect((*R).numbofdim,ptrNdel,effDelRect);
      memcpy(ptrNdel,ptrNlast,(*R).DIR.entLen);
      if ((*n).s.nofentries >= m || level == (*par).rootlvl) {
        (*R).L[level].Modif= TRUE;
        AdjustPathAfterSingDel(R,level,effDelRect);
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
    NtoNDel(R,level);
    level++;
  } /* for(;;) */
  ReInsertPath(R);
  /*** --- Shrink the tree --- ***/
  if ((*par).rootlvl != 0 && (*(*R).L[(*par).rootlvl].N).s.nofentries == 1) {
    ShrinkTree(R);
  }
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

static void NtoNDel(RSTREE R,
                    Rint level)

{
  void *ptrN0, *ptrNDel0;
  refnode n, nd;
  
  if ((*(*R).L[level].N).s.nofentries != 0) {
    if ((*R).buflinked) {
      
      n= (*R).L[level].N;
      ptrN0= n;
      ptrN0+= (*R).Pent0;
      
      if (level == 0) {
        (*R).LDel[level].N= allocM((*R).DATA.nodeLen);
        nd= (*R).LDel[level].N;
        ptrNDel0= nd;
        ptrNDel0+= (*R).Pent0;
        (*nd).s= (*n).s;
        memcpy(ptrNDel0,ptrN0,(*n).s.nofentries * (*R).DATA.entLen);
      }
      else {
        (*R).LDel[level].N= allocM((*R).DIR.nodeLen);
        nd= (*R).LDel[level].N;
        ptrNDel0= nd;
        ptrNDel0+= (*R).Pent0;
        (*nd).s= (*n).s;
        memcpy(ptrNDel0,ptrN0,(*n).s.nofentries * (*R).DIR.entLen);
      }
    }
    else {
      (*R).LDel[level].N= (*R).L[level].N;
      if (level == 0) {
        (*R).L[level].N= allocM((*R).DATA.nodeLen);
      }
      else {
        (*R).L[level].N= allocM((*R).DIR.nodeLen);
      }
    }
  }
  else {
    /* empty page arisen, possible for (level == 0 != rootlvl) if m == 1. */
  }
  PutPageNr(R,(*R).L[level].P,level);
  (*R).L[level].P= EMPTY_BL; /* never a hit! */
  (*R).L[level].Modif= FALSE;
}

/***********************************************************************/

static void ReInsertPath(RSTREE R)

{
  void *ptrNDel0, *ptrNDeli;
  refnode nd;
  Rint i, level;

  level= (*R).parameters._.rootlvl - 1;
  while (level >= 0) {
    if ((*R).LDel[level].N != NULL) {
    
      nd= (*R).LDel[level].N;
      ptrNDel0= nd;
      ptrNDel0+= (*R).Pent0;
      
      if (level == 0) {
        ptrNDeli= ptrNDel0;
        for (i= 0; i < (*nd).s.nofentries; i++) {
          Insert(R,ptrNDeli,level);
          ptrNDeli+= (*R).DATA.entLen;
        }
      }
      else {
        ptrNDeli= ptrNDel0;
        for (i= 0; i < (*nd).s.nofentries; i++) {
          Insert(R,ptrNDeli,level);
          ptrNDeli+= (*R).DIR.entLen;
        }
      }
      freeM(&(*R).LDel[level].N);
    }
    level--;
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

/* ------------------------------------------------------------------- */
/* functions repeated here to be available for inlining: */
/* ------------------------------------------------------------------- */
/* identical to ClearVector() in RSTUtil: */

static void ID_ClearVector(typword *ptr, Rint wordsqty)

{
  typword *max= ptr + wordsqty;
  while (ptr < max) {
    *ptr= 0;
    ptr++;
  }
}

/***********************************************************************/

