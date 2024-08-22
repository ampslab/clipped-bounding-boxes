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
static void GetEnlRectsAreaEnls(RSTREE R,
                                typinterval *newrect,
                                refnode DIRnode,
                                typinterval *EnlRects,
                                Rfloat Enlarge[]);
static void FindLeastAreaEnl(RSTREE R,
                             typinterval *newrect,
                             refnode DIRnode,
                             Rint I[]);
/* ------------------------------------------------------------------- */
/* functions repeated here to be available for inlining: */
/* ------------------------------------------------------------------- */
/* identical to Covers() in RSTUtil: */
static boolean CS_Covers(Rint numbOfDim,
                         const typinterval *intvC,
                         const typinterval *intv);

/***********************************************************************/
/* ----- ChooseSubtree version (incl. declarations) below ------------ */
/***********************************************************************/
static void FindLeastOvlpEnl(RSTREE R,
                             refnode node,
                             typinterval *EnlRects,
                             Rint numbexam,
                             Rint I[]);

/* NOTE: pM-version of FindLeastOvlpEnl! let: p = numbexam, M = nofentries.
   Cost here: p * M. */

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
# define ChooseSubtree_p 32
  
  void *ptrNi;
  Rint entLen, nofent;
  Rint i, P;
  IntervalArray EnlRects= (*R).CSCM_EnlRects;
  ValueArray Enls= (*R).CSCM_Enls;
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
    if (level == 1) { /* Subtrees are leaves */
      GetEnlRectsAreaEnls(R,newrect,node,EnlRects,Enls);
      for (i= 0; i < nofent; i++) {
        I[i]= i;
      }
      QSortIofRfloats(0,nofent-1,Enls,I);
      if (ChooseSubtree_p < nofent) {
        P= ChooseSubtree_p;
      }
      else {
        P= nofent;
      }
      FindLeastOvlpEnl(R,node,EnlRects,P,I);
    }
    else {
      FindLeastAreaEnl(R,newrect,node,I);
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
    FindLeastArea(R,node,*nCov,I);
#ifndef COUNTS_OFF
    if ((*c).on) {
      (*c).CS_SomeFit++;
    }
#endif
  }
# undef ChooseSubtree_p
}

/***********************************************************************/
/* NOTE: pM-version of FindLeastOvlpEnl! let: p = numbexam, M = nofentries.
   Cost here: p * M. */

static void FindLeastOvlpEnl(RSTREE R,
                             refnode node,
                             typinterval *EnlRects,
                             Rint numbexam,
                             Rint I[])

{
  void *ptrN0, *ptrE0, *ptrNIi, *ptrNIj, *ptrEIi;
  Rint entLen, nofent;
  Rint i, j, found;
  Rfloat enlarge, leastenlarge;
#ifndef COUNTS_OFF
  refcount c= &(*R).count;
#endif
  
  ptrN0= node;
  ptrN0+= (*R).Pent0;
  ptrE0= EnlRects;
  entLen= (*R).DIR.entLen;
  nofent= (*node).s.nofentries;
  if (numbexam > 1) {
    i= 0;
    do {
      enlarge= 0.0;
      for (j= 0; j < nofent; j++) {
        if (j != i) {
          ptrNIi= ptrN0 + I[i] * entLen;
          ptrNIj= ptrN0 + I[j] * entLen;
          ptrEIi= ptrE0 + I[i] * (*R).SIZErect;
          enlarge+= OverlapEnl(R,
                               ptrNIi,
                               ptrEIi,
                               ptrNIj);
        }
      }
      if (i == 0) {
        leastenlarge= enlarge;
        found= i;
      }
      else {
        if (enlarge < leastenlarge) {
          leastenlarge= enlarge;
          found= i;
        }
      }
      i++;
    } while (enlarge > 0.0 && i < numbexam);
    if (found > 0) {
      I[0]= I[found];
    }
#ifndef COUNTS_OFF
    if ((*c).on) {
      (*c).CS_OvlpEnlOpt++;
      if (enlarge == 0.0) {
        (*c).CS_AfterwOvlpEnl0++;
      }
    }
#endif
  }
}

/***********************************************************************/
/* ------------ ChooseMerge (incl. declarations) below --------------- */
/***********************************************************************/

/***********************************************************************/
/* This ChooseMerge algorithm is derived from the ChooseSubtree algorithm in
   this source (alop32); it slightly differs from it as follows:
   Strategy:
   No distinction between leaves and inner nodes. Overlap optimization is
   always performed (one "if" less).
   Programming:
   newrect is one of the rectangles of the considered node itself, thus the
   concerning entry has to be skipped in the search.
   - Covers-query: just skip.
   - Initialization of index array I: shift I[skip+1] .. I[nofentries-1]
                                        to  I[skip]  ..  I[nofentries-2], and
     (not really necessary) set I[nofentries-1]= skip. Thereafter merely
     consider I[0] .. I[nofentries-2].
   - Sort: merely consider I[0] .. I[nofentries-2].
   - P: maximum is (*node).s.nofentries - 1 (reduced by 1).
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
# define ChooseSubtree_p 32
  
  void *ptrNi;
  Rint entLen, nofent;
  Rint i, ind, P;
  IntervalArray EnlRects= (*R).CSCM_EnlRects;
  ValueArray Enls= (*R).CSCM_Enls;
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
    GetEnlRectsAreaEnls(R,newrect,node,EnlRects,Enls);
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
    QSortIofRfloats(0,nofent-2,Enls,I);
    if (ChooseSubtree_p < nofent - 1) {
      P= ChooseSubtree_p;
    }
    else {
      P= nofent - 1;
    }
    FindLeastOvlpEnl(R,node,EnlRects,P,I);
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
    FindLeastArea(R,node,*nCov,I);
#ifndef COUNTS_OFF
    if ((*c).on) {
      (*c).CS_SomeFit++;
    }
#endif
  }

# undef ChooseSubtree_p
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

static void GetEnlRectsAreaEnls(RSTREE R,
                                typinterval *newrect,
                                refnode node,
                                typinterval *EnlRects,
                                Rfloat Enlarge[])

{
  void *ptrNi;
  Rint entLen, nofent, numbOfDim;
  Rint i, d;
  Rfloat area, newarea;
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
    
    area= 1.0; newarea= 1.0;
    for (d= 0; d < numbOfDim; d++) {
      *enlReIntv= *nodReIntv;      
      area*= (*enlReIntv).h - (*enlReIntv).l;
      if ((*enlReIntv).l > (*newReIntv).l) {
        (*enlReIntv).l= (*newReIntv).l;
      }
      if ((*enlReIntv).h < (*newReIntv).h) {
        (*enlReIntv).h= (*newReIntv).h;
      }
      newarea*= (*enlReIntv).h - (*enlReIntv).l;
      nodReIntv++; enlReIntv++; newReIntv++;
    } /* store possibly enlarged rectangle, calculate old and new area */
    Enlarge[i]= newarea - area;
    ptrNi+= entLen;
  }
}

/***********************************************************************/

static void FindLeastAreaEnl(RSTREE R,
                             typinterval *newrect,
                             refnode node,
                             Rint I[])

{
  void *ptrNi;
  Rint entLen, nofent, numbOfDim;
  Rint i, d;
  Rfloat area, newarea, enlarge, leastenlarge;
  typatomkey low, high;
  refinterval newReIntv, nodReIntv;
  
  ptrNi= node;
  ptrNi+= (*R).Pent0;
  entLen= (*R).DIR.entLen;
  nofent= (*node).s.nofentries;
  numbOfDim= (*R).numbofdim;
  for (i= 0; i < nofent; i++) {
    
    newReIntv= newrect;
    nodReIntv= ptrNi;
    
    area= 1.0; newarea= 1.0;
    for (d= 0; d < numbOfDim; d++) {
      low= (*nodReIntv).l;
      high= (*nodReIntv).h;
      area*= high - low;
      if (low > (*newReIntv).l) {
        low= (*newReIntv).l;
      }
      if (high < (*newReIntv).h) {
        high= (*newReIntv).h;
      }
      newarea*=  high - low;
      nodReIntv++; newReIntv++;
    }
    enlarge= newarea - area;
    if (i == 0) {
      leastenlarge= enlarge;
      I[0]= 0;
    }
    else {
      if (enlarge < leastenlarge) {
        leastenlarge= enlarge;
        I[0]= i;
      }
    }
    ptrNi+= entLen;
  }
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

