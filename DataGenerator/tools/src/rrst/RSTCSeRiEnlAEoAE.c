/* ----- RSTChooseSub.c ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//


#include "RSTChooseSub.h"
#include "RSTUtil.h"
#include "RSTMemAlloc.h"


/* declarations */

static void FindLeastArea(RSTREE R,
                          refnode DIRnode,
                          Rint numbexam,
                          Rint I[]);
static void FindLeastEdge(RSTREE R,
                          refnode DIRnode,
                          Rint numbexam,
                          Rint I[]);
static void GetEnlRectsEdgeEnls(RSTREE R,
                                typinterval *newrect,
                                refnode DIRnode,
                                typinterval *EnlRects,
                                Rfloat Enlarge[]);
static boolean ExistsRec0OverlapEnl(RSTREE R,
                                    refnode DIRnode,
                                    typinterval *EnlRects,
                                    Rint numbexam,
                                    Rint I[],
                                    byte X[],
                                    Rfloat OvlpEnls[],
                                    Rint i);
static boolean ExistsRec0OvlpEdgeEnl(RSTREE R,
                                     refnode DIRnode,
                                     typinterval *EnlRects,
                                     Rint numbexam,
                                     Rint I[],
                                     byte X[],
                                     Rfloat OvlpEnls[],
                                     Rint i);
static Rint FlaggedIofLeastRfloat(byte X[],
                                  Rint begin,
                                  Rint end,
                                  Rfloat val[],
                                  Rint I[],
                                  Rint *PminusQ);
static boolean ExistArea0RectsInN(RSTREE R,
                                  refnode DIRnode,
                                  Rint I[],
                                  Rint numbexam);
/* Returns TRUE if node contains at least one rectangle whose area is 0,
   because its extension is 0 in at least one dimension.
   Ckeck-range: i= 0 .. numbexam, (*node).entries[I[i]].rect. */

static boolean ExistArea0RectsInA(RSTREE R,
                                  typinterval *rects,
                                  Rint I[],
                                  Rint numbexam);
/* Returns TRUE if rect[] contains at least one rectangle whose area is 0,
   because its extension is 0 in at least one dimension.
   Ckeck-range: i= 0 .. numbexam, rect[I[i]]. */
/* ------------------------------------------------------------------- */
/* functions repeated here to be available for inlining: */
/* ------------------------------------------------------------------- */
/* identical to Covers() in RSTUtil: */
static boolean CS_Covers(Rint numbOfDim,
                         const typinterval *intvC,
                         const typinterval *intv);
/* identical to ClearVector() in RSTUtil: */
static void CS_ClearVector(typword *ptr, Rint wordsqty);

/***********************************************************************/
/* ----- ChooseSubtree version (incl. declarations) below ------------ */
/***********************************************************************/
static void FindOvlpEdgeInvolvedMaxInd(RSTREE R,
                                       refnode node,
                                       typinterval *EnlRects,
                                       Rint I[],
                                       Rint *numbinv);

/***********************************************************************/
/* NOTE: Functions implementing the "some fit" case may exchange but must not
         destroy elements of I.
   THIS NOTE IS OBSOLETE SINCE THE NODES OF THE PATH MAY BE LINKS !!
*/

void ChooseSubtree(RSTREE R,
                   typinterval *newrect,
                   Rint level,
                   refnode node,
                   Rint *nCov,
                   Rint I[])

{
  void *ptrNi;
  Rint entLen, nofent;
  Rint i;
  IntervalArray EnlRects= (*R).CSCM_EnlRects;
  ValueArray EdgeEnls= (*R).CSCM_EdgeEnls;
  ValueArray OvlpEnls= (*R).CSCM_OvlpEnls;
  uWordsBytes *X= (*R).CSCM_X;
  Rint P;
  Rint PminusQ;
  boolean exists0ovlpEnl;
#ifndef COUNTS_OFF
  refcount c= &(*R).count;
  if ((*c).on) {
    (*c).CS_Call++;
  }
#endif
  
  *nCov= 0;
  ptrNi= node;
  ptrNi+= (*R).Pent0;
  entLen= (*R).DIR.entLen;
  nofent= (*node).s.nofentries;
  for (i= 0; i < nofent; i++) {
    if (CS_Covers((*R).numbofdim,ptrNi,newrect)) {
      I[*nCov]= i;
      (*nCov)++;
    }
    ptrNi+= entLen;
  }
  if (*nCov == 0) {
    /* no fit */
    GetEnlRectsEdgeEnls(R,newrect,node,EnlRects,EdgeEnls);
    for (i= 0; i < nofent; i++) {
      I[i]= i;
    }
    QSortIofRfloats(0,nofent-1,EdgeEnls,I);
    P= 1;
    FindOvlpEdgeInvolvedMaxInd(R,node,EnlRects,I,&P);
    if (P > 1) {
#ifndef COUNTS_OFF
      if ((*c).on) {
        (*c).CS_P+= P;
        if (P > (*c).CS_MaxP) {
          (*c).CS_MaxP= P;
        }
      }
#endif
      CS_ClearVector((*X).words,(*R).DIReqvWords_Mp1);
      if (ExistArea0RectsInA(R,EnlRects,I,P)) {
#ifndef COUNTS_OFF
        if ((*c).on) {
          (*c).CS_Area0++;
        }
#endif
        exists0ovlpEnl= ExistsRec0OvlpEdgeEnl(R,node,EnlRects,P,I,(*X)._,OvlpEnls,0);
      }
      else {
        exists0ovlpEnl= ExistsRec0OverlapEnl(R,node,EnlRects,P,I,(*X)._,OvlpEnls,0);
      }
      if (! exists0ovlpEnl) {
        I[0]= FlaggedIofLeastRfloat((*X)._,0,P-1,OvlpEnls,I,&PminusQ);
#ifndef COUNTS_OFF
        if ((*c).on) {
          (*c).CS_PminusQ+= PminusQ;
        }
#endif
      }
      else {
        /* merely count */
#ifndef COUNTS_OFF
        if ((*c).on) {
          (*c).CS_AfterwOvlpEnl0++;
        }
#endif
      }
#ifndef COUNTS_OFF
      if ((*c).on) {
        (*c).CS_OvlpEnlOpt++;
      }
#endif
    }
    else {
      /* merely count */
#ifndef COUNTS_OFF
      if ((*c).on) {
        (*c).CS_P1OvlpEnl0++;
      }
#endif
    }
#ifndef COUNTS_OFF
    if ((*c).on) {
      (*c).CS_NoFit++;
    }
#endif
  }
  else if (*nCov == 1) {
    /* unique fit */
#ifndef COUNTS_OFF
    if ((*c).on) {
      (*c).CS_UniFit++;
    }
#endif
  }
  else {
    /* some fit */
    if (ExistArea0RectsInN(R,node,I,*nCov)) {
#ifndef COUNTS_OFF
      if ((*c).on) {
        (*c).CS_Area0++;
      }
#endif
      FindLeastEdge(R,node,*nCov,I);
    }
    else {
      FindLeastArea(R,node,*nCov,I);
    }
#ifndef COUNTS_OFF
    if ((*c).on) {
      (*c).CS_SomeFit++;
    }
#endif
  }
}

/***********************************************************************/
/* *numbinv assumed to be at least 1 */

static void FindOvlpEdgeInvolvedMaxInd(RSTREE R,
                                       refnode node,
                                       typinterval *EnlRects,
                                       Rint I[],
                                       Rint *numbinv)

{
  void *ptrN0, *ptrNI0, *ptrNIj, *ptrEI0;
  Rint entLen;
  Rint j;
  Rfloat enlarge;
  
  ptrN0= node;
  ptrN0+= (*R).Pent0;
  entLen= (*R).DIR.entLen;
  ptrNI0= ptrN0 + I[0] * entLen;
  ptrEI0= EnlRects;
  ptrEI0+= I[0] * (*R).SIZErect;
  j= (*node).s.nofentries - 1;
  while (j >= *numbinv) {
    ptrNIj= ptrN0 + I[j] * entLen;
    enlarge= OvlpEdgeEnl(R,
                         ptrNI0,
                         ptrEI0,
                         ptrNIj);
    if (enlarge > 0.0) {
      *numbinv= j + 1;
    }
    j--;
  }
}

/***********************************************************************/
/* ------------ ChooseMerge (incl. declarations) below --------------- */
/***********************************************************************/

static void CM_FindOvlpEdgeInvolvedMaxInd(RSTREE R,
                                          refnode DIRnode,
                                          typinterval *EnlRects,
                                          Rint I[],
                                          Rint *numbinv);

/***********************************************************************/
/* This ChooseMerge algorithm is derived from the most efficient ChooseSubtree
   algorithm (eRiEnlAEoAE); it slightly differs from it as follows:
   newrect is one of the rectangles of the considered node itself, thus the
   concerning entry has to be skipped in the search.
   - Covers-query: just skip.
   - Initialization of index array I: shift I[skip+1] .. I[nofentries-1]
                                        to  I[skip]  ..  I[nofentries-2], and
     (not really necessary) set I[nofentries-1]= skip. Thereafter merely
     consider I[0] .. I[nofentries-2].
   - Sort: merely consider I[0] .. I[nofentries-2].
   - FindOvlpEdgeInvolvedMaxInd exchanged by CM_FindOvlpEdgeInvolvedMaxInd.
     The sole difference is that
     FindOvlpEdgeInvolvedMaxInd initially examines entries[I[nofentries-1]] but
     CM_FindOvlpEdgeInvolvedMaxInd initially examines entries[I[nofentries-2]].
   - Functions called thereafter do not need modifications because they only
     examine the first P or nCov indices anyway.
*/

void ChooseMerge(RSTREE R,
                 typinterval *newrect,
                 Rint skip,
                 refnode node,
                 Rint *nCov,
                 Rint I[])

{
  void *ptrNi;
  Rint entLen, nofent;
  Rint i, ind;
  IntervalArray EnlRects= (*R).CSCM_EnlRects;
  ValueArray EdgeEnls= (*R).CSCM_EdgeEnls;
  ValueArray OvlpEnls= (*R).CSCM_OvlpEnls;
  uWordsBytes *X= (*R).CSCM_X;
  Rint P;
  Rint PminusQ;
  boolean exists0ovlpEnl;
#ifndef COUNTS_OFF
  refcount c= &(*R).count;
  if ((*c).on) {
    (*c).CS_Call++;
  }
#endif
  
  *nCov= 0;
  ptrNi= node;
  ptrNi+= (*R).Pent0;
  entLen= (*R).DIR.entLen;
  nofent= (*node).s.nofentries;
  for (i= 0; i < nofent; i++) {
    if (i != skip && CS_Covers((*R).numbofdim,ptrNi,newrect)) {
      I[*nCov]= i;
      (*nCov)++;
    }
    ptrNi+= entLen;
  }
  if (*nCov == 0) {
    /* no fit */
    GetEnlRectsEdgeEnls(R,newrect,node,EnlRects,EdgeEnls);
    ind= 0;
    for (i= 0; i < nofent; i++) {
      if (i != skip) {
        I[ind]= i;
        ind++;
      }
      else {
        I[nofent-1]= i;
      }
    }
    QSortIofRfloats(0,nofent-2,EdgeEnls,I);
    P= 1;
    CM_FindOvlpEdgeInvolvedMaxInd(R,node,EnlRects,I,&P);
    if (P > 1) {
#ifndef COUNTS_OFF
      if ((*c).on) {
        (*c).CS_P+= P;
        if (P > (*c).CS_MaxP) {
          (*c).CS_MaxP= P;
        }
      }
#endif
      CS_ClearVector((*X).words,(*R).DIReqvWords_Mp1);
      if (ExistArea0RectsInA(R,EnlRects,I,P)) {
#ifndef COUNTS_OFF
        if ((*c).on) {
          (*c).CS_Area0++;
        }
#endif
        exists0ovlpEnl= ExistsRec0OvlpEdgeEnl(R,node,EnlRects,P,I,(*X)._,OvlpEnls,0);
      }
      else {
        exists0ovlpEnl= ExistsRec0OverlapEnl(R,node,EnlRects,P,I,(*X)._,OvlpEnls,0);
      }
      if (! exists0ovlpEnl) {
        I[0]= FlaggedIofLeastRfloat((*X)._,0,P-1,OvlpEnls,I,&PminusQ);
#ifndef COUNTS_OFF
        if ((*c).on) {
          (*c).CS_PminusQ+= PminusQ;
        }
#endif
      }
      else {
        /* merely count */
#ifndef COUNTS_OFF
        if ((*c).on) {
          (*c).CS_AfterwOvlpEnl0++;
        }
#endif
      }
#ifndef COUNTS_OFF
      if ((*c).on) {
        (*c).CS_OvlpEnlOpt++;
      }
#endif
    }
    else {
      /* merely count */
#ifndef COUNTS_OFF
      if ((*c).on) {
        (*c).CS_P1OvlpEnl0++;
      }
#endif
    }
#ifndef COUNTS_OFF
    if ((*c).on) {
      (*c).CS_NoFit++;
    }
#endif
  }
  else if (*nCov == 1) {
    /* unique fit */
#ifndef COUNTS_OFF
    if ((*c).on) {
      (*c).CS_UniFit++;
    }
#endif
  }
  else {
    /* some fit */
    if (ExistArea0RectsInN(R,node,I,*nCov)) {
#ifndef COUNTS_OFF
      if ((*c).on) {
        (*c).CS_Area0++;
      }
#endif
      FindLeastEdge(R,node,*nCov,I);
    }
    else {
      FindLeastArea(R,node,*nCov,I);
    }
#ifndef COUNTS_OFF
    if ((*c).on) {
      (*c).CS_SomeFit++;
    }
#endif
  }
}

/***********************************************************************/
/* *numbinv assumed to be at least 1 */

static void CM_FindOvlpEdgeInvolvedMaxInd(RSTREE R,
                                          refnode node,
                                          typinterval *EnlRects,
                                          Rint I[],
                                          Rint *numbinv)

{
  void *ptrN0, *ptrNI0, *ptrNIj, *ptrEI0;
  Rint entLen;
  Rint j;
  Rfloat enlarge;
  
  ptrN0= node;
  ptrN0+= (*R).Pent0;
  entLen= (*R).DIR.entLen;
  ptrNI0= ptrN0 + I[0] * entLen;
  ptrEI0= EnlRects;
  ptrEI0+= I[0] * (*R).SIZErect;
  j= (*node).s.nofentries - 2;
  while (j >= *numbinv) {
    ptrNIj= ptrN0 + I[j] * entLen;
    enlarge= OvlpEdgeEnl(R,
                         ptrNI0,
                         ptrEI0,
                         ptrNIj);
    if (enlarge > 0.0) {
      *numbinv= j + 1;
    }
    j--;
  }
}

/***********************************************************************/
/* NOTE: Functions implementing the "some fit" case may exchange but must not
         destroy elements of I.
   THIS NOTE IS OBSOLETE SINCE THE NODES OF THE PATH MAY BE LINKS !!
*/

static void FindLeastArea(RSTREE R,
                          refnode node,
                          Rint numbexam,
                          Rint I[])

{
  void *ptrN0, *ptrNIi;
  Rint entLen, numbOfDim;
  Rint i, d, found, help;
  Rfloat area, leastarea;
  refinterval intv;
  
  ptrN0= node;
  ptrN0+= (*R).Pent0;
  entLen= (*R).DIR.entLen;
  numbOfDim= (*R).numbofdim;
  for (i= 0; i < numbexam; i++) {
    
    ptrNIi= ptrN0 + I[i] * entLen;
    intv= ptrNIi;
    
    area= 1.0;
    for (d= 0; d < numbOfDim; d++) {
      area*= (*intv).h - (*intv).l;
      intv++;
    }
    if (i == 0) {
      leastarea= area;
      found= i;
    }
    else {
      if (area < leastarea) {
        leastarea= area;
        found= i;
      }
    }
  }
  if (found > 0) {
    help= I[0];
    I[0]= I[found];
    I[found]= help;
  }
}

/***********************************************************************/
/* NOTE: Functions implementing the "some fit" case may exchange but must not
         destroy elements of I.
   THIS NOTE IS OBSOLETE SINCE THE NODES OF THE PATH MAY BE LINKS !!
*/

static void FindLeastEdge(RSTREE R,
                          refnode node,
                          Rint numbexam,
                          Rint I[])

{
  void *ptrN0, *ptrNIi;
  Rint entLen, numbOfDim;
  Rint i, d, found, help;
  Rfloat edge, leastedge;
  refinterval intv;
  
  ptrN0= node;
  ptrN0+= (*R).Pent0;
  entLen= (*R).DIR.entLen;
  numbOfDim= (*R).numbofdim;
  for (i= 0; i < numbexam; i++) {
    
    ptrNIi= ptrN0 + I[i] * entLen;
    intv= ptrNIi;
    
    edge= 0.0;
    for (d= 0; d < numbOfDim; d++) {
      edge+= (*intv).h - (*intv).l;
      intv++;
    }
    if (i == 0) {
      leastedge= edge;
      found= i;
    }
    else {
      if (edge < leastedge) {
        leastedge= edge;
        found= i;
      }
    }
  }
  if (found > 0) {
    help= I[0];
    I[0]= I[found];
    I[found]= help;
  }
}

/***********************************************************************/

static void GetEnlRectsEdgeEnls(RSTREE R,
                                typinterval *newrect,
                                refnode node,
                                typinterval *EnlRects,
                                Rfloat Enlarge[])

{
  void *ptrNi;
  Rint entLen, nofent, numbOfDim;
  Rint i, d;
  Rfloat edge, newedge;
  refinterval newReIntv, nodReIntv, enlReIntv;
  
  ptrNi= node;
  ptrNi+= (*R).Pent0;
  enlReIntv= EnlRects;
  entLen= (*R).DIR.entLen;
  nofent= (*node).s.nofentries;
  numbOfDim= (*R).numbofdim;
  for (i= 0; i < nofent; i++) {
    
    newReIntv= newrect;
    nodReIntv= ptrNi;
    
    edge= 0.0; newedge= 0.0;
    for (d= 0; d < numbOfDim; d++) {
      *enlReIntv= *nodReIntv;      
      edge+= (*enlReIntv).h - (*enlReIntv).l;
      if ((*enlReIntv).l > (*newReIntv).l) {
        (*enlReIntv).l= (*newReIntv).l;
      }
      if ((*enlReIntv).h < (*newReIntv).h) {
        (*enlReIntv).h= (*newReIntv).h;
      }
      newedge+= (*enlReIntv).h - (*enlReIntv).l;
      nodReIntv++; enlReIntv++; newReIntv++;
    } /* store possibly enlarged rectangle, calculate old and new edge */
    Enlarge[i]= newedge - edge;
    ptrNi+= entLen;
  }
}

/***********************************************************************/

static boolean ExistsRec0OverlapEnl(RSTREE R,
                                    refnode node,
                                    typinterval *EnlRects,
                                    Rint numbexam,
                                    Rint I[],
                                    byte X[],
                                    Rfloat OvlpEnls[],
                                    Rint i)

{
  void *ptrN0, *ptrNIi, *ptrNIj, *ptrE0, *ptrEIi;
  Rint entLen;
  Rint j;
  boolean found;
  Rfloat enlarge, totenlarge;

  X[i]= 1;
  totenlarge= 0.0;
  found= FALSE;
  ptrN0= node;
  ptrN0+= (*R).Pent0;
  ptrE0= EnlRects;
  entLen= (*R).DIR.entLen;
  for (j= 0; j < numbexam; j++) {
    if (j != i) {
      ptrNIi= ptrN0 + I[i] * entLen;
      ptrNIj= ptrN0 + I[j] * entLen;
      ptrEIi= ptrE0 + I[i] * (*R).SIZErect;
      enlarge= OverlapEnl(R,
                          ptrNIi,
                          ptrEIi,
                          ptrNIj);
      if (enlarge > 0.0) {
        totenlarge+= enlarge;
        if (X[j] == 0) {
          found= ExistsRec0OverlapEnl(R,node,EnlRects,numbexam,I,X,OvlpEnls,j);
          if (found) {
            break;
          }
        }
      }
    }
  }
  if (! found) {
    if (totenlarge == 0.0) {
      found= TRUE;
      I[0]= I[i];
    }
    else {
      OvlpEnls[I[i]]= totenlarge;
    }
  }
  return found;
}

/***********************************************************************/

static boolean ExistsRec0OvlpEdgeEnl(RSTREE R,
                                     refnode node,
                                     typinterval *EnlRects,
                                     Rint numbexam,
                                     Rint I[],
                                     byte X[],
                                     Rfloat OvlpEnls[],
                                     Rint i)

{
  void *ptrN0, *ptrNIi, *ptrNIj, *ptrE0, *ptrEIi;
  Rint entLen;
  Rint j;
  boolean found;
  Rfloat enlarge, totenlarge;

  X[i]= 1;
  totenlarge= 0.0;
  found= FALSE;
  ptrN0= node;
  ptrN0+= (*R).Pent0;
  ptrE0= EnlRects;
  entLen= (*R).DIR.entLen;
  for (j= 0; j < numbexam; j++) {
    if (j != i) {
      ptrNIi= ptrN0 + I[i] * entLen;
      ptrNIj= ptrN0 + I[j] * entLen;
      ptrEIi= ptrE0 + I[i] * (*R).SIZErect;
      enlarge= OvlpEdgeEnl(R,
                           ptrNIi,
                           ptrEIi,
                           ptrNIj);
      if (enlarge > 0.0) {
        totenlarge+= enlarge;
        if (X[j] == 0) {
          found= ExistsRec0OvlpEdgeEnl(R,node,EnlRects,numbexam,I,X,OvlpEnls,j);
          if (found) {
            break;
          }
        }
      }
    }
  }
  if (! found) {
    if (totenlarge == 0.0) {
      found= TRUE;
      I[0]= I[i];
    }
    else {
      OvlpEnls[I[i]]= totenlarge;
    }
  }
  return found;
}

/***********************************************************************/

static Rint FlaggedIofLeastRfloat(byte X[],
                                  Rint begin,
                                  Rint end,
                                  Rfloat val[],
                                  Rint I[],
                                  Rint *PminusQ)

{
  Rint i, index;
  Rfloat leastval;
  
  *PminusQ= 0;
  while (X[begin] == 0 && begin < end) {
    begin++;
    (*PminusQ)++;
  }
  I+= begin;
  leastval= val[*I];
  index= *I;
  I++;
  for (i= begin+1; i <= end; i++) {
    if (X[i] == 1) {
      if (val[*I] < leastval) {
        leastval= val[*I];
        index= *I;
      }
    }
    else {
      (*PminusQ)++;
    }
    I++;
  }
  return index;
}

/***********************************************************************/

static boolean ExistArea0RectsInN(RSTREE R,
                                  refnode node,
                                  Rint I[],
                                  Rint numbexam)

{
  void *ptrN0, *ptrNIi;
  Rint entLen;
  Rint i;
  
  ptrN0= node;
  ptrN0+= (*R).Pent0;
  entLen= (*R).DIR.entLen;
  for (i= 0; i < numbexam; i++) {
    ptrNIi= ptrN0 + I[i] * entLen;
    if (IsArea0((*R).numbofdim,ptrNIi)) {
      return TRUE;
    }
  }
  return FALSE;
}

/***********************************************************************/

static boolean ExistArea0RectsInA(RSTREE R,
                                  typinterval *rects,
                                  Rint I[],
                                  Rint numbexam)

{
  void *ptrRects, *ptrRectIi;
  Rint i;
  
  ptrRects= rects;
  for (i= 0; i < numbexam; i++) {
    ptrRectIi= ptrRects + I[i] * (*R).SIZErect;
    if (IsArea0((*R).numbofdim,ptrRectIi)) {
      return TRUE;
    }
  }
  return FALSE;
}

/* ------------------------------------------------------------------- */
/* functions repeated here to be available for inlining: */
/* ------------------------------------------------------------------- */
/* identical to Covers() in RSTUtil: */

static boolean CS_Covers(Rint numbOfDim,
                         const typinterval *intvC,
                         const typinterval *intv)

{
  Rint d;
  
  d= 0;
  do {
    if ((*intvC).l > (*intv).l || (*intvC).h < (*intv).h) {
      return FALSE;
    }
    d++; intvC++; intv++;
  } while (d < numbOfDim);
  return TRUE;
}

/***********************************************************************/
/* identical to ClearVector() in RSTUtil: */

static void CS_ClearVector(typword *ptr, Rint wordsqty)

{
  typword *max= ptr + wordsqty;
  while (ptr < max) {
    *ptr= 0;
    ptr++;
  }
}

/***********************************************************************/

