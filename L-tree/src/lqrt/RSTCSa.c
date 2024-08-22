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
    FindLeastAreaEnl(R,newrect,node,I);
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
}

/***********************************************************************/
/* ------------ ChooseMerge (incl. declarations) below --------------- */
/***********************************************************************/

static void CM_FindLeastAreaEnl(RSTREE R,
                                typinterval *newrect,
                                refnode DIRnode,
                                Rint skip,
                                Rint I[]);

/***********************************************************************/
/* This ChooseMerge algorithm is derived from the ChooseSubtree algorithm in
   this source (a); it slightly differs from it as follows:
   newrect is one of the rectangles of the considered node itself, thus the
   concerning entry has to be skipped in the search.
   - Covers-query: just skip.
   - FindLeastAreaEnl: replaced by CM_FindLeastAreaEnl which considers skip.
   - Functions called thereafter do not need modifications because they only
     examine the first nCov indices anyway.
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
  Rint i;
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
    CM_FindLeastAreaEnl(R,newrect,node,skip,I);
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
}

/***********************************************************************/

static void CM_FindLeastAreaEnl(RSTREE R,
                                typinterval *newrect,
                                refnode node,
                                Rint skip,
                                Rint I[])

{
  void *ptrNi;
  Rint entLen, nofent, numbOfDim;
  Rint i, d;
  Rfloat area, newarea, enlarge, leastenlarge;
  typatomkey low, high;
  refinterval newReIntv, nodReIntv;
  boolean first= TRUE;
  
  ptrNi= node;
  ptrNi+= (*R).Pent0;
  entLen= (*R).DIR.entLen;
  nofent= (*node).s.nofentries;
  numbOfDim= (*R).numbofdim;
  for (i= 0; i < nofent; i++) {
    if (i != skip) {
      
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
      if (first) {
        leastenlarge= enlarge;
        I[0]= i;
        first= FALSE;
      }
      else {
        if (enlarge < leastenlarge) {
          leastenlarge= enlarge;
          I[0]= i;
        }
      }
    }
    ptrNi+= entLen;
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

