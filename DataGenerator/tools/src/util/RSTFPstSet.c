/* ----- RSTFPstSet.c ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//

/***** Ordered set of FPst of arbitrary cardinality *****/
/* Binary tree implementation. */


#include <stdlib.h>

#include "RSTFPstSet.h"
#include "RSTMemAlloc.h"
#include "RSTErrors.h"


/* ----- types ----- */

typedef struct FPstBinTree {
                           FPst                data;
                           struct FPstBinTree  *lson;
                           struct FPstBinTree  *rson;
                           } FPSETrec;

typedef FPSETrec  *FPSET;


/* declarations */

static void FPS_AddLeft(FPSET set, FPSET branch);
static void FPS_OutputSet(FILE *stream,
                          FPSET set,
                          Rint *cardcount,
                          Rint epl);


/*********************************************************************/

t_FPS  FPS_Empty()

{
  return NULL;
}

/*********************************************************************/

void  FPS_Delete(t_FPS *s)

{
  FPSET set;
  
  if (*s != NULL) {
    set= (FPSET)*s;
    
    FPS_Delete((t_FPS*)&(*set).lson);
    FPS_Delete((t_FPS*)&(*set).rson);
    freeM(s);
  }
}

/*********************************************************************/

boolean  FPS_Include(t_FPS *s, FPst value, boolean *Done)

{
  FPSET set;
  
  if (*s == NULL) {
    *s= allocM(sizeof(FPSETrec));
    if (*s == NULL) {
      *Done= FALSE;
      pRSTerr("FPstSet.FPS_Include",reAlloc);
      return FALSE;
    }
    set= (FPSET)*s;
    
    (*set).data= value;
    (*set).lson= NULL;
    (*set).rson= NULL;
    return TRUE;
  }
  else {
    set= (FPSET)*s;
    
    if ((*((FPun *)&value)).i < (*((FPun *)&(*set).data)).i) {
      return FPS_Include((t_FPS*)&(*set).lson,value,Done);
    }
    else if ((*((FPun *)&value)).i > (*((FPun *)&(*set).data)).i) {
      return FPS_Include((t_FPS*)&(*set).rson,value,Done);
    }
    else {
      return FALSE;
    }
  }
}

/*********************************************************************/

boolean  FPS_Exclude(t_FPS *s, FPst value)

{
  FPSET help, set;
  
  if (*s == NULL) {
    return FALSE;
  }
  else {
    set= (FPSET)*s;
    
    if ((*((FPun *)&value)).i < (*((FPun *)&(*set).data)).i) {
      return FPS_Exclude((t_FPS*)&(*set).lson,value);
    }
    else if ((*((FPun *)&value)).i > (*((FPun *)&(*set).data)).i) {
      return FPS_Exclude((t_FPS*)&(*set).rson,value);
    }
    else {
      if ((*set).rson == NULL) {
        help= (*set).lson;
        freeM(s);
        *s= (t_FPS)help;
      }
      else {
        FPS_AddLeft((*set).rson,(*set).lson);
        help= (*set).rson;
        freeM(s);
        *s= (t_FPS)help;
      }
      return TRUE;
    }
  }
}

/*********************************************************************/

static void FPS_AddLeft(FPSET set, FPSET branch)

{
  if ((*set).lson == NULL) {
    (*set).lson= branch;
  }
  else {
    FPS_AddLeft((*set).lson,branch);
  }
}

/*********************************************************************/

boolean FPS_Pop(t_FPS *s, FPst *value)

{
  FPSET help, set;
  
  if (*s == NULL) {
    return FALSE;
  }
  else {
    set= (FPSET)*s;
    
    *value= (*set).data;
    if ((*set).rson == NULL) {
      help= (*set).lson;
      freeM(s);
      *s= (t_FPS)help;
    }
    else {
      FPS_AddLeft((*set).rson,(*set).lson);
      help= (*set).rson;
      freeM(s);
      *s= (t_FPS)help;
    }
    return TRUE;
  }
}

/*********************************************************************/

boolean FPS_PopMin(t_FPS *s, FPst *value)

{
  FPSET help, set;
  
  if (*s == NULL) {
    return FALSE;
  }
  else {
    set= (FPSET)*s;
    
    if ((*set).lson == NULL) {
      *value= (*set).data;
      if ((*set).rson != NULL) {
        help= (*set).rson;
        freeM(s);
        *s= (t_FPS)help;
      }
      else {
        freeM(s);
      }
      return TRUE;
    }
    else {
      return FPS_PopMin((t_FPS*)&(*set).lson,value);
    }
  }
}

/*********************************************************************/

boolean FPS_PopMax(t_FPS *s, FPst *value)

{
  FPSET help, set;
  
  if (*s == NULL) {
    return FALSE;
  }
  else {
    set= (FPSET)*s;
    
    if ((*set).rson == NULL) {
      *value= (*set).data;
      if ((*set).lson != NULL) {
        help= (*set).lson;
        freeM(s);
        *s= (t_FPS)help;
      }
      else {
        freeM(s);
      }
      return TRUE;
    }
    else {
      return FPS_PopMax((t_FPS*)&(*set).rson,value);
    }
  }
}

/*********************************************************************/

boolean FPS_In(t_FPS s, FPst value)

{
  FPSET set;
  
  if (s == NULL) {
    return FALSE;
  }
  else {
    set= (FPSET)s;
    
    if ((*((FPun *)&value)).i < (*((FPun *)&(*set).data)).i) {
      return FPS_In((t_FPS)(*set).lson,value);
    }
    else if ((*((FPun *)&value)).i > (*((FPun *)&(*set).data)).i) {
      return FPS_In((t_FPS)(*set).rson,value);
    }
    else {
      return TRUE;
    }
  }
}

/*********************************************************************/

Rint FPS_Cardinality(t_FPS s)

{
  FPSET set;
  
  if (s == NULL) {
    return 0;
  }
  else {
    set= (FPSET)s;
    
    return 1+FPS_Cardinality((t_FPS)(*set).lson) + FPS_Cardinality((t_FPS)(*set).rson);
  }
}

/*********************************************************************/

void FPS_Print(FILE *stream, t_FPS s, Rint epl)

{
  FPSET set= (FPSET)s;
  Rint cardcount= 0;
  
  FPS_OutputSet(stream,set,&cardcount,epl);
  if (cardcount == 0 || cardcount % epl != 0) {
    fprintf(stream,"\n");
  }
}

/*********************************************************************/

static void FPS_OutputSet(FILE *stream,
                          FPSET set,
                          Rint *cardcount,
                          Rint epl)

{
  char str[80];
  
  if (set != NULL) {
    FPS_OutputSet(stream,(*set).lson,cardcount,epl);
    fprintf(stream,strans(" f%I.p%N",str),(*set).data.f,(*set).data.p);
    (*cardcount)++;
    if (*cardcount % epl == 0) {
      fprintf(stream,"\n");
    }
    FPS_OutputSet(stream,(*set).rson,cardcount,epl);
  }
}

/*********************************************************************/

void FPS_PrintTree(FILE *stream, t_FPS s)

{
  FPSET set;
  char str[80];
  
  if (s == NULL) {
    fprintf(stream,"0");
  }
  else {
    set= (FPSET)s;
    
    fprintf(stream,"[");
    FPS_PrintTree(stream,(t_FPS)(*set).lson);
    fprintf(stream,strans("f%I.p%N",str),(*set).data.f,(*set).data.p);
    FPS_PrintTree(stream,(t_FPS)(*set).rson);
    fprintf(stream,"]");
  }
}

/*********************************************************************/
