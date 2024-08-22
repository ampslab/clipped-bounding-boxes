/* ----- RSTFilePageIO.c ----- */
#//
#// Copyright (c) 1994 - 2013 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//


#include "RSTFilePageIO.h"
#include "RSTFileAccess.h"


/* declarations */


/************************************************************************/
/* NOTE: (*R).DIR.str, (*R).DIR.PA.str i.e. administrative counts separate */

void ReadPage(RSTREE R,
              typstorage *str,
              Rpnint pagenr,
              void *block)

{
  if (RdPage((*str).f,(*str).psize,pagenr,block)) {
#ifndef COUNTS_OFF
    if ((*R).count.on) {
      (*str).cnt.read++;
    }
#endif
  }
  else {
    (*R).RSTDone= FALSE;
  }
}

/************************************************************************/
/* NOTE: (*R).DIR.str, (*R).DIR.PA.str i.e. administrative counts separate */

void WritePage(RSTREE R,
               typstorage *str,
               Rpnint pagenr,
               void *block)

{
  if (WrPage((*str).f,(*str).psize,pagenr,block)) {
#ifndef COUNTS_OFF
    if ((*R).count.on) {
      (*str).cnt.write++;
    }
#endif
  }
  else {
    (*R).RSTDone= FALSE;
  }
}

/************************************************************************/
