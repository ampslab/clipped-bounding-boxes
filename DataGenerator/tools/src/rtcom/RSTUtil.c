/* ----- RSTUtil.c ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//


#include "RSTUtil.h"
#include "RSTQuery.h"
#include "RSTMemAlloc.h"


static void ExchangeRfloats(Rfloat *x, Rfloat *y);
static Rfloat RfloatsMedian(Rfloat v[],
                            Rint *b,
                            Rint m,
                            Rint *e);
static Rfloat IofRfloatsMedian(Rfloat v[],
                               Rint I[],
                               Rint *b,
                               Rint m,
                               Rint *e);
static Rint IofEntsMedian(void *entArr,
                          Rint entrylen,
                          Rint I[],
                          Rint dim,
                          Side side,
                          Rint *b,
                          Rint m,
                          Rint *e);
static void ExchangeRints(Rint *x, Rint *y);
static Rint RintsMedian(Rint v[],
                        Rint *b,
                        Rint m,
                        Rint *e);
static void ExchangeRpnints(Rpnint *x, Rpnint *y);
static Rpnint RpnintsMedian(Rpnint v[],
                            Rlint *b,
                            Rlint m,
                            Rlint *e);

/***********************************************************************/

void ClearVector(typword *ptr, Rint wordsqty)

{
  typword *max= ptr + wordsqty;
  while (ptr < max) {
    *ptr= 0;
    ptr++;
  }
}

/***********************************************************************/

void CopyRect(Rint numbOfDim,
              const typinterval *from,
              typinterval *to)

{
  typinterval *max= to + numbOfDim;
  while (to < max) {
    *to= *from;
    to++; from++;
  }
}

/***********************************************************************/

void CopyPoint(Rint numbOfDim,
               typcoord *from,
               typcoord *to)

{
  typcoord *max= to + numbOfDim;
  while (to < max) {
    *to= *from;
    to++; from++;
  }
}

/***********************************************************************/

void EvalCenter(Rint numbOfDim, refinterval intv, Rfloat *center)

{
  Rint d;
  
  for (d= 0; d < numbOfDim; d++) {
    *center= ((*intv).l + (*intv).h) * 0.5;
    intv++; center++;
  }
}

/***********************************************************************/

Rfloat PPDistance(Rint numbOfDim, Rfloat *point1, Rfloat *point2)

{
  Rfloat distval;
  Rint d;
  
  distval= 0.0;
  for (d= 0; d < numbOfDim; d++) {
    distval+= (*point1 - *point2) * (*point1 - *point2);
    point1++; point2++;
  }
  return distval;
}

/***********************************************************************/

Rfloat PPDist(Rint numbOfDim,
              DistCalcFunc DistFunc,
              Rfloat *point1,
              Rfloat *point2)

{
  Rfloat distval;
  Rint d;
  
  distval= 0.0;
  for (d= 0; d < numbOfDim; d++) {
    DistFunc(*point1 - *point2,&distval);
    point1++; point2++;
  }
  return distval;
}

/***********************************************************************/

Rfloat PRectDist(RSTREE R,
                 DistCalcFunc DistFunc,
                 Rfloat *point,
                 refinterval intv)

{
  Rfloat low, high; /* not a typatomkey here! */
  Rfloat distval, coord;
  Rint d;
  
  Rint numbOfDim= (*R).numbofdim;
  distval= 0.0;
  for (d= 0; d < numbOfDim; d++) {
    coord= *point;
    low= (*intv).l;
    high= (*intv).h;
    if (high < coord) {
      DistFunc(coord - high,&distval);
    }
    else if (low > coord) {
      DistFunc(low - coord,&distval);
    }
    point++; intv++;
  }
  return distval;
}

/***********************************************************************/

Rfloat PRectMaxDist(RSTREE R,
                    DistCalcFunc DistFunc,
                    Rfloat *point,
                    refinterval intv)

{
  Rfloat distval, p_l, h_p;
  Rint d;
  
  Rint numbOfDim= (*R).numbofdim;
  distval= 0.0;
  for (d= 0; d < numbOfDim; d++) {
    h_p= (*intv).h - *point;
    p_l= *point - (*intv).l;
    if (h_p > p_l) {
      DistFunc(h_p,&distval);
    }
    else {
      DistFunc(p_l,&distval);
    }
    point++; intv++;
  }
  return distval;
}

/***********************************************************************/

Rfloat PRectCntrDist(RSTREE R,
                     DistCalcFunc DistFunc,
                     typcoord *point,
                     typinterval *rect)

{
  Rfloat ppdist;
  typcoord *cntr= allocM((*R).SIZEpoint);
  
  EvalCenter((*R).numbofdim,rect,cntr);
  ppdist= PPDist((*R).numbofdim,DistFunc,point,cntr);
  
  freeM(&cntr);
  return ppdist;
}

/***********************************************************************/

void EvalDirNodesMBB(RSTREE R, refnode node, typinterval *mbb)

{
  void *ptrNi;
  ptrNi= node;
  ptrNi+= (*R).Pent0;
  Rint entLen= (*R).DIR.entLen;
  Rint nofent=  (*node).s.nofentries;
  Rint numbOfDim= (*R).numbofdim;
  refinterval intv, intvMbb;
  Rint i, d;
  
  CopyRect(numbOfDim,ptrNi,mbb);
  
  ptrNi+= entLen;
  
  for (i= 1; i < nofent; i++) {
    
    intv= ptrNi;
    intvMbb= mbb;
    
    for (d= 0; d < numbOfDim; d++) {
      if ((*intvMbb).l > (*intv).l) {
        (*intvMbb).l= (*intv).l;
      }
      if ((*intvMbb).h < (*intv).h) {
        (*intvMbb).h= (*intv).h;
      }
      intvMbb++; intv++;
    }
    ptrNi+= entLen;
  }
}

/***********************************************************************/

void EvalDataNodesMBB(RSTREE R, refnode node, typinterval *mbb)

{
  void *ptrNi;
  ptrNi= node;
  ptrNi+= (*R).Pent0;
  Rint entLen= (*R).DATA.entLen;
  Rint nofent=  (*node).s.nofentries;
  Rint numbOfDim= (*R).numbofdim;
  refinterval intv, intvMbb;
  Rint i, d;
  
  CopyRect(numbOfDim,ptrNi,mbb);
  
  ptrNi+= entLen;
  
  for (i= 1; i < nofent; i++) {
    
    intv= ptrNi;
    intvMbb= mbb;
    
    for (d= 0; d < numbOfDim; d++) {
      if ((*intvMbb).l > (*intv).l) {
        (*intvMbb).l= (*intv).l;
      }
      if ((*intvMbb).h < (*intv).h) {
        (*intvMbb).h= (*intv).h;
      }
      intvMbb++; intv++;
    }
    ptrNi+= entLen;
  }
}

/***********************************************************************/
/* GetOverlap provides valid overlap areas (and is called)
   only if Overlaps() yielded TRUE! */

void GetOverlap(Rint numbOfDim,
                refinterval intv1,
                refinterval intv2,
                Rfloat *area)

{
  typatomkey low, high;
  Rint d;
  
  *area= 1.0;
  for (d= 0; d < numbOfDim; d++) {
    if ((*intv1).l < (*intv2).l) {
      low= (*intv2).l;
    }
    else {
      low= (*intv1).l;
    }
    if ((*intv1).h < (*intv2).h) {
      high= (*intv1).h;
    }
    else {
      high= (*intv2).h;
    }
    *area= *area * (high - low);
    /* conversion typatomkey --> Rfloat */
    intv1++; intv2++;
  }
}

/***********************************************************************/

Rfloat OverlapEnl(RSTREE R,
                  typinterval *oldRect,
                  typinterval *enlRect,
                  typinterval *otherRect)

{
  Rfloat overlap, enlarge;
#ifndef COUNTS_OFF
  refcount c= &(*R).count;
  if ((*c).on) {
    (*c).CS_OvlpEnlComput++;
  }
#endif
  enlarge= 0.0;
  if (Overlaps((*R).numbofdim,enlRect,otherRect)) {
    GetOverlap((*R).numbofdim,enlRect,otherRect,&overlap);
    enlarge+= overlap;
    if (Overlaps((*R).numbofdim,oldRect,otherRect)) {
      GetOverlap((*R).numbofdim,oldRect,otherRect,&overlap);
      enlarge-= overlap;
    }
  }
  return enlarge;
}

/***********************************************************************/
/* GetOvlpEdge provides valid overlap edges (and is called)
   only if Overlaps() yielded TRUE! */

void GetOvlpEdge(Rint numbOfDim,
                 refinterval intv1,
                 refinterval intv2,
                 Rfloat *edge)

{
  typatomkey low, high;
  Rint d;
  
  *edge= 0.0;
  for (d= 0; d < numbOfDim; d++) {
    if ((*intv1).l < (*intv2).l) {
      low= (*intv2).l;
    }
    else {
      low= (*intv1).l;
    }
    if ((*intv1).h < (*intv2).h) {
      high= (*intv1).h;
    }
    else {
      high= (*intv2).h;
    }
    *edge= *edge + (high - low);
    /* conversion typatomkey --> Rfloat */
    intv1++; intv2++;
  }
  /* For punktiform overlap regions Overlaps(r1,r2) returns TRUE, and there */
  /* IS overlap, probably creating cost! */
  /* Provide some *edge != 0.0 for punktiform overlap regions: */
  if (*edge == 0.0) {
    *edge= MIN_RFLOAT;
  }
}

/***********************************************************************/

Rfloat OvlpEdgeEnl(RSTREE R,
                   typinterval *oldRect,
                   typinterval *enlRect,
                   typinterval *otherRect)

{
  Rfloat ovlpEdge, enlarge;
#ifndef COUNTS_OFF
  refcount c= &(*R).count;
  if ((*c).on) {
    (*c).CS_OvlpEnlComput++;
  }
#endif
  enlarge= 0.0;
  if (Overlaps((*R).numbofdim,enlRect,otherRect)) {
    GetOvlpEdge((*R).numbofdim,enlRect,otherRect,&ovlpEdge);
    enlarge+= ovlpEdge;
    if (Overlaps((*R).numbofdim,oldRect,otherRect)) {
      GetOvlpEdge((*R).numbofdim,oldRect,otherRect,&ovlpEdge);
      enlarge-= ovlpEdge;
    }
  }
  return enlarge;
}

/***********************************************************************/

void GetOvlpRect(Rint numbOfDim,
                 refinterval intv1,
                 refinterval intv2,
                 refinterval intvInter)

{
  Rint d;
  
  for (d= 0; d < numbOfDim; d++) {
    if ((*intv1).l < (*intv2).l) {
      (*intvInter).l= (*intv2).l;
    }
    else {
      (*intvInter).l= (*intv1).l;
    }
    if ((*intv1).h < (*intv2).h) {
      (*intvInter).h= (*intv1).h;
    }
    else {
      (*intvInter).h= (*intv2).h;
    }
    intv1++; intv2++; intvInter++;
  }
}

/***********************************************************************/

boolean RectsEql(Rint numbOfDim,
                 const typinterval *intv1,
                 const typinterval *intv2)

{
  Rint d;
  
  d= 0;
  do {
    if ((*intv1).l != (*intv2).l || (*intv1).h != (*intv2).h) {
      return FALSE;
    }
    d++; intv1++; intv2++;
  } while (d < numbOfDim);
  return TRUE;
}

/***********************************************************************/

boolean  Covers(Rint numbOfDim,
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

boolean Overlaps(Rint numbOfDim,
                 refinterval intv1,
                 refinterval intv2)

{
  Rint d;
  
  d= 0;
  do {
    if ((*intv1).l > (*intv2).h || (*intv1).h < (*intv2).l) {
      return FALSE;
    }
    d++; intv1++; intv2++;
  } while (d < numbOfDim);
  return TRUE;
}

/***********************************************************************/

boolean DirOvlps(RSTREE R,
                 const typinterval *intv1,
                 const typinterval *intv2)

{
  Rint d;
#ifndef COUNTS_OFF
  refcount c= &(*R).count;
  if ((*c).on) {
    (*c).DIR.comp++;
  }
#endif
  Rint numbOfDim= (*R).numbofdim;
  d= 0;
  do {
    if ((*intv1).l > (*intv2).h || (*intv1).h < (*intv2).l) {
      return FALSE;
    }
    d++; intv1++; intv2++;
  } while (d < numbOfDim);
  return TRUE;
}

/***********************************************************************/

boolean DataOvlps(RSTREE R,
                  const typinterval *intv1,
                  const typinterval *intv2)

{
  Rint d;
#ifndef COUNTS_OFF
  refcount c= &(*R).count;
  if ((*c).on) {
    (*c).DATA.comp++;
  }
#endif
  Rint numbOfDim= (*R).numbofdim;
  d= 0;
  do {
    if ((*intv1).l > (*intv2).h || (*intv1).h < (*intv2).l) {
      return FALSE;
    }
    d++; intv1++; intv2++;
  } while (d < numbOfDim);
  return TRUE;
}

/***********************************************************************/

void QSortRfloats(Rint begin,
                  Rint end,
                  Rfloat value[])
/* Sorts value. */

{
  Rfloat midelem;
  Rint i, j;
  
  i= begin; j= end;
  /* midelem= value[(i+j) >> 1]; */
  /* profitable for #elements > 7: */
  midelem= RfloatsMedian(value,&i,(i+j) >> 1,&j);
  if (i < j) {
    do {
      while (value[i] < midelem) {
        i++;
      }
      while (value[j] > midelem) {
        j--;
      }
      if (i < j) {
        ExchangeRfloats(&value[i],&value[j]);
        i++; j--;
      }
      else if (i == j) {
        i++; j--;
      }
    } while (i <= j);
    if (begin < j) {
      if (j - begin > 1) {
        QSortRfloats(begin,j,value);
      }
      else {
        if (value[begin] > value[j]) {
          ExchangeRfloats(&value[begin],&value[j]);
        }
      }
    }
    if (i < end) {
      if (end - i > 1) {
        QSortRfloats(i,end,value);
      }
      else {
        if (value[i] > value[end]) {
          ExchangeRfloats(&value[i],&value[end]);
        }
      }
    }
  }
}

/***********************************************************************/

static void ExchangeRfloats(Rfloat *x, Rfloat *y)

{
  Rfloat z;
  
  z= *x; *x= *y; *y= z;
}

/***********************************************************************/

static Rfloat RfloatsMedian(Rfloat v[],
                            Rint *b,
                            Rint m,
                            Rint *e)

{
  if (v[*b] <= v[m]) {
    if (v[m] <= v[*e]) {                 //bme (sorted)
    }
    else {
      if (v[*b] <= v[*e]) {              //bem
        ExchangeRfloats(&v[m],&v[*e]);
      }
      else {                             //ebm  if (b == m) { x(b,e) } else ...
        ExchangeRfloats(&v[m],&v[*e]);   //     optimizes #exchanges
        ExchangeRfloats(&v[*b],&v[m]);
      }
    }
  }
  else {
    if (v[*b] <= v[*e]) {                //mbe
      ExchangeRfloats(&v[*b],&v[m]);
    }
    else {
      if (v[m] < v[*e]) {                //meb (NOTE the "<"!!)
        ExchangeRfloats(&v[*b],&v[m]);
        ExchangeRfloats(&v[m],&v[*e]);
      }
      else {                             //emb
        ExchangeRfloats(&v[*b],&v[*e]);
      }
    }
  }
  (*b)++; (*e)--;
  return v[m];
}

/***********************************************************************/

void QSortIofRfloats(Rint begin,
                     Rint end,
                     Rfloat value[],
                     Rint I[])
/* Sorts I
   by value[I[i]]. */

{
  Rfloat midelem;
  Rint i, j;
  
  i= begin; j= end;
  /* midelem= value[I[(i+j) >> 1]]; */
  /* profitable for #elements > 7: */
  midelem= IofRfloatsMedian(value,I,&i,(i+j) >> 1,&j);
  if (i < j) {
    do {
      while (value[I[i]] < midelem) {
        i++;
      }
      while (value[I[j]] > midelem) {
        j--;
      }
      if (i < j) {
        ExchangeRints(&I[i],&I[j]);
        i++; j--;
      }
      else if (i == j) {
        i++; j--;
      }
    } while (i <= j);
    if (begin < j) {
      if (j - begin > 1) {
        QSortIofRfloats(begin,j,value,I);
      }
      else {
        if (value[I[begin]] > value[I[j]]) {
          ExchangeRints(&I[begin],&I[j]);
        }
      }
    }
    if (i < end) {
      if (end - i > 1) {
        QSortIofRfloats(i,end,value,I);
      }
      else {
        if (value[I[i]] > value[I[end]]) {
          ExchangeRints(&I[i],&I[end]);
        }
      }
    }
  }
}

/***********************************************************************/

static Rfloat IofRfloatsMedian(Rfloat v[],
                               Rint I[],
                               Rint *b,
                               Rint m,
                               Rint *e)

{
  if (v[I[*b]] <= v[I[m]]) {
    if (v[I[m]] <= v[I[*e]]) {           //bme (sorted)
    }
    else {
      if (v[I[*b]] <= v[I[*e]]) {        //bem
        ExchangeRints(&I[m],&I[*e]);
      }
      else {                             //ebm  if (b == m) { x(b,e) } else ...
        ExchangeRints(&I[m],&I[*e]);     //     optimizes #exchanges
        ExchangeRints(&I[*b],&I[m]);
      }
    }
  }
  else {
    if (v[I[*b]] <= v[I[*e]]) {          //mbe
      ExchangeRints(&I[*b],&I[m]);
    }
    else {
      if (v[I[m]] < v[I[*e]]) {          //meb (NOTE the "<"!!)
        ExchangeRints(&I[*b],&I[m]);
        ExchangeRints(&I[m],&I[*e]);
      }
      else {                             //emb
        ExchangeRints(&I[*b],&I[*e]);
      }
    }
  }
  (*b)++; (*e)--;
  return v[I[m]];
}

/***********************************************************************/

void QSortIofEnts(Rint begin,
                  Rint end,
                  Rint dim,
                  Side side,
                  void *entArr,
                  Rint entrylen,
                  Rint I[])
/* Sorts I
   primarily   by Ntosort[I[i]].rect[dim].side,
   secondarily by Ntosort[I[i]].rect[dim]."otherside". */

{
  void *ptr;
  refinterval re;
  typatomkey midlow, midhigh;
  refatomkey lcoord, hcoord;
  refinterval int1, int2;
  Rint i, j, median, mean;
  
  i= begin; j= end;
  if (side == low) {
    
    mean= (i+j) >> 1;
    /* re = &Ntosort[I[mean]].rect[dim]; */
    median= IofEntsMedian(entArr,entrylen,I,dim,side,&i,mean,&j);
    ptr= entArr + median * entrylen + dim * SIZEinterval;
    re = ptr;
    
    if (i < j) {
      midlow= (*re).l;
      midhigh= (*re).h;
      do {
        ptr= entArr + I[i] * entrylen + dim * SIZEinterval;
        lcoord= ptr;
        hcoord= lcoord + 1;
        while (*lcoord < midlow || (*lcoord == midlow && *hcoord < midhigh)) {
          i++;
          ptr= entArr + I[i] * entrylen + dim * SIZEinterval;
          lcoord= ptr;
          hcoord= lcoord + 1;
        }
        ptr= entArr + I[j] * entrylen + dim * SIZEinterval;
        lcoord= ptr;
        hcoord= lcoord + 1;
        while (*lcoord > midlow || (*lcoord == midlow && *hcoord > midhigh ) ) {
          j--;
          ptr= entArr + I[j] * entrylen + dim * SIZEinterval;
          lcoord= ptr;
          hcoord= lcoord + 1;
        }
        if (i < j) {
          ExchangeRints(&I[i],&I[j]);
          i++; j--;
        }
        else if (i == j) {
          i++; j--;
        }
      } while (i <= j);
      if (begin < j) {
        if (j - begin > 1) {
          QSortIofEnts(begin,j,dim,low,entArr,entrylen,I);
        }
        else {
          ptr= entArr + I[begin] * entrylen + dim * SIZEinterval;
          int1= ptr;
          ptr= entArr + I[j] * entrylen + dim * SIZEinterval;
          int2= ptr;
          if ((*int1).l > (*int2).l || ((*int1).l == (*int2).l && (*int1).h > (*int2).h)) {
            ExchangeRints(&I[begin],&I[j]);
          }
        }
      }
      if (i < end) {
        if (end - i > 1) {
          QSortIofEnts(i,end,dim,low,entArr,entrylen,I);
        }
        else {
          ptr= entArr + I[i] * entrylen + dim * SIZEinterval;
          int1= ptr;
          ptr= entArr + I[end] * entrylen + dim * SIZEinterval;
          int2= ptr;
          if ((*int1).l > (*int2).l || ((*int1).l == (*int2).l && (*int1).h > (*int2).h)) {
            ExchangeRints(&I[i],&I[end]);
          }
        }
      }
    }
  }
  else {
  
    mean= (i+j) >> 1;
    /* re = &Ntosort[I[mean]].rect[dim]; */
    median= IofEntsMedian(entArr,entrylen,I,dim,side,&i,mean,&j);
    ptr= entArr + median * entrylen + dim * SIZEinterval;
    re = ptr;
    
    if (i < j) {
      midhigh= (*re).h;
      midlow= (*re).l;
      do {
        ptr= entArr + I[i] * entrylen + dim * SIZEinterval;
        lcoord= ptr;
        hcoord= lcoord + 1;
        while (*hcoord < midhigh || (*hcoord == midhigh && *lcoord < midlow)) {
          i++;
          ptr= entArr + I[i] * entrylen + dim * SIZEinterval;
          lcoord= ptr;
          hcoord= lcoord + 1;
        }
        ptr= entArr + I[j] * entrylen + dim * SIZEinterval;
        lcoord= ptr;
        hcoord= lcoord + 1;
        while (*hcoord > midhigh || (*hcoord == midhigh && *lcoord > midlow)) {
          j--;
          ptr= entArr + I[j] * entrylen + dim * SIZEinterval;
          lcoord= ptr;
          hcoord= lcoord + 1;
        }
        if (i < j) {
          ExchangeRints(&I[i],&I[j]);
          i++; j--;
        }
        else if (i == j) {
          i++; j--;
        }
      } while (i <= j);
      if (begin < j) {
        if (j - begin > 1) {
          QSortIofEnts(begin,j,dim,high,entArr,entrylen,I);
        }
        else {
          ptr= entArr + I[begin] * entrylen + dim * SIZEinterval;
          int1= ptr;
          ptr= entArr + I[j] * entrylen + dim * SIZEinterval;
          int2= ptr;
          if ((*int1).h > (*int2).h || ((*int1).h == (*int2).h && (*int1).l > (*int2).l)) {
            ExchangeRints(&I[begin],&I[j]);
          }
        }
      }
      if (i < end) {
        if (end - i > 1) {
          QSortIofEnts(i,end,dim,high,entArr,entrylen,I);
        }
        else {
          ptr= entArr + I[i] * entrylen + dim * SIZEinterval;
          int1= ptr;
          ptr= entArr + I[end] * entrylen + dim * SIZEinterval;
          int2= ptr;
          if ((*int1).h > (*int2).h || ((*int1).h == (*int2).h && (*int1).l > (*int2).l)) {
            ExchangeRints(&I[i],&I[end]);
          }
        }     
      }
    }
  }
}

/***********************************************************************/

static Rint IofEntsMedian(void *entArr,
                          Rint entrylen,
                          Rint I[],
                          Rint dim,
                          Side side,
                          Rint *b,
                          Rint m,
                          Rint *e)

/* returns the index (in v) of the pivot element */

{
  void *ptr;
  refinterval ib, im, ie;

  ptr= entArr + I[*b] * entrylen + dim * SIZEinterval;
  ib= ptr;
  ptr= entArr + I[m] * entrylen + dim * SIZEinterval;
  im= ptr;
  ptr= entArr + I[*e] * entrylen + dim * SIZEinterval;
  ie= ptr;
  if (side == low) {
    if ((*ib).l < (*im).l || ((*ib).l == (*im).l && (*ib).h <= (*im).h)) {
      if ((*im).l < (*ie).l || ((*im).l == (*ie).l && (*im).h <= (*ie).h)) {
        //bme (sorted)
      }
      else {
        if ((*ib).l < (*ie).l || ((*ib).l == (*ie).l && (*ib).h <= (*ie).h)) {
          //bem
          ExchangeRints(&I[m],&I[*e]);
        }
        else {
          //ebm  if (b == m) { x(b,e) } else ...
          //     optimizes #exchanges
          ExchangeRints(&I[m],&I[*e]);
          ExchangeRints(&I[*b],&I[m]);
        }
      }
    }
    else {
      if ((*ib).l < (*ie).l || ((*ib).l == (*ie).l && (*ib).h <= (*ie).h)) {
        //mbe
        ExchangeRints(&I[*b],&I[m]);
      }
      else {
        if ((*im).l < (*ie).l || ((*im).l == (*ie).l && (*im).h < (*ie).h)) {
          //meb (NOTE the "<"!!)
          ExchangeRints(&I[*b],&I[m]);
          ExchangeRints(&I[m],&I[*e]);
        }
        else {
          //emb
          ExchangeRints(&I[*b],&I[*e]);
        }
      }
    }
  }
  else {
    if ((*ib).h < (*im).h || ((*ib).h == (*im).h && (*ib).l <= (*im).l)) {
      if ((*im).h < (*ie).h || ((*im).h == (*ie).h && (*im).l <= (*ie).l)) {
        //bme (sorted)
      }
      else {
        if ((*ib).h < (*ie).h || ((*ib).h == (*ie).h && (*ib).l <= (*ie).l)) {
          //bem
          ExchangeRints(&I[m],&I[*e]);
        }
        else {
          //ebm  if (b == m) { x(b,e) } else ...
          //     optimizes #exchanges
          ExchangeRints(&I[m],&I[*e]);
          ExchangeRints(&I[*b],&I[m]);
        }
      }
    }
    else {
      if ((*ib).h < (*ie).h || ((*ib).h == (*ie).h && (*ib).l <= (*ie).l)) {
        //mbe
        ExchangeRints(&I[*b],&I[m]);
      }
      else {
        if ((*im).h < (*ie).h || ((*im).h == (*ie).h && (*im).l < (*ie).l)) {
          //meb (NOTE the "<"!!)
          ExchangeRints(&I[*b],&I[m]);
          ExchangeRints(&I[m],&I[*e]);
        }
        else {
          //emb
          ExchangeRints(&I[*b],&I[*e]);
        }
      }
    }
  }
  (*b)++; (*e)--;
  return I[m]; // the index in v
}

/***********************************************************************/

static void ExchangeRints(Rint *x, Rint *y)

{
  Rint z;
  
  z= *x; *x= *y; *y= z;
}

/***********************************************************************/

void QSortRints(Rint begin,
                Rint end,
                Rint value[])
/* Sorts value. */

{
  Rint midelem;
  Rint i, j;
  
  i= begin; j= end;
  /* midelem= value[(i+j) >> 1]; */
  /* profitable for #elements > 7: */
  midelem= RintsMedian(value,&i,(i+j) >> 1,&j);
  if (i < j) {
    do {
      while (value[i] < midelem) {
        i++;
      }
      while (value[j] > midelem) {
        j--;
      }
      if (i < j) {
        ExchangeRints(&value[i],&value[j]);
        i++; j--;
      }
      else if (i == j) {
        i++; j--;
      }
    } while (i <= j);
    if (begin < j) {
      if (j - begin > 1) {
        QSortRints(begin,j,value);
      }
      else {
        if (value[begin] > value[j]) {
          ExchangeRints(&value[begin],&value[j]);
        }
      }
    }
    if (i < end) {
      if (end - i > 1) {
        QSortRints(i,end,value);
      }
      else {
        if (value[i] > value[end]) {
          ExchangeRints(&value[i],&value[end]);
        }
      }
    }
  }
}

/***********************************************************************/

static Rint RintsMedian(Rint v[],
                        Rint *b,
                        Rint m,
                        Rint *e)

{
  if (v[*b] <= v[m]) {
    if (v[m] <= v[*e]) {                 //bme (sorted)
    }
    else {
      if (v[*b] <= v[*e]) {              //bem
        ExchangeRints(&v[m],&v[*e]);
      }
      else {                             //ebm  if (b == m) { x(b,e) } else ...
        ExchangeRints(&v[m],&v[*e]);     //     optimizes #exchanges
        ExchangeRints(&v[*b],&v[m]);
      }
    }
  }
  else {
    if (v[*b] <= v[*e]) {                //mbe
      ExchangeRints(&v[*b],&v[m]);
    }
    else {
      if (v[m] < v[*e]) {                //meb (NOTE the "<"!!)
        ExchangeRints(&v[*b],&v[m]);
        ExchangeRints(&v[m],&v[*e]);
      }
      else {                             //emb
        ExchangeRints(&v[*b],&v[*e]);
      }
    }
  }
  (*b)++; (*e)--;
  return v[m];
}

/***********************************************************************/

static void ExchangeRpnints(Rpnint *x, Rpnint *y)

{
  Rpnint z;
  
  z= *x; *x= *y; *y= z;
}

/***********************************************************************/
/* QuickSort may decrement j down to -1, while type Rpnint may be unsigned.
   ==> Set all index variables to type Rlint. Analogously applied to 
   RpnintsMedian(). */

void QSortRpnints(Rlint begin,
                  Rlint end,
                  Rpnint value[])
/* Sorts value. */

{
  Rpnint midelem;
  Rlint i, j;
  
  i= begin; j= end;
  /* midelem= value[(i+j) >> 1]; */
  /* profitable for #elements > 7: */
  midelem= RpnintsMedian(value,&i,(i+j) >> 1,&j);
  if (i < j) {
    do {
      while (value[i] < midelem) {
        i++;
      }
      while (value[j] > midelem) {
        j--;
      }
      if (i < j) {
        ExchangeRpnints(&value[i],&value[j]);
        i++; j--;
      }
      else if (i == j) {
        i++; j--;
      }
    } while (i <= j);
    if (begin < j) {
      if (j - begin > 1) {
        QSortRpnints(begin,j,value);
      }
      else {
        if (value[begin] > value[j]) {
          ExchangeRpnints(&value[begin],&value[j]);
        }
      }
    }
    if (i < end) {
      if (end - i > 1) {
        QSortRpnints(i,end,value);
      }
      else {
        if (value[i] > value[end]) {
          ExchangeRpnints(&value[i],&value[end]);
        }
      }
    }
  }
}

/***********************************************************************/
/* Note comment top of QSortRpnints()! */

static Rpnint RpnintsMedian(Rpnint v[],
                            Rlint *b,
                            Rlint m,
                            Rlint *e)

{
  if (v[*b] <= v[m]) {
    if (v[m] <= v[*e]) {                 //bme (sorted)
    }
    else {
      if (v[*b] <= v[*e]) {              //bem
        ExchangeRpnints(&v[m],&v[*e]);
      }
      else {                             //ebm  if (b == m) { x(b,e) } else ...
        ExchangeRpnints(&v[m],&v[*e]);   //     optimizes #exchanges
        ExchangeRpnints(&v[*b],&v[m]);
      }
    }
  }
  else {
    if (v[*b] <= v[*e]) {                //mbe
      ExchangeRpnints(&v[*b],&v[m]);
    }
    else {
      if (v[m] < v[*e]) {                //meb (NOTE the "<"!!)
        ExchangeRpnints(&v[*b],&v[m]);
        ExchangeRpnints(&v[m],&v[*e]);
      }
      else {                             //emb
        ExchangeRpnints(&v[*b],&v[*e]);
      }
    }
  }
  (*b)++; (*e)--;
  return v[m];
}

/***********************************************************************/

void InstallDirPath(RSTREE R,
                    Rint level,
                    typinterval *rect,
                    Rpnint pagenr)

{
  boolean found;
  
  found= FALSE;
  XstsPageNrInLevel(R,(*R).parameters._.rootlvl,level,rect,pagenr,&found);
  /* XstsPageNrInLevel valid only for level > 0 */
  if (! found) {
    fprintf(stderr,"%s\n"," RST: FATAL INTERNAL ERROR");
    fprintf(stderr,"%s\n","InstallDirPath 1");
    (*R).RSTDone= FALSE;
  }
}

/***********************************************************************/

Rfloat LeastRfloat(Rint begin, Rint end, Rfloat val[])

{
  Rint i;
  Rfloat leastval;
  
  leastval= val[begin];
  for (i= begin+1; i <= end; i++) {
    if (val[i] < leastval) {
      leastval= val[i];
    }
  }
  return leastval;
}

/***********************************************************************/

Rfloat MaxRfloat(Rint begin, Rint end, Rfloat val[])

{
  Rint i;
  Rfloat maxval;
  
  maxval= val[begin];
  for (i= begin+1; i <= end; i++) {
    if (val[i] > maxval) {
      maxval= val[i];
    }
  }
  return maxval;
}

/***********************************************************************/

Rint IofLeastRfloat(Rint begin, Rint end, Rfloat val[], Rint I[])

{
  Rint i, index;
  Rfloat leastval;
  
  leastval= val[I[begin]];
  index= I[begin];
  for (i= begin+1; i <= end; i++) {
    if (val[I[i]] < leastval) {
      leastval= val[I[i]];
      index= I[i];
    }
  }
  return index;
}

/***********************************************************************/

Rint IofMaxRfloat(Rint begin, Rint end, Rfloat val[], Rint I[])

{
  Rint i, index;
  Rfloat maxval;
  
  maxval= val[I[begin]];
  index= I[begin];
  for (i= begin+1; i <= end; i++) {
    if (val[I[i]] > maxval) {
      maxval= val[I[i]];
      index= I[i];
    }
  }
  return index;
}

/***********************************************************************/

Rint IofIofLeastRfloat(Rint begin, Rint end, Rfloat val[], Rint I[])

{
  Rint i, index;
  Rfloat leastval;
  
  leastval= val[I[begin]];
  index= begin;
  for (i= begin+1; i <= end; i++) {
    if (val[I[i]] < leastval) {
      leastval= val[I[i]];
      index= i;
    }
  }
  return index;
}

/***********************************************************************/

Rint IofIofMaxRfloat(Rint begin, Rint end, Rfloat val[], Rint I[])

{
  Rint i, index;
  Rfloat maxval;
  
  maxval= val[I[begin]];
  index= begin;
  for (i= begin+1; i <= end; i++) {
    if (val[I[i]] > maxval) {
      maxval= val[I[i]];
      index= i;
    }
  }
  return index;
}

/***********************************************************************/

boolean IsArea0(Rint numbOfDim, refinterval intv)

{
  Rint d;
  
  for (d= 0; d < numbOfDim; d++) {
    if ((*intv).l == (*intv).h) {
      return TRUE;
    }
    intv++;
  }
  return FALSE;
}

/***********************************************************************/

void setRSTerr(RSTREE R,
               char *preMessage,
               typerrors error)

{
  if (R != NULL) {
    (*R).error= error;
  }
  pRSTerr(preMessage,error);
}

/************************************************************************/

void setRSTwarn(RSTREE R,
                char *preMessage,
                typerrors error)

{
  if (R != NULL) {
    (*R).error= error;
  }
  pRSTwarn(preMessage,error);
}

/************************************************************************/

void AlignInt(Rint *numb, Rint alignm) {

  Rint ill= *numb & (alignm - 1);
  if (ill != 0) {
    (*numb)+= alignm - ill;
  }
}

/************************************************************************/

