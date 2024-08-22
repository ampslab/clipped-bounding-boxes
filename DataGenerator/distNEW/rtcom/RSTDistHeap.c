/* ----- RSTDistHeap.c ----- */
#//
#// Copyright (c) 1994 - 2012 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//

/*** heap of DistHeapElem ***/
/* array implementation */


#include "RSTDistHeap.h"
#include "RSTMemAlloc.h"


/* declarations */

static void minDH_Inst(DistHeap *h, refDHelem elem, Rint i);
static void maxDH_Inst(DistHeap *h, refDHelem elem, Rint i);
static void minDH_DelMin(DistHeap *h, refDHelem elem, Rint i);
static void maxDH_DelMax(DistHeap *h, refDHelem elem, Rint i);
static boolean DH_Extension(DistHeap *h);

/************************************************************************/

boolean DH_New(DistHeap *h,
               Rint elemLen,
               Rint max,
               Rfloat extFac,
               boolean verbose)

{
  Rint i;
  
  (*h).elemLen= elemLen;
  (*h).max= max;
  (*h).extFac= extFac;
  (*h).verbose= verbose;
  (*h).qty= 0;
  (*h).arr= allocM((*h).max * (*h).elemLen);
  if ((*h).arr == NULL) {
    return FALSE;
  }
  (*h).I= allocM((*h).max * sizeof(DistHeapIndElem));
  if ((*h).I == NULL) {
    freeM(&(*h).arr);
    return FALSE;
  }
  for (i= 0; i < (*h).max; i++) {
    (*h).I[i]= i;
  }
  return TRUE;
}

/************************************************************************/

void DH_Dispose(DistHeap *h)

{
  freeM(&(*h).arr);
  freeM(&(*h).I);
  (*h).max= 0;
}

/************************************************************************/

boolean minDH_Insert(DistHeap *h, refDHelem elem)

{
  void *ptrElemIqty;
  
  (*h).qty++;
  if ((*h).qty == (*h).max) { /* heap starts at arr[1] */
    if (! DH_Extension(h)) {
      (*h).qty--;
      return FALSE;
    }
  }
  ptrElemIqty= (*h).arr;
  ptrElemIqty+= (*h).I[(*h).qty] * (*h).elemLen;
  memcpy(ptrElemIqty,elem,(*h).elemLen);
  (*h).I[0]= (*h).I[(*h).qty];
  minDH_Inst(h,elem,(*h).qty);
  return TRUE;
}

/************************************************************************/

static void minDH_Inst(DistHeap *h, refDHelem elem, Rint i)

{
  Rint up= i>>1;
  void *ptrElemIup= (*h).arr;
  
  ptrElemIup+= (*h).I[up] * (*h).elemLen;
  if (i == 1 || (*elem).s.dist >= (*(refDHelem)ptrElemIup).s.dist) {
    (*h).I[i]= (*h).I[0];
  }
  else {
    (*h).I[i]= (*h).I[up];
    minDH_Inst(h,elem,up);
  }
}

/************************************************************************/

boolean maxDH_Insert(DistHeap *h, refDHelem elem)

{
  void *ptrElemIqty;
  
  (*h).qty++;
  if ((*h).qty == (*h).max) { /* heap starts at arr[1] */
    if (! DH_Extension(h)) {
      (*h).qty--;
      return FALSE;
    }
  }
  ptrElemIqty= (*h).arr;
  ptrElemIqty+= (*h).I[(*h).qty] * (*h).elemLen;
  memcpy(ptrElemIqty,elem,(*h).elemLen);
  (*h).I[0]= (*h).I[(*h).qty];
  maxDH_Inst(h,elem,(*h).qty);
  return TRUE;
}

/************************************************************************/

static void maxDH_Inst(DistHeap *h, refDHelem elem, Rint i)

{
  Rint up= i>>1;
  void *ptrElemIup= (*h).arr;
  
  ptrElemIup+= (*h).I[up] * (*h).elemLen;
  if (i == 1 || (*elem).s.dist <= (*(refDHelem)ptrElemIup).s.dist) {
    (*h).I[i]= (*h).I[0];
  }
  else {
    (*h).I[i]= (*h).I[up];
    maxDH_Inst(h,elem,up);
  }
}

/************************************************************************/

static boolean DH_Extension(DistHeap *h)

{
  char s[80];
  DistHeapArray newArr;
  DistHeapIndArr newIndArr;
  Rint oldmax, i;
  
  oldmax= (*h).max;
  (*h).max*= (*h).extFac;
  if ((*h).verbose) {
    fprintf(stderr,"PRIORITY QUEUE EXTENSION: ");
    fprintf(stderr,strans("%I --> %I records\n",s),(*h).qty-1,(*h).max-1);
  }
  newArr= (DistHeapArray)reallocM((*h).arr,(*h).max * (*h).elemLen);
  if (newArr == NULL) {
    fprintf(stderr,"PRIORITY QUEUE EXTENSION: FAILED\n");
    (*h).max= oldmax;
    return FALSE;
  }
  newIndArr= (DistHeapIndArr)reallocM((*h).I,(*h).max * sizeof(DistHeapIndElem));
  if (newIndArr == NULL) {
    fprintf(stderr,"PRIORITY QUEUE EXTENSION: FAILED\n");
    (*h).max= oldmax;
    return FALSE;
  }
  else {
    if ((*h).verbose) {
      fprintf(stderr,"Done\n");
    }
    (*h).arr= newArr;
    (*h).I= newIndArr;
    for (i= oldmax; i < (*h).max; i++) {
      (*h).I[i]= i;
    }
    return TRUE;
  }
}

/************************************************************************/

boolean minDH_Pop(DistHeap *h, refDHelem elem)

{
  Rint gap;
  void *ptrElemGap, *ptrElemIqty;
  
  if ((*h).qty == 0) {
    return FALSE;
  }
  else {
    gap= (*h).I[1];
    ptrElemGap= (*h).arr;
    ptrElemGap+= gap * (*h).elemLen;
    memcpy(elem,ptrElemGap,(*h).elemLen);
    ptrElemIqty= (*h).arr;
    ptrElemIqty+= (*h).I[(*h).qty] * (*h).elemLen;
    minDH_DelMin(h,ptrElemIqty,1);
    (*h).I[(*h).qty]= gap;
    (*h).qty--;
    return TRUE;
  }
}

/************************************************************************/

static void minDH_DelMin(DistHeap *h, refDHelem elem, Rint i)

{
  Rint down= i<<1;
  void *ptrElemA= (*h).arr;
  void *ptrElemB= (*h).arr;
  
  if (down >= (*h).qty) { /* no ||, pointers cannot be evaluated in advance! */
    (*h).I[i]= (*h).I[(*h).qty]; /* same as below! */
  }
  else {
    ptrElemA+= (*h).I[down] * (*h).elemLen;
    ptrElemB+= (*h).I[down+1] * (*h).elemLen;
    if ((*elem).s.dist <= (*(refDHelem)ptrElemA).s.dist && (*elem).s.dist <= (*(refDHelem)ptrElemB).s.dist) {
      (*h).I[i]= (*h).I[(*h).qty]; /* same as above! */
    }
    else {
      if ((*(refDHelem)ptrElemB).s.dist < (*(refDHelem)ptrElemA).s.dist) {
        down++;
      }
      (*h).I[i]= (*h).I[down];
      minDH_DelMin(h,elem,down);
    }
  }
}

/************************************************************************/

boolean maxDH_Pop(DistHeap *h, refDHelem elem)

{
  Rint gap;
  void *ptrElemGap, *ptrElemIqty;
  
  if ((*h).qty == 0) {
    return FALSE;
  }
  else {
    gap= (*h).I[1];
    ptrElemGap= (*h).arr;
    ptrElemGap+= gap * (*h).elemLen;
    memcpy(elem,ptrElemGap,(*h).elemLen);
    ptrElemIqty= (*h).arr;
    ptrElemIqty+= (*h).I[(*h).qty] * (*h).elemLen;
    maxDH_DelMax(h,ptrElemIqty,1);
    (*h).I[(*h).qty]= gap;
    (*h).qty--;
    return TRUE;
  }
}

/************************************************************************/

static void maxDH_DelMax(DistHeap *h, refDHelem elem, Rint i)

{
  Rint down= i<<1;
  void *ptrElemA= (*h).arr;
  void *ptrElemB= (*h).arr;
  
  if (down >= (*h).qty) { /* no ||, pointers cannot be evaluated in advance! */
    (*h).I[i]= (*h).I[(*h).qty]; /* same as below! */
  }
  else {
    ptrElemA+= (*h).I[down] * (*h).elemLen;
    ptrElemB+= (*h).I[down+1] * (*h).elemLen;
    if ((*elem).s.dist >= (*(refDHelem)ptrElemA).s.dist && (*elem).s.dist >= (*(refDHelem)ptrElemB).s.dist) {
      (*h).I[i]= (*h).I[(*h).qty]; /* same as above! */
    }
    else {
      if ((*(refDHelem)ptrElemB).s.dist > (*(refDHelem)ptrElemA).s.dist) {
        down++;
      }
      (*h).I[i]= (*h).I[down];
      maxDH_DelMax(h,elem,down);
    }
  }
}

/************************************************************************/

Rfloat DH_TopDist(DistHeap *h)

{
  void *ptr= (*h).arr;
  
  ptr+= (*h).I[1] * (*h).elemLen;
  return (*(refDHelem)ptr).s.dist;
}

/************************************************************************/

Rint DH_Qty(DistHeap *h)

{
  return (*h).qty;
}

/************************************************************************/

Rint DH_MaxQty(DistHeap *h)

{
  return (*h).max - 1;
}

/************************************************************************/
