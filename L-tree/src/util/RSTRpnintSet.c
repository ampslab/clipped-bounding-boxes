/* ----- RSTRpnintSet.c ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//

/***** Ordered set of Rpnint of arbitrary cardinality *****/
/* Binary tree implementation. */


#include <stdlib.h>

#include "RSTRpnintSet.h"
#include "RSTMemAlloc.h"
#include "RSTErrors.h"


/* ----- types ----- */

typedef struct RpnintBinTree {
                             Rpnint                data;
                             struct RpnintBinTree  *lson;
                             struct RpnintBinTree  *rson;
                             } pnSETrec;

typedef pnSETrec  *pnSET;


/* declarations */

static void PNS_AddLeft(pnSET set, pnSET branch);
static void PNS_OutputSet(FILE *stream,
                          pnSET set,
                          Rint *cardcount,
                          Rint epl);


/*********************************************************************/

t_pnS  PNS_Empty()

{
  return NULL;
}

/*********************************************************************/

void  PNS_Delete(t_pnS *s)

{
  pnSET set;
  
  if (*s != NULL) {
    set= (pnSET)*s;
    
    PNS_Delete((t_pnS*)&(*set).lson);
    PNS_Delete((t_pnS*)&(*set).rson);
    freeM(s);
  }
}

/*********************************************************************/

boolean  PNS_Include(t_pnS *s, Rpnint value, boolean *Done)

{
  pnSET set;
  
  if (*s == NULL) {
    *s= allocM(sizeof(pnSETrec));
    if (*s == NULL) {
      *Done= FALSE;
      pRSTerr("RpnintSet.PNS_Include",reAlloc);
      return FALSE;
    }
    set= (pnSET)*s;
    
    (*set).data= value;
    (*set).lson= NULL;
    (*set).rson= NULL;
    return TRUE;
  }
  else {
    set= (pnSET)*s;
    
    if (value < (*set).data) {
      return PNS_Include((t_pnS*)&(*set).lson,value,Done);
    }
    else if (value > (*set).data) {
      return PNS_Include((t_pnS*)&(*set).rson,value,Done);
    }
    else {
      return FALSE;
    }
  }
}

/*********************************************************************/

boolean  PNS_Exclude(t_pnS *s, Rpnint value)

{
  pnSET help, set;
  
  if (*s == NULL) {
    return FALSE;
  }
  else {
    set= (pnSET)*s;
    
    if (value < (*set).data) {
      return PNS_Exclude((t_pnS*)&(*set).lson,value);
    }
    else if (value > (*set).data) {
      return PNS_Exclude((t_pnS*)&(*set).rson,value);
    }
    else {
      if ((*set).rson == NULL) {
        help= (*set).lson;
        freeM(s);
        *s= (t_pnS)help;
      }
      else {
        PNS_AddLeft((*set).rson,(*set).lson);
        help= (*set).rson;
        freeM(s);
        *s= (t_pnS)help;
      }
      return TRUE;
    }
  }
}

/*********************************************************************/

static void PNS_AddLeft(pnSET set, pnSET branch)

{
  if ((*set).lson == NULL) {
    (*set).lson= branch;
  }
  else {
    PNS_AddLeft((*set).lson,branch);
  }
}

/*********************************************************************/

boolean PNS_Pop(t_pnS *s, Rpnint *value)

{
  pnSET help, set;
  
  if (*s == NULL) {
    return FALSE;
  }
  else {
    set= (pnSET)*s;
    
    *value= (*set).data;
    if ((*set).rson == NULL) {
      help= (*set).lson;
      freeM(s);
      *s= (t_pnS)help;
    }
    else {
      PNS_AddLeft((*set).rson,(*set).lson);
      help= (*set).rson;
      freeM(s);
      *s= (t_pnS)help;
    }
    return TRUE;
  }
}

/*********************************************************************/

boolean PNS_PopMin(t_pnS *s, Rpnint *value)

{
  pnSET help, set;
  
  if (*s == NULL) {
    return FALSE;
  }
  else {
    set= (pnSET)*s;
    
    if ((*set).lson == NULL) {
      *value= (*set).data;
      if ((*set).rson != NULL) {
        help= (*set).rson;
        freeM(s);
        *s= (t_pnS)help;
      }
      else {
        freeM(s);
      }
      return TRUE;
    }
    else {
      return PNS_PopMin((t_pnS*)&(*set).lson,value);
    }
  }
}

/*********************************************************************/

boolean PNS_PopMax(t_pnS *s, Rpnint *value)

{
  pnSET help, set;
  
  if (*s == NULL) {
    return FALSE;
  }
  else {
    set= (pnSET)*s;
    
    if ((*set).rson == NULL) {
      *value= (*set).data;
      if ((*set).lson != NULL) {
        help= (*set).lson;
        freeM(s);
        *s= (t_pnS)help;
      }
      else {
        freeM(s);
      }
      return TRUE;
    }
    else {
      return PNS_PopMax((t_pnS*)&(*set).rson,value);
    }
  }
}

/*********************************************************************/

boolean PNS_In(t_pnS s, Rpnint value)

{
  pnSET set;
  
  if (s == NULL) {
    return FALSE;
  }
  else {
    set= (pnSET)s;
    
    if (value < (*set).data) {
      return PNS_In((t_pnS)(*set).lson,value);
    }
    else if (value > (*set).data) {
      return PNS_In((t_pnS)(*set).rson,value);
    }
    else {
      return TRUE;
    }
  }
}

/*********************************************************************/

Rint PNS_Cardinality(t_pnS s)

{
  pnSET set;
  
  if (s == NULL) {
    return 0;
  }
  else {
    set= (pnSET)s;
    
    return 1+PNS_Cardinality((t_pnS)(*set).lson) + PNS_Cardinality((t_pnS)(*set).rson);
  }
}

/*********************************************************************/

void PNS_Print(FILE *stream, t_pnS s, Rint epl)

{
  pnSET set= (pnSET)s;
  Rint cardcount= 0;
  
  PNS_OutputSet(stream,set,&cardcount,epl);
  if (cardcount == 0 || cardcount % epl != 0) {
    fprintf(stream,"\n");
  }
}

/*********************************************************************/

static void PNS_OutputSet(FILE *stream,
                          pnSET set,
                          Rint *cardcount,
                          Rint epl)

{
  char str[80];
  
  if (set != NULL) {
    PNS_OutputSet(stream,(*set).lson,cardcount,epl);
    fprintf(stream,strans(" %N",str),(*set).data);
    (*cardcount)++;
    if (*cardcount % epl == 0) {
      fprintf(stream,"\n");
    }
    PNS_OutputSet(stream,(*set).rson,cardcount,epl);
  }
}

/*********************************************************************/

void PNS_PrintTree(FILE *stream, t_pnS s)

{
  pnSET set;
  char str[80];
  
  if (s == NULL) {
    fprintf(stream,".");
  }
  else {
    set= (pnSET)s;
    
    fprintf(stream,"[");
    PNS_PrintTree(stream,(t_pnS)(*set).lson);
    fprintf(stream,strans("%N",str),(*set).data);
    PNS_PrintTree(stream,(t_pnS)(*set).rson);
    fprintf(stream,"]");
  }
}

/*********************************************************************/
