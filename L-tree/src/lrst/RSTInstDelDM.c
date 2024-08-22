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

typedef struct {
               Rint    ml, mh;            /* minimal #entries low, high side */
               Rint    mlow, mhigh;       /* minimum, maximum index */
               Rfloat  mu;                /* mid point (all curves) */
               Rfloat  grad;              /* parameters (straight lines) */
               Rfloat  a;                 /* parameters (parable) */
               Rfloat  sigma, y1, yscale; /* parameters (gaussian) */
               } typnormrec, *refnormrec;


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
 static void DirPlaneSweep(RSTREE R,
                           refnode DIRn,
                           IndexArray I,
                           Rfloat allarea,
                           Rint i,
                           Rint M,
                           refnormrec rNR,
                           Rfloat *validedge,
                           Rfloat *validvalue,
                           Rint *cnt);
static void DataPlaneSweep(RSTREE R,
                           refnode DATAn,
                           IndexArray I,
                           Rfloat allarea,
                           Rint i,
                           Rint M,
                           refnormrec rNR,
                           Rfloat *validedge,
                           Rfloat *validvalue,
                           Rint *cnt);
static void Eval1ExtNodesMBB(RSTREE R,
                             Rint level,
                             typinterval *newrect,
                             typinterval *allrect);
static void MergeMBBsToMBB(RSTREE R,
                           typinterval *from1,
                           typinterval *from2,
                           typinterval *to);
static void Eval1ShrNodesMBB(RSTREE R,
                             Rint level,
                             const typinterval *delRect,
                             typinterval *reducedrect);
static Rfloat Normal(Rint M,
                     Rint S_ind,
                     refnormrec rNR);
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
static void SplitMerged(RSTREE R,
                        refnode NSplit,
                        Rint level,
                        typinterval *reducedrect,
                        Rint newind,
                        Rpnint oldpagenr,
                        Rint numbentries);
static void CleanUpMerge(RSTREE R,
                         Rint level,
                         typinterval *reducedrect,
                         Rint newind,
                         Rpnint oldpagenr);
static void ShrinkTree(RSTREE R);


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
  void *ptrNSplit0, *ptrNSplitIj, *ptrNj, *ptrNSiblj;
  refnode n;
  IndexArray I= (*R).SADDD_I;
  Side sortside, currsortside, axissortside;
  Rint axis,
       axiscnt, cnt, cntlo, cnthi;
  Rfloat axisedge, edge, edgelo, edgehi,
         valuelo, valuehi,
         allarea;
  typnormrec NR;
  Rint d, j;
  
  /*
  fprintf(stderr,"DIR-Split: -----------------------------\n");
  */
  for (j= 0; j <= M; j++) {
    I[j]= j;
  }
  NR.mlow= m - 1;
  NR.mhigh= M - m + 1;
  
  allarea= 1.0;
  for (d= 0; d < (*R).numbofdim; d++) {
    allarea*= allrect[d].h - allrect[d].l;
  }
  
  ptrNSplit0= NSplit;
  ptrNSplit0+= (*R).Pent0;
  
  for (d= 0; d < (*R).numbofdim; d++) {
    currsortside= low;
    QSortIofEnts(0,M,d,low,ptrNSplit0,(*R).DIR.entLen,I);
    DirPlaneSweep(R,NSplit,I,allarea,d,M,&NR,&edgelo,&valuelo,&cntlo);
    currsortside= high;
    QSortIofEnts(0,M,d,high,ptrNSplit0,(*R).DIR.entLen,I);
    DirPlaneSweep(R,NSplit,I,allarea,d,M,&NR,&edgehi,&valuehi,&cnthi);
    
    if (valuehi < valuelo) {
      sortside= high;
      cnt= cnthi;
    }
    else {
      sortside= low;
      cnt= cntlo;
    }
    edge= edgelo + edgehi;
    if (d == 0) {
      axissortside= sortside;
      axis= d;
      axisedge= edge;
      axiscnt= cnt;
    }
    else if (edge < axisedge) {
      axissortside= sortside;
      axis= d;
      axisedge= edge;
      axiscnt= cnt;
    }
    /*
    fprintf(stderr,"-------------------------------------\n");
    */
  }
  /*
  fprintf(stderr,"----------------------------------------\n");
  */
  if (axis != (*R).numbofdim - 1 || axissortside != currsortside) {
    QSortIofEnts(0,M,axis,axissortside,ptrNSplit0,(*R).DIR.entLen,I);
  }
  /*
  fprintf(stderr,"DIR: axis, cnt: %3d %3d\n",axis,axiscnt);
  */
  n= (*R).L[level].N;
  
  (*n).s.nofentries= axiscnt;
  ptrNj= n;
  ptrNj+= (*R).Pent0;
  for (j= 0; j < (*n).s.nofentries; j++) {
    ptrNSplitIj= ptrNSplit0 + I[j] * (*R).DIR.entLen;
    memcpy(ptrNj,ptrNSplitIj,(*R).DIR.entLen);
    ptrNj+= (*R).DIR.entLen;
  }
  
  (*NSibl).s.nofentries= M+1-axiscnt;
  ptrNSiblj= NSibl;
  ptrNSiblj+= (*R).Pent0;
  for (j= 0; j < (*NSibl).s.nofentries; j++) {
    ptrNSplitIj= ptrNSplit0 + I[axiscnt+j] * (*R).DIR.entLen;
    memcpy(ptrNSiblj,ptrNSplitIj,(*R).DIR.entLen);
    ptrNSiblj+= (*R).DIR.entLen;
  }
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
  void *ptrNSplit0, *ptrNSplitIj, *ptrNj, *ptrNSiblj;
  refnode n;
  IndexArray I= (*R).SADDD_I;
  Side sortside, currsortside, axissortside;
  Rint axis,
       axiscnt, cnt, cntlo, cnthi;
  Rfloat axisedge, edge, edgelo, edgehi,
         valuelo, valuehi,
         allarea;
  typnormrec NR;
  Rint d, j;
  
  /*
  fprintf(stderr,"DATA-Split: ----------------------------\n");
  */
  for (j= 0; j <= M; j++) {
    I[j]= j;
  }
  NR.mlow= m - 1;
  NR.mhigh= M - m + 1;
  
  allarea= 1.0;
  for (d= 0; d < (*R).numbofdim; d++) {
    allarea*= allrect[d].h - allrect[d].l;
  }
  
  ptrNSplit0= NSplit;
  ptrNSplit0+= (*R).Pent0;
  
  for (d= 0; d < (*R).numbofdim; d++) {
    currsortside= low;
    QSortIofEnts(0,M,d,low,ptrNSplit0,(*R).DATA.entLen,I);
    DataPlaneSweep(R,NSplit,I,allarea,d,M,&NR,&edgelo,&valuelo,&cntlo);
    currsortside= high;
    QSortIofEnts(0,M,d,high,ptrNSplit0,(*R).DATA.entLen,I);
    DataPlaneSweep(R,NSplit,I,allarea,d,M,&NR,&edgehi,&valuehi,&cnthi);
    
    if (valuehi < valuelo) {
      sortside= high;
      cnt= cnthi;
    }
    else {
      sortside= low;
      cnt= cntlo;
    }
    edge= edgelo + edgehi;
    if (d == 0) {
      axissortside= sortside;
      axis= d;
      axisedge= edge;
      axiscnt= cnt;
    }
    else if (edge < axisedge) {
      axissortside= sortside;
      axis= d;
      axisedge= edge;
      axiscnt= cnt;
    }
    /*
    fprintf(stderr,"-------------------------------------\n");
    */
  }
  /*
  fprintf(stderr,"----------------------------------------\n");
  */
  if (axis != (*R).numbofdim - 1 || axissortside != currsortside) {
    QSortIofEnts(0,M,axis,axissortside,ptrNSplit0,(*R).DATA.entLen,I);
  }
  /*
  fprintf(stderr,"DATA: axis, cnt: %3d %3d\n",axis,axiscnt);
  */
  n= (*R).L[level].N;
  
  (*n).s.nofentries= axiscnt;
  ptrNj= n;
  ptrNj+= (*R).Pent0;
  for (j= 0; j < (*n).s.nofentries; j++) {
    ptrNSplitIj= ptrNSplit0 + I[j] * (*R).DATA.entLen;
    memcpy(ptrNj,ptrNSplitIj,(*R).DATA.entLen);
    ptrNj+= (*R).DATA.entLen;
  }
  
  (*NSibl).s.nofentries= M+1-axiscnt;
  ptrNSiblj= NSibl;
  ptrNSiblj+= (*R).Pent0;
  for (j= 0; j < (*NSibl).s.nofentries; j++) {
    ptrNSplitIj= ptrNSplit0 + I[axiscnt+j] * (*R).DATA.entLen;
    memcpy(ptrNSiblj,ptrNSplitIj,(*R).DATA.entLen);
    ptrNSiblj+= (*R).DATA.entLen;
  }
}

/***********************************************************************/

 static void DirPlaneSweep(RSTREE R,
                           refnode n,
                           IndexArray I,
                           Rfloat allarea,
                           Rint dim,
                           Rint M,
                           refnormrec rNR,
                           Rfloat *validedge,
                           Rfloat *validvalue,
                           Rint *cnt)

{
  void *ptrN0, *ptrNIj, *ptrRA0;
  void *ptrTmpRect;
  ValueArray edgearray= (*R).DDPS_edgearray;
  IntervalArray rectarray= (*R).DDPS_rectarray;
  refinterval re, leftre, rightre;
  typinterval *leftrect= (*R).DDPS_leftrect;
  typinterval *rightrect= (*R).DDPS_rightrect;
  Rfloat leftspace, rightspace, space,
         leftedge, rightedge, edge,
         ovlp,
         value, norm;
  Rint d, j;
  
  ptrN0= n;
  ptrN0+= (*R).Pent0;
  
  ptrRA0= rectarray;
  
  ptrTmpRect= ptrN0 + I[M] * (*R).DIR.entLen;
  CopyRect((*R).numbofdim,ptrTmpRect,rightrect);
  for (j= M; j > (*rNR).mlow; j--) {
    ptrNIj= ptrN0 + I[j] * (*R).DIR.entLen;
    re= ptrNIj;
    rightre= rightrect;
    for (d= 0; d < (*R).numbofdim; d++) {
      if ((*re).l < (*rightre).l) {
        (*rightre).l= (*re).l;
      }
      if ((*re).h > (*rightre).h) {
        (*rightre).h= (*re).h;
      }
      re++;
      rightre++;
    }
    if (j <= (*rNR).mhigh) {
      rightedge= 0.0;
      for (d= 0; d < (*R).numbofdim; d++) {
        rightedge+= rightrect[d].h - rightrect[d].l;
      }
      ptrTmpRect= ptrRA0 + j * (*R).SIZErect;
      CopyRect((*R).numbofdim,rightrect,ptrTmpRect);
      edgearray[j]= rightedge;
    }
  }
  /* fill array from M-m+1 down to m
     with rightrect, rightedge */
  ptrTmpRect= ptrN0 + I[0] * (*R).DIR.entLen;
  CopyRect((*R).numbofdim,ptrTmpRect,leftrect);
  for (j= 0; j < (*rNR).mhigh; j++) {
    ptrNIj= ptrN0 + I[j] * (*R).DIR.entLen;
    re= ptrNIj;
    leftre= leftrect;
    for (d= 0; d < (*R).numbofdim; d++) {
      if ((*re).l < (*leftre).l) {
        (*leftre).l= (*re).l;
      }
      if ((*re).h > (*leftre).h) {
        (*leftre).h= (*re).h;
      }
      re++;
      leftre++;
    }
    if (j >= (*rNR).mlow) {
      leftedge= 0.0;
      for (d= 0; d < (*R).numbofdim; d++) {
        leftedge+= leftrect[d].h - leftrect[d].l;
      }
      norm= Normal(M,j+1,rNR);
      if (j == (*rNR).mlow) {
        *validedge= leftedge+edgearray[j+1];
      }
      else {
        edge= leftedge+edgearray[j+1];
        *validedge= *validedge+edge;
      }
      
      ptrTmpRect= ptrRA0 + (j+1) * (*R).SIZErect;
      
      if (Overlaps((*R).numbofdim,leftrect,ptrTmpRect)) {
        GetOverlap((*R).numbofdim,leftrect,ptrTmpRect,&ovlp);
        if (j == (*rNR).mlow) {
          *validvalue= ovlp / norm;
          *cnt= j+1;
        }
        else {
          value= ovlp / norm;
          if (value < *validvalue) {
            *validvalue= value;
            *cnt= j+1;
          }
        }
      }
      else {
        leftspace= 1.0; rightspace= 1.0;
        leftre= leftrect;
        rightre= ptrTmpRect;
        for (d= 0; d < (*R).numbofdim; d++) {
          leftspace*= (*leftre).h - (*leftre).l;
          rightspace*= (*rightre).h - (*rightre).l;
          leftre++;
          rightre++;
        }
        space= leftspace + rightspace;
        if (j == (*rNR).mlow) {
          *validvalue= (space-allarea) * norm;
          *cnt= j+1;
        }
        else {
          value= (space-allarea) * norm;
          if (value < *validvalue) {
            *validvalue= value;
            *cnt= j+1;
          }
        }
      }
    }
  }
  /* Compute sum of the edges
     from leftedge[m-1]+rightedge[m]
     up to leftedge[M-m]+rightedge[M-m+1], -> axis;
     minimal area resp. overlap, -> *cnt. */
}

/***********************************************************************/

static void DataPlaneSweep(RSTREE R,
                           refnode n,
                           IndexArray I,
                           Rfloat allarea,
                           Rint dim,
                           Rint M,
                           refnormrec rNR,
                           Rfloat *validedge,
                           Rfloat *validvalue,
                           Rint *cnt)

{
  void *ptrN0, *ptrNIj, *ptrRA0;
  void *ptrTmpRect;
  ValueArray edgearray= (*R).DDPS_edgearray;
  IntervalArray rectarray= (*R).DDPS_rectarray;
  refinterval re, leftre, rightre;
  typinterval *leftrect= (*R).DDPS_leftrect;
  typinterval *rightrect= (*R).DDPS_rightrect;
  Rfloat leftspace, rightspace, space,
         leftedge, rightedge, edge,
         ovlp,
         value, norm;
  Rint d, j;
  
  ptrN0= n;
  ptrN0+= (*R).Pent0;
  
  ptrRA0= rectarray;
  
  ptrTmpRect= ptrN0 + I[M] * (*R).DATA.entLen;
  CopyRect((*R).numbofdim,ptrTmpRect,rightrect);
  for (j= M; j > (*rNR).mlow; j--) {
    ptrNIj= ptrN0 + I[j] * (*R).DATA.entLen;
    re= ptrNIj;
    rightre= rightrect;
    for (d= 0; d < (*R).numbofdim; d++) {
      if ((*re).l < (*rightre).l) {
        (*rightre).l= (*re).l;
      }
      if ((*re).h > (*rightre).h) {
        (*rightre).h= (*re).h;
      }
      re++;
      rightre++;
    }
    if (j <= (*rNR).mhigh) {
      rightedge= 0.0;
      for (d= 0; d < (*R).numbofdim; d++) {
        rightedge+= rightrect[d].h - rightrect[d].l;
      }
      ptrTmpRect= ptrRA0 + j * (*R).SIZErect;
      CopyRect((*R).numbofdim,rightrect,ptrTmpRect);
      edgearray[j]= rightedge;
    }
  }
  /* fill array from M-m+1 down to m
     with rightrect, rightedge */
  ptrTmpRect= ptrN0 + I[0] * (*R).DATA.entLen;
  CopyRect((*R).numbofdim,ptrTmpRect,leftrect);
  for (j= 0; j < (*rNR).mhigh; j++) {
    ptrNIj= ptrN0 + I[j] * (*R).DATA.entLen;
    re= ptrNIj;
    leftre= leftrect;
    for (d= 0; d < (*R).numbofdim; d++) {
      if ((*re).l < (*leftre).l) {
        (*leftre).l= (*re).l;
      }
      if ((*re).h > (*leftre).h) {
        (*leftre).h= (*re).h;
      }
      re++;
      leftre++;
    }
    if (j >= (*rNR).mlow) {
      leftedge= 0.0;
      for (d= 0; d < (*R).numbofdim; d++) {
        leftedge+= leftrect[d].h - leftrect[d].l;
      }
      norm= Normal(M,j+1,rNR);
      if (j == (*rNR).mlow) {
        *validedge= leftedge+edgearray[j+1];
      }
      else {
        edge= leftedge+edgearray[j+1];
        *validedge= *validedge+edge;
      }
      
      ptrTmpRect= ptrRA0 + (j+1) * (*R).SIZErect;
      
      if (Overlaps((*R).numbofdim,leftrect,ptrTmpRect)) {
        GetOverlap((*R).numbofdim,leftrect,ptrTmpRect,&ovlp);
        if (j == (*rNR).mlow) {
          *validvalue= ovlp / norm;
          *cnt= j+1;
        }
        else {
          value= ovlp / norm;
          if (value < *validvalue) {
            *validvalue= value;
            *cnt= j+1;
          }
        }
      }
      else {
        leftspace= 1.0; rightspace= 1.0;
        leftre= leftrect;
        rightre= ptrTmpRect;
        for (d= 0; d < (*R).numbofdim; d++) {
          leftspace*= (*leftre).h - (*leftre).l;
          rightspace*= (*rightre).h - (*rightre).l;
          leftre++;
          rightre++;
        }
        space= leftspace + rightspace;
        if (j == (*rNR).mlow) {
          *validvalue= (space-allarea) * norm;
          *cnt= j+1;
        }
        else {
          value= (space-allarea) * norm;
          if (value < *validvalue) {
            *validvalue= value;
            *cnt= j+1;
          }
        }
      }
    }
  }
  /* Compute sum of the edges
     from leftedge[m-1]+rightedge[m]
     up to leftedge[M-m]+rightedge[M-m+1], -> axis;
     minimal area resp. overlap, -> *cnt. */
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
/*    call:
      norm= Normal(M,j+1,rNR);
*/
static Rfloat Normal(Rint M,
                     Rint S_ind,          /* [1 .. M] */
                     refnormrec rNR)

{
  return 1.0;
}

/***********************************************************************/

void DeleteOneRec(RSTREE R, const typinterval *delRect)

{
  void *ptrN0, *ptrNlast, *ptrNdel;
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
      memcpy(ptrNdel,ptrNlast,(*R).DIR.entLen);
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
  void *ptrN0, *ptrNMerge0;
  void *ptrUpDIN0, *ptrUpDINptts;
  refnode NMerge= (*R).M_NMerge;
  refnode n;
  refnode upDIN;
  refcount c;
  IndexArray use0= (*R).M_use0;
  Rpnint oldpagenr;
  Rint numbentries, entrycount, unused, newind;
  boolean isdata;
  typinterval *reducedrect= (*R).M_reducedrect;
  
#ifndef COUNTS_OFF
  c= &(*R).count;
#endif
  upDIN= (*R).L[level+1].N;
  ptrUpDIN0= upDIN;
  ptrUpDIN0+= (*R).Pent0;
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
    ChooseMerge(R,
                reducedrect,
                (*R).L[level+1].E,
                upDIN,
                &unused,
                use0);
    newind= use0[0];
    oldpagenr= (*R).L[level].P;
    ptrUpDINptts= ptrUpDIN0 + newind * (*R).DIR.entLen + (*R).entPptts;
    (*R).L[level].P= *(refptrtosub)ptrUpDINptts;
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
      /* merge to N: */
      if (isdata) {
        entrycount= MergeDataNodes(R,NMerge,(*R).L[level].N);
      }
      else {
        entrycount= MergeDirNodes(R,NMerge,(*R).L[level].N);
      }
      CleanUpMerge(R,level,reducedrect,newind,oldpagenr);
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
      /* merge to NMerge: */
      if (isdata) {
        entrycount= MergeDataNodes(R,(*R).L[level].N,NMerge);
      }
      else {
        entrycount= MergeDirNodes(R,(*R).L[level].N,NMerge);
      }
      SplitMerged(R,NMerge,level,reducedrect,newind,oldpagenr,numbentries);
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

static void CleanUpMerge(RSTREE R,
                         Rint level,
                         typinterval *reducedrect,
                         Rint newind,
                         Rpnint oldpagenr)

{
  void *ptrUpNewRect;
  
  ptrUpNewRect= (*R).L[level+1].N;
  ptrUpNewRect+= (*R).Pent0 + newind * (*R).DIR.entLen;
  
  MergeMBBsToMBB(R,reducedrect,ptrUpNewRect,ptrUpNewRect);
  PutPageNr(R,oldpagenr,level);
  (*R).L[level].Modif= TRUE;
}

/***********************************************************************/

static void SplitMerged(RSTREE R,
                        refnode NSplit,
                        Rint level,
                        typinterval *reducedrect,
                        Rint newind,
                        Rpnint oldpagenr,
                        Rint numbentries)

{
  void *ptrUp0, *ptrUpRect, *ptrUpNewRect;
  refparameters par;
  refversion ver= &(*R).version._;
  refcount c;
  refnode NSibl= (*R).SM_NSibl;
  refnode upDIN;
  Rint splergeM, splergem;
  typinterval *allrect= (*R).SM_allrect;
  
  par= &(*R).parameters._;
#ifndef COUNTS_OFF
  c= &(*R).count;
#endif
  
  upDIN= (*R).L[level+1].N;
  ptrUp0= upDIN;
  ptrUp0+= (*R).Pent0;
  
  ptrUpRect= ptrUp0 + (*R).L[level+1].E * (*R).DIR.entLen;
  ptrUpNewRect= ptrUp0 + newind * (*R).DIR.entLen;
  
  MergeMBBsToMBB(R,
                 reducedrect,
                 ptrUpNewRect,
                 allrect);
  splergeM= numbentries - 1;
  splergem= ((*ver).minfillpercent * splergeM + 50) / 100;
  /* build the raw m just like for split during insertion */
  
  upDIN= (*R).L[level+1].N;
  
  if (level == 0) {
    if (splergem < DATA_MIN_ENTRY_QTY) {
      splergem= DATA_MIN_ENTRY_QTY;
    }
    if (numbentries - splergem > (*par).DATA.M) {
      splergem= numbentries - (*par).DATA.M;
    }
    /* correct the data level m as necessary */
    
    SplitAndDistributData(R,level,NSplit,NSibl,allrect,splergeM,splergem);
    
    EvalDataNodesMBB(R,NSibl,ptrUpRect);
    EvalDataNodesMBB(R,(*R).L[level].N,ptrUpNewRect);
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
    
    SplitAndDistributDir(R,level,NSplit,NSibl,allrect,splergeM,splergem);
    
    EvalDirNodesMBB(R,NSibl,ptrUpRect);
    EvalDirNodesMBB(R,(*R).L[level].N,ptrUpNewRect);
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

