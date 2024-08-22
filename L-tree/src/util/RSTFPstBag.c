/* ----- RSTFPstBag.c ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//

/*****          Ordered bag of FPst of arbitrary cardinality           *****/
/***** Open bag implementation, may hold negative numbers of instances *****/
/*****   Values are deleted when their number of instances becomes 0   *****/
/*****          Change to closed bag: see "closed bag" below           *****/
/* Binary tree implementation. */


#include <stdlib.h>

#include "RSTFPstBag.h"
#include "RSTMemAlloc.h"
#include "RSTErrors.h"


/* ----- types ----- */

typedef struct FPstBinBag {
                          FPst                data;
                          Rint                numb;
                          struct FPstBinBag   *lson;
                          struct FPstBinBag   *rson;
                          } FPBAGrec;

typedef FPBAGrec  *FPBAG;


/* declarations */

static void FPB_AddLeft(FPBAG bag, FPBAG branch);
static void FPB_OutputBag(FILE *stream,
                          FPBAG bag,
                          Rint *cardcount,
                          Rint epl);


/*********************************************************************/

t_FPB  FPB_Empty()

{
  return NULL;
}

/*********************************************************************/

void  FPB_Delete(t_FPB *b)

{
  FPBAG bag;
  
  if (*b != NULL) {
    bag= (FPBAG)*b;
    
    FPB_Delete((t_FPB*)&(*bag).lson);

    FPB_Delete((t_FPB*)&(*bag).rson);
    freeM(b);
  }
}

/*********************************************************************/

Rint  FPB_Include(t_FPB *b, FPst value, Rint NumbOf, boolean *Done)

{
  FPBAG bag;
  
  if (*b == NULL) {
    if (NumbOf != 0) {
      *b= allocM(sizeof(FPBAGrec));
      if (*b == NULL) {
        *Done= FALSE;
        pRSTerr("FPstBag.FPB_Include",reAlloc);
        return 0;
      }
      bag= (FPBAG)*b;
      
      (*bag).data= value;
      (*bag).numb= NumbOf;
      (*bag).lson= NULL;
      (*bag).rson= NULL;
    }
    return NumbOf;
  }
  else {
    bag= (FPBAG)*b;
    
    if ((*((FPun *)&value)).i < (*((FPun *)&(*bag).data)).i) {
      return FPB_Include((t_FPB*)&(*bag).lson,value,NumbOf,Done);
    }
    else if ((*((FPun *)&value)).i > (*((FPun *)&(*bag).data)).i) {
      return FPB_Include((t_FPB*)&(*bag).rson,value,NumbOf,Done);
    }
    else {
      (*bag).numb+= NumbOf;
      return (*bag).numb;
    }
  }
}

/*********************************************************************/

Rint  FPB_Exclude(t_FPB *b, FPst value, Rint NumbOf)

{
  FPBAG help, bag;
  Rint NewNumb;
  
  if (*b == NULL) {
    return -NumbOf;
  }
  else {
    bag= (FPBAG)*b;
    
    if ((*((FPun *)&value)).i < (*((FPun *)&(*bag).data)).i) {
      return FPB_Exclude((t_FPB*)&(*bag).lson,value,NumbOf);
    }
    else if ((*((FPun *)&value)).i > (*((FPun *)&(*bag).data)).i) {
      return FPB_Exclude((t_FPB*)&(*bag).rson,value,NumbOf);
    }
    else {
      NewNumb= (*bag).numb - NumbOf;
      if (NewNumb == 0) {               /* closed bag: if (NewNumb <= 0) { */
        if ((*bag).rson == NULL) {
          help= (*bag).lson;
          freeM(b);
          *b= (t_FPB)help;
        }
        else {
          FPB_AddLeft((*bag).rson,(*bag).lson);
          help= (*bag).rson;
          freeM(b);
          *b= (t_FPB)help;
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

static void FPB_AddLeft(FPBAG bag, FPBAG branch)

{
  if ((*bag).lson == NULL) {
    (*bag).lson= branch;
  }
  else {
    FPB_AddLeft((*bag).lson,branch);
  }
}

/*********************************************************************/

Rint  FPB_Pop(t_FPB *b, FPst *object)

{
  FPBAG help, bag;
  Rint Numb;
  
  if (*b == NULL) {
    return 0;
  }
  else {
    bag= (FPBAG)*b;
    
    *object= (*bag).data;
    Numb= (*bag).numb;
    if ((*bag).rson == NULL) {
      help= (*bag).lson;
      freeM(b);
      *b= (t_FPB)help;
    }
    else {
      FPB_AddLeft((*bag).rson,(*bag).lson);
      help= (*bag).rson;
      freeM(b);
      *b= (t_FPB)help;
    }
    return Numb;
  }
}

/*********************************************************************/

Rint  FPB_PopMin(t_FPB *b, FPst *object)

{
  FPBAG help, bag;
  Rint Numb;
  
  if (*b == NULL) {
    return 0;
  }
  else {
    bag= (FPBAG)*b;
    
    if ((*bag).lson == NULL) {
      *object= (*bag).data;
      Numb= (*bag).numb;
      if ((*bag).rson != NULL) {
        help= (*bag).rson;
        freeM(b);
        *b= (t_FPB)help;
      }
      else {
        freeM(b);
      }
      return Numb;
    }
    else {
      return FPB_PopMin((t_FPB*)&(*bag).lson,object);
    }
  }
}

/*********************************************************************/

Rint  FPB_PopMax(t_FPB *b, FPst *object)

{
  FPBAG help, bag;
  Rint Numb;
  
  if (*b == NULL) {
    return 0;
  }
  else {
    bag= (FPBAG)*b;
    
    if ((*bag).rson == NULL) {
      *object= (*bag).data;
      Numb= (*bag).numb;
      if ((*bag).lson != NULL) {
        help= (*bag).lson;
        freeM(b);
        *b= (t_FPB)help;
      }
      else {
        freeM(b);
      }
      return Numb;
    }
    else {
      return FPB_PopMax((t_FPB*)&(*bag).rson,object);
    }
  }
}

/*********************************************************************/

Rint  FPB_In(t_FPB b, FPst object)

{
  FPBAG bag;
  
  if (b == NULL) {
    return 0;
  }
  else {
    bag= (FPBAG)b;
    
    if ((*((FPun *)&object)).i < (*((FPun *)&(*bag).data)).i) {
      return FPB_In((t_FPB)(*bag).lson,object);
    }
    else if ((*((FPun *)&object)).i > (*((FPun *)&(*bag).data)).i) {
      return FPB_In((t_FPB)(*bag).rson,object);
    }
    else {
      return (*bag).numb;
    }
  }
}

/*********************************************************************/

Rint FPB_NumbElems(t_FPB b)

{
  FPBAG bag;
  
  if (b == NULL) {
    return 0;
  }
  else {
    bag= (FPBAG)b;
    
    return 1 + FPB_NumbElems((t_FPB)(*bag).lson) + FPB_NumbElems((t_FPB)(*bag).rson);
  }
}

/*********************************************************************/

Rint FPB_NumbInsts(t_FPB b)

{
  FPBAG bag;
  
  if (b == NULL) {
    return 0;
  }
  else {
    bag= (FPBAG)b;
    
    return (*bag).numb + FPB_NumbInsts((t_FPB)(*bag).lson) + FPB_NumbInsts((t_FPB)(*bag).rson);
  }
}

/*********************************************************************/

void FPB_Print(FILE *stream, t_FPB b, Rint epl)

{
  FPBAG bag= (FPBAG)b;
  Rint cardcount= 0;
  
  FPB_OutputBag(stream,bag,&cardcount,epl);
  if (cardcount == 0 || cardcount % epl != 0) {
    fprintf(stream,"\n");
  }
}

/*********************************************************************/

static void FPB_OutputBag(FILE *stream,
                          FPBAG bag,
                          Rint *cardcount,
                          Rint epl)

{
  char str[80];
  
  if (bag != NULL) {
    FPB_OutputBag(stream,(*bag).lson,cardcount,epl);
    fprintf(stream,strans(" %I*f%I.p%N",str),(*bag).numb,(*bag).data.f,(*bag).data.p);
    (*cardcount)++;
    if (*cardcount % epl == 0) {
      fprintf(stream,"\n");
    }
    FPB_OutputBag(stream,(*bag).rson,cardcount,epl);
  }
}

/*********************************************************************/
