/* ----- RSTSupport.c ----- */
#//
#// Copyright (c) 1994 - 2015 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//


#include "RSTSupport.h"
#include "RSTUtil.h"
#include "RSTMemAlloc.h"
#include "RSTErrors.h"


/* constants */


/* types */


/* declarations */

static void CopyParamsToR(RSTREE R);
static void SetArrayLengths(RSTREE R);

/************************************************************************/

static void CopyParamsToR(RSTREE R) {

  /* copy important parameters to record R for faster evaluation: */
  (*R).DIReqvWords_Mp1= (*R).parameters._.DIR.EqvWords_Mp1;
  (*R).DATAeqvWords_Mp1= (*R).parameters._.DATA.EqvWords_Mp1;
}

/************************************************************************/

static void SetArrayLengths(RSTREE R) {
  
  Rint entryRange, splitOverAllRange, maxEntries;
  refparameters par= &(*R).parameters._;
  
  /* to avoid multiple lengths, simply get the greatest: */
  if ((*par).DIR.M > (*par).DATA.M) {
    maxEntries= (*par).DIR.M;
  }
  else {
    maxEntries= (*par).DATA.M;
  }
  entryRange= 2 * maxEntries;
  /* sufficient for the deletion-by-reinsertion version:
     entryRange= maxEntries + sizeof(typword); */
  splitOverAllRange= maxEntries * 2 * (*R).numbofdim;
  (*R).ValArrLen= entryRange * sizeof(Rfloat);
  (*R).IndArrLen= entryRange * sizeof(Rint);
  (*R).BytArrLen= (entryRange / sizeof(typword) + 1) * sizeof(typword);
  /* (treatment corresponding to ClearVector(), EqvWords_Mp1 respectively) */
  (*R).RectArrLen= entryRange * (*R).SIZErect;
  (*R).PointArrLen= entryRange * (*R).SIZEpoint;
  (*R).ValAllArrLen= splitOverAllRange * sizeof(Rfloat);
  (*R).IndAllArrLen= splitOverAllRange * sizeof(Rint);
}

/************************************************************************/

void SetSizesPreAllocArrs(RSTREE R) {

  CopyParamsToR(R);
  SetArrayLengths(R);
  
  /* initialize pointers to NULL, allowing to free them bundled if
     allocation of one of them fails: */
  /* --- in RSTInstDel, RSTChooseSub: */
  (*R).rootMBB= NULL;
  (*R).GIP_use0= NULL;
  (*R).I_instRect= NULL;
  (*R).CSCM_EnlRects= NULL;
  (*R).CSCM_Enls= NULL;
  (*R).S_NSibl= NULL;
  (*R).S_NSplit= NULL;
  (*R).S_allrect= NULL;
  (*R).SADDD_I= NULL;
  (*R).DDPS_edgearray= NULL;
  (*R).DDPS_rectarray= NULL;
  (*R).DDPS_leftrect= NULL;
  (*R).DDPS_rightrect= NULL;
  (*R).M_NMerge= NULL;
  (*R).M_use0= NULL;
  (*R).M_reducedrect= NULL;
  (*R).SM_NSibl= NULL;
  (*R).SM_allrect= NULL;
  (*R).EDDE_allcenter= NULL;
  (*R).EDDE_newrectcenter= NULL;
  (*R).EDDE_center= NULL;
  (*R).EDDE_distarr= NULL;
  (*R).EDDE_I= NULL;
  (*R).EDDE_chosen= NULL;
  (*R).DOR_effDelRect= NULL;
  (*R).APAMD_helprect= NULL;
  /* --- in RSTree: */
  (*R).IR_entry= NULL;
  /* --- in RSTQuery: */
  (*R).RQA_rect= NULL;
  (*R).RQA_info= NULL;
  /* --- in RSTUtil: */
  (*R).PRCD_cntr= NULL;
  
  (*R).rootMBB= allocM((*R).SIZErect);
  if ((*R).rootMBB == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  
  (*R).GIP_use0= allocM((*R).IndArrLen);
  if ((*R).GIP_use0 == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).I_instRect= allocM((*R).SIZErect);
  if ((*R).I_instRect == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).CSCM_EnlRects= allocM((*R).RectArrLen);
  if ((*R).CSCM_EnlRects == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).CSCM_Enls= allocM((*R).ValArrLen);
  if ((*R).CSCM_Enls == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).S_NSibl= allocM((*R).UNInodeLen);
  if ((*R).S_NSibl == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).S_NSplit= allocM((*R).UNInodeLen);
  if ((*R).S_NSplit == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).S_allrect= allocM((*R).SIZErect);
  if ((*R).S_allrect == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).SADDD_I= allocM((*R).IndArrLen);
  if ((*R).SADDD_I == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).DDPS_edgearray= allocM((*R).ValArrLen);
  if ((*R).DDPS_edgearray == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).DDPS_rectarray= allocM((*R).RectArrLen);
  if ((*R).DDPS_rectarray == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).DDPS_leftrect= allocM((*R).SIZErect);
  if ((*R).DDPS_leftrect == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).DDPS_rightrect= allocM((*R).SIZErect);
  if ((*R).DDPS_rightrect == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).M_NMerge= allocM((*R).UNInodeLen);
  if ((*R).M_NMerge == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).M_use0= allocM((*R).IndArrLen);
  if ((*R).M_use0 == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).M_reducedrect= allocM((*R).SIZErect);
  if ((*R).M_reducedrect == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).SM_NSibl= allocM((*R).UNInodeLen);
  if ((*R).SM_NSibl == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).SM_allrect= allocM((*R).SIZErect);
  if ((*R).SM_allrect == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).EDDE_allcenter= allocM((*R).SIZEpoint);
  if ((*R).EDDE_allcenter == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).EDDE_newrectcenter= allocM((*R).SIZEpoint);
  if ((*R).EDDE_newrectcenter == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).EDDE_center= allocM((*R).SIZEpoint);
  if ((*R).EDDE_center == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).EDDE_distarr= allocM((*R).ValArrLen);
  if ((*R).EDDE_distarr == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).EDDE_I= allocM((*R).IndArrLen);
  if ((*R).EDDE_I == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).EDDE_chosen= allocM((*R).BytArrLen);
  if ((*R).EDDE_chosen == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).DOR_effDelRect= allocM((*R).SIZErect);
  if ((*R).DOR_effDelRect == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).APAMD_helprect= allocM((*R).SIZErect);
  if ((*R).APAMD_helprect == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).IR_entry= allocM((*R).DATA.entLen);
  if ((*R).IR_entry == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).RQA_rect= allocM((*R).SIZErect);
  if ((*R).RQA_rect == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).RQA_info= allocM((*R).SIZEinfo);
  if ((*R).RQA_info == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
  (*R).PRCD_cntr= allocM((*R).SIZEpoint);
  if ((*R).PRCD_cntr == NULL) {
    setRSTerr(R,"SetSizesPreAllocArrs",reAlloc);
    DeallocArrs(R);
    (*R).RSTDone= FALSE;
    return;
  }
}

/************************************************************************/

void DeallocArrs(RSTREE R) {

  freeM(&(*R).rootMBB);
  
  freeM(&(*R).GIP_use0);
  freeM(&(*R).I_instRect);
  freeM(&(*R).CSCM_EnlRects);
  freeM(&(*R).CSCM_Enls);
  freeM(&(*R).S_NSibl);
  freeM(&(*R).S_NSplit);
  freeM(&(*R).S_allrect);
  freeM(&(*R).SADDD_I);
  freeM(&(*R).DDPS_edgearray);
  freeM(&(*R).DDPS_rectarray);
  freeM(&(*R).DDPS_leftrect);
  freeM(&(*R).DDPS_rightrect);
  freeM(&(*R).M_NMerge);
  freeM(&(*R).M_use0);
  freeM(&(*R).M_reducedrect);
  freeM(&(*R).SM_NSibl);
  freeM(&(*R).SM_allrect);
  freeM(&(*R).EDDE_allcenter);
  freeM(&(*R).EDDE_newrectcenter);
  freeM(&(*R).EDDE_center);
  freeM(&(*R).EDDE_distarr);
  freeM(&(*R).EDDE_I);
  freeM(&(*R).EDDE_chosen);
  freeM(&(*R).DOR_effDelRect);
  freeM(&(*R).APAMD_helprect);
  freeM(&(*R).IR_entry);
  freeM(&(*R).RQA_rect);
  freeM(&(*R).RQA_info);
  freeM(&(*R).PRCD_cntr);
}

/************************************************************************/
