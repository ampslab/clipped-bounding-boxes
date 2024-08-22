/* ----- RSTRpnintBag.c ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//

/*****          Ordered bag of Rpnint of arbitrary cardinality         *****/
/***** Open bag implementation, may hold negative numbers of instances *****/
/*****   Values are deleted when their number of instances becomes 0   *****/
/*****          Change to closed bag: see "closed bag" below           *****/
/* Binary tree implementation. */


#include <stdlib.h>

#include "RSTRpnintBag.h"
#include "RSTMemAlloc.h"
#include "RSTErrors.h"


/* ----- types ----- */

typedef struct RpnintBinBag {
                            Rpnint               data;
                            Rint                 numb;
                            struct RpnintBinBag  *lson;
                            struct RpnintBinBag  *rson;
                            } pnBAGrec;

typedef pnBAGrec  *pnBAG;


/* declarations */

static void PNB_AddLeft(pnBAG bag, pnBAG branch);
static void PNB_OutputBag(FILE *stream,
                          pnBAG bag,
                          Rint *cardcount,
                          Rint epl);


/*********************************************************************/

t_pnB  PNB_Empty()

{
  return NULL;
}

/*********************************************************************/

void  PNB_Delete(t_pnB *b)

{
  pnBAG bag;
  
  if (*b != NULL) {
    bag= (pnBAG)*b;
    
    PNB_Delete((t_pnB*)&(*bag).lson);
    PNB_Delete((t_pnB*)&(*bag).rson);
    freeM(b);
  }
}

/*********************************************************************/

Rint  PNB_Include(t_pnB *b, Rpnint value, Rint NumbOf, boolean *Done)

{
  pnBAG bag;
  
  if (*b == NULL) {
    if (NumbOf != 0) {
      *b= allocM(sizeof(pnBAGrec));
      if (*b == NULL) {
        *Done= FALSE;
        pRSTerr("RpnintBag.PNB_Include",reAlloc);
        return 0;
      }
      bag= (pnBAG)*b;
      
      (*bag).data= value;
      (*bag).numb= NumbOf;
      (*bag).lson= NULL;
      (*bag).rson= NULL;
    }
    return NumbOf;
  }
  else {
    bag= (pnBAG)*b;
    
    if (value < (*bag).data) {
      return PNB_Include((t_pnB*)&(*bag).lson,value,NumbOf,Done);
    }
    else if (value > (*bag).data) {
      return PNB_Include((t_pnB*)&(*bag).rson,value,NumbOf,Done);
    }
    else {
      (*bag).numb+= NumbOf;
      return (*bag).numb;
    }
  }
}

/*********************************************************************/

Rint  PNB_Exclude(t_pnB *b, Rpnint value, Rint NumbOf)

{
  pnBAG help, bag;
  Rint NewNumb;
  
  if (*b == NULL) {
    return -NumbOf;
  }
  else {
    bag= (pnBAG)*b;
    
    if (value < (*bag).data) {
      return PNB_Exclude((t_pnB*)&(*bag).lson,value,NumbOf);
    }
    else if (value > (*bag).data) {
      return PNB_Exclude((t_pnB*)&(*bag).rson,value,NumbOf);
    }
    else {
      NewNumb= (*bag).numb - NumbOf;
      if (NewNumb == 0) {               /* closed bag: if (NewNumb <= 0) { */
        if ((*bag).rson == NULL) {
          help= (*bag).lson;
          freeM(b);
          *b= (t_pnB)help;
        }
        else {
          PNB_AddLeft((*bag).rson,(*bag).lson);
          help= (*bag).rson;
          freeM(b);
          *b= (t_pnB)help;
        }
      }
      else {
        (*bag).numb= NewNumb;
      }
    }
    return NewNumb;
  }
}

/*********************************************************************/

static void PNB_AddLeft(pnBAG bag, pnBAG branch)

{
  if ((*bag).lson == NULL) {
    (*bag).lson= branch;
  }
  else {
    PNB_AddLeft((*bag).lson,branch);
  }
}

/*********************************************************************/

Rint  PNB_Pop(t_pnB *b, Rpnint *object)

{
  pnBAG help, bag;
  Rint Numb;
  
  if (*b == NULL) {
    return 0;
  }
  else {
    bag= (pnBAG)*b;
    
    *object= (*bag).data;
    Numb= (*bag).numb;
    if ((*bag).rson == NULL) {
      help= (*bag).lson;
      freeM(b);
      *b= (t_pnB)help;
    }
    else {
      PNB_AddLeft((*bag).rson,(*bag).lson);
      help= (*bag).rson;
      freeM(b);
      *b= (t_pnB)help;
    }
    return Numb;
  }
}

/*********************************************************************/

Rint  PNB_PopMin(t_pnB *b, Rpnint *object)

{
  pnBAG help, bag;
  Rint Numb;
  
  if (*b == NULL) {
    return 0;
  }
  else {
    bag= (pnBAG)*b;
    
    if ((*bag).lson == NULL) {
      *object= (*bag).data;
      Numb= (*bag).numb;
      if ((*bag).rson != NULL) {
        help= (*bag).rson;
        freeM(b);
        *b= (t_pnB)help;
      }
      else {
        freeM(b);
      }
      return Numb;
    }
    else {
      return PNB_PopMin((t_pnB*)&(*bag).lson,object);
    }
  }
}

/*********************************************************************/

Rint  PNB_PopMax(t_pnB *b, Rpnint *object)

{
  pnBAG help, bag;
  Rint Numb;
  
  if (*b == NULL) {
    return 0;
  }
  else {
    bag= (pnBAG)*b;
    
    if ((*bag).rson == NULL) {
      *object= (*bag).data;
      Numb= (*bag).numb;
      if ((*bag).lson != NULL) {
        help= (*bag).lson;
        freeM(b);
        *b= (t_pnB)help;
      }
      else {
        freeM(b);
      }
      return Numb;
    }
    else {
      return PNB_PopMax((t_pnB*)&(*bag).rson,object);
    }
  }
}

/*********************************************************************/

Rint  PNB_In(t_pnB b, Rpnint object)

{
  pnBAG bag;
  
  if (b == NULL) {
    return 0;
  }
  else {
    bag= (pnBAG)b;
    
    if (object < (*bag).data) {
      return PNB_In((t_pnB)(*bag).lson,object);
    }
    else if (object > (*bag).data) {
      return PNB_In((t_pnB)(*bag).rson,object);
    }
    else {
      return (*bag).numb;
    }
  }
}

/*********************************************************************/

Rint PNB_NumbElems(t_pnB b)

{
  pnBAG bag;
  
  if (b == NULL) {
    return 0;
  }
  else {
    bag= (pnBAG)b;
    
    return 1 + PNB_NumbElems((t_pnB)(*bag).lson) + PNB_NumbElems((t_pnB)(*bag).rson);
  }
}

/*********************************************************************/

Rint PNB_NumbInsts(t_pnB b)

{
  pnBAG bag;
  
  if (b == NULL) {
    return 0;
  }
  else {
    bag= (pnBAG)b;
    
    return (*bag).numb + PNB_NumbInsts((t_pnB)(*bag).lson) + PNB_NumbInsts((t_pnB)(*bag).rson);
  }
}

/*********************************************************************/

void PNB_Print(FILE *stream,
               t_pnB b,
               Rint epl)

{
  pnBAG bag= (pnBAG)b;
  Rint cardcount= 0;
  
  PNB_OutputBag(stream,bag,&cardcount,epl);
  if (cardcount == 0 || cardcount % epl != 0) {
    fprintf(stream,"\n");
  }
}

/*********************************************************************/

static void PNB_OutputBag(FILE *stream,
                          pnBAG bag,
                          Rint *cardcount,
                          Rint epl)

{
  char str[80];
  
  if (bag != NULL) {
    PNB_OutputBag(stream,(*bag).lson,cardcount,epl);
    fprintf(stream,strans(" %I*%N",str),(*bag).numb,(*bag).data);
    (*cardcount)++;
    if (*cardcount % epl == 0) {
      fprintf(stream,"\n");
    }
    PNB_OutputBag(stream,(*bag).rson,cardcount,epl);
  }
}

/*********************************************************************/
