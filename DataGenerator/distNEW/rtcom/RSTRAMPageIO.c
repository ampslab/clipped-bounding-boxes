/* ----- RSTRAMPageIO.c ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//


#include "RSTRAMPageIO.h"


/* declarations */


/************************************************************************/
/* called by -, administrative by GetPageNr, FreePageNr */

void ReadRAMPage(typstorage *str,
                 Rpnint pagenr,
                 void *block)

{
  void *adr;
  
  adr= (void *)((Rpint)(*str).RAM.ptr + pagenr * (*str).psize);
  memcpy(block,adr,(*str).psize);
}

/************************************************************************/
/* called by GetNode */

void LinkRAMPage(typstorage *str,
                 Rpnint pagenr,
                 refnode *nodeptr)

{
  void *adr;
  
  adr= (void *)((Rpint)(*str).RAM.ptr + pagenr * (*str).psize);
  *nodeptr= adr;
}

/************************************************************************/
/* called by PutExtNode, administrative by PutPageNr */

void WriteRAMPage(typstorage *str,
                  Rpnint pagenr,
                  void *block)

{
  void *adr;
  
  adr= (void *)((Rpint)(*str).RAM.ptr + pagenr * (*str).psize);
  memcpy(adr,block,(*str).psize);
}

/************************************************************************/
/* called by PutGetNode */

void WriteLinkRAMPage(typstorage *str,
                      Rpnint pagenr,
                      refnode *nodeptr)

{
  void *adr;
  
  adr= (void *)((Rpint)(*str).RAM.ptr + pagenr * (*str).psize);
  if (*nodeptr != adr) {
    memcpy(adr,*nodeptr,(*str).psize);
    *nodeptr= adr;
  }
}

/************************************************************************/
