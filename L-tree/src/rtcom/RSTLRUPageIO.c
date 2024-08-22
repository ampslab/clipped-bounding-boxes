/* ----- RSTLRUPageIO.c ----- */
#//
#// Copyright (c) 1994 - 2015 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//


#include "RSTLRUPageIO.h"
#include "RSTLRUBuf.h"
#include "RSTErrors.h"


/* NOTE: the write flag is NEVER set to FALSE. */


/* declarations */


/************************************************************************/
/* called by UnlinkPage, PutNode */

void UnlinkLRUPage(RSTREE R,
                   typstorage *str,
                   boolean toBeWritten,
                   Rpnint pageNr)

{
  boolean avail;
  refLRUentry refEntry;
  FPst filePage;
  
  filePage.f= (*str).f;
  filePage.p= pageNr;
  avail= HashLRUEntry((*R).LRU,filePage,&refEntry,&(*R).RSTDone);
  
  /* Begin internal error checks: */
  if (! avail) {
    fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
    fprintf(stderr,"%s\n","UnlinkLRUPage 1: missing availability.");
    if (refEntry == NULL) {
      fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
      fprintf(stderr,"%s\n","UnlinkLRUPage 2: missing entry.");
    }
    (*R).RSTDone= FALSE;
    return;
  }
  if ((*refEntry).locked < 1) {
    fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
    fprintf(stderr,"%s\n","UnlinkLRUPage 3: missing lock.");
    (*R).RSTDone= FALSE;
  }
  /* End   internal error checks */
  
  /* LRU entry handling: */
  /* (*refEntry).adr unconcerned */
  (*refEntry).write= (*refEntry).write || toBeWritten;
  (*refEntry).locked--;
}

/************************************************************************/
/* called by PutExtNode */

void WriteLRUPage(RSTREE R,
                  typstorage *str,
                  Rpnint pageNr,
                  refnode nodeptr)

{
  boolean avail;
  refLRUentry refEntry;
  FPst filePage;
  
  filePage.f= (*str).f;
  filePage.p= pageNr;
  avail= HashLRUEntry((*R).LRU,filePage,&refEntry,&(*R).RSTDone);
  
  /* Begin internal error checks: */
  if (! avail) {
    fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
    fprintf(stderr,"%s\n","WriteLRUPage 1: missing availability.");
    if (refEntry == NULL) {
      fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
      fprintf(stderr,"%s\n","WriteLRUPage 2: missing entry.");
    }
    (*R).RSTDone= FALSE;
    return;
  }
  if ((*refEntry).locked < 1) {
    fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
    fprintf(stderr,"%s\n","WriteLRUPage 3: missing lock.");
    (*R).RSTDone= FALSE;
  }
  /* End   internal error checks */
  
  /* LRU entry handling: */
  memcpy((*refEntry).adr,nodeptr,(*str).psize);
  (*refEntry).write= TRUE;
  (*refEntry).locked--;
}

/************************************************************************/
/* called by PutGetNode */

void WriteLinkLRUPage(RSTREE R,
                      typstorage *str,
                      Rpnint pageNr,
                      refnode *nodeptr)

{
  boolean avail;
  refLRUentry refEntry;
  FPst filePage;
  
  filePage.f= (*str).f;
  filePage.p= pageNr;
  avail= HashLRUEntry((*R).LRU,filePage,&refEntry,&(*R).RSTDone);
  
  /* Begin internal error checks: */
  if (! avail) {
    fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
    fprintf(stderr,"%s\n","WriteLinkLRUPage 1: missing availability.");
    if (refEntry == NULL) {
      fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
      fprintf(stderr,"%s\n","WriteLinkLRUPage 2: missing entry.");
    }
    (*R).RSTDone= FALSE;
    return;
  }
  if ((*refEntry).locked < 1) {
    fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
    fprintf(stderr,"%s\n","WriteLinkLRUPage 3: missing lock.");
    (*R).RSTDone= FALSE;
  }
  /* End   internal error checks */
  
  /* LRU entry handling: */
  if (*nodeptr != (*refEntry).adr) {
    memcpy((*refEntry).adr,*nodeptr,(*str).psize);
    *nodeptr= (*refEntry).adr;
  }
  (*refEntry).write= TRUE;
  /* (*refEntry).locked unmodified */
}

/************************************************************************/
/* called by GetNode */

void LinkLRUPage(RSTREE R,
                 typstorage *str,
                 Rpnint pageNr,
                 refnode *nodeptr)

{
  char s[80];
  boolean avail;
  refLRUentry refEntry;
  Rpnint oldqty, newqty;
  FPst filePage;
  boolean LRUDone= TRUE;
  
  filePage.f= (*str).f;
  filePage.p= pageNr;
  avail= HashLRUEntry((*R).LRU,filePage,&refEntry,&LRUDone);
  if (refEntry != NULL) {
  
    /* LRU entry handling: */
    *nodeptr= (*refEntry).adr;
    /* (*refEntry).write unmodified */
    (*refEntry).locked++;
  }
  if (! LRUDone) {
    /* handle possible displacement_Locks error (no lock-free entry): */
    if (LRUError((*R).LRU) == displacement_Locks) {
      GetLRUCapUse((*R).LRU,&oldqty,&newqty);
      ModifLRUCap((*R).LRU,newqty,&(*R).RSTDone);
      if ((*R).RSTDone) {
        fprintf(stderr,strans("WARNING: LRU buffer extension: %I --> %I\n",s),oldqty,newqty);
      }
    }
    else {
      (*R).RSTDone= FALSE;
    }
  }
}

/************************************************************************/

void EstablishLRUPage(RSTREE R, typstorage *str, Rpnint pageNr)

{
  char s[80];
  refLRUentry refEntry;
  Rpnint oldqty, newqty;
  FPst filePage;
  boolean LRUDone= TRUE;
  
  filePage.f= (*str).f;
  filePage.p= pageNr;
  if (! NewLRUEntry((*R).LRU,filePage,&refEntry,&LRUDone)) {
    fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
    fprintf(stderr,"%s\n","EstablishLRUpage 1: page not new.");
    (*R).RSTDone= FALSE;
  }
  if (refEntry != NULL) {
  
    /* LRU entry handling: */
    /* (*refEntry).adr unconcerned */
    /* (*refEntry).write unmodified */
    (*refEntry).locked++;
  }
  if (! LRUDone) {
    /* handle possible displacement_Locks error (no lock-free entry): */
    if (LRUError((*R).LRU) == displacement_Locks) {
      GetLRUCapUse((*R).LRU,&oldqty,&newqty);
      ModifLRUCap((*R).LRU,newqty,&(*R).RSTDone);
      if ((*R).RSTDone) {
        fprintf(stderr,strans("WARNING: LRU buffer extension: %I --> %I\n",s),oldqty,newqty);
      }
    }
    else {
      (*R).RSTDone= FALSE;
    }
  }
}

/************************************************************************/

void DeleteLRUPage(RSTREE R, typstorage *str, Rpnint pageNr)

{
  FPst filePage;
  
  filePage.f= (*str).f;
  filePage.p= pageNr;
  if (! DisposeLRUEntry((*R).LRU,filePage,&(*R).RSTDone)) {
    fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
    fprintf(stderr,"%s\n","DeleteLRUpage 1: page not available.");
    (*R).RSTDone= FALSE;
  }
}

/************************************************************************/
