/* ----- RSTPageInOut.c ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//


#include "RSTPageInOut.h"
#include "RSTQuery.h"
#include "RSTUtil.h"
#include "RSTLRUPageIO.h"
#include "RSTRAMPageIO.h"
#include "RSTFilePageIO.h"
#include "RSTFileAccess.h"
#include "RSTMemAlloc.h"


/* declarations */

static boolean FreePageNr(RSTREE R, Rpnint *pagenr, Rint level);
static Rpnint SortedFreePageNrs(RSTREE R, Rint level, Rpnint gaps[]);
static boolean GapForLastPage(Rpnint gaps[],
                              Rlint *begin,
                              Rlint *end,
                              Rpnint *freep,
                              Rpnint *lastp);
static void RestorePage(RSTREE R,
                        boolean isdata,
                        typinterval *rect,
                        Rpnint lastp,
                        Rpnint freep,
                        boolean pagelvl_known,
                        Rint known_pagelvl);
static Rpnint ReorgDataMedium(RSTREE R);
static Rpnint ReorgDirMedium(RSTREE R);
static boolean TruncateMedia(RSTREE R, boolean shortest);
static void NewReorgNode(RSTREE R, Rint level);
static void LinkHelpToReorg(RSTREE R, Rint level);

/************************************************************************/

void UnlinkPage(RSTREE R,
                Rpnint pagenr,
                Rint level)

{
  if (level == 0) {
    if ((*R).storkind == pri) {
      /* nix */
    }
    else if ((*R).storkind == lru) {
      if (pagenr != EMPTY_BL) {
        UnlinkLRUPage(R,&(*R).DATA.str,FALSE,pagenr);
      }
    }
    else {
      /* nix */
    }
  }
  else {
    if ((*R).storkind == pri) {
      /* nix */
    }
    else if ((*R).storkind == lru) {
      if (pagenr != EMPTY_BL) {
        UnlinkLRUPage(R,&(*R).DIR.str,FALSE,pagenr);
      }
    }
    else {
      /* nix */
    }
  }
}
    
/************************************************************************/

void PutNode(RSTREE R,
             refnode nodeptr,
             Rpnint pagenr,
             Rint level)

{
  if (level == 0) {
    if ((*R).storkind == pri) {
      /* nix */
    }
    else if ((*R).storkind == lru) {
      UnlinkLRUPage(R,&(*R).DATA.str,TRUE,pagenr);
    }
    else {
      WritePage(R,&(*R).DATA.str,pagenr,nodeptr);
    }
#ifndef COUNTS_OFF
    if ((*R).count.on) {
      (*R).count.DATA.put++;
    }
#endif
  }
  else {
    if ((*R).storkind == pri) {
      /* nix */
    }
    else if ((*R).storkind == lru) {
      UnlinkLRUPage(R,&(*R).DIR.str,TRUE,pagenr);
    }
    else {
      WritePage(R,&(*R).DIR.str,pagenr,nodeptr);
    }
#ifndef COUNTS_OFF
    if ((*R).count.on) {
      (*R).count.DIR.put++;
    }
#endif
  }
}
    
/************************************************************************/

void PutExtNode(RSTREE R,
                refnode nodeptr,
                Rpnint pagenr,
                Rint level)

{
  if (level == 0) {
    if ((*R).storkind == pri) {
      WriteRAMPage(&(*R).DATA.str,pagenr,nodeptr);
    }
    else if ((*R).storkind == lru) {
      WriteLRUPage(R,&(*R).DATA.str,pagenr,nodeptr);
    }
    else {
      WritePage(R,&(*R).DATA.str,pagenr,nodeptr);
    }
#ifndef COUNTS_OFF
    if ((*R).count.on) {
      (*R).count.DATA.put++;
    }
#endif
  }
  else {
    if ((*R).storkind == pri) {
      WriteRAMPage(&(*R).DIR.str,pagenr,nodeptr);
    }
    else if ((*R).storkind == lru) {
      WriteLRUPage(R,&(*R).DIR.str,pagenr,nodeptr);
    }
    else {
      WritePage(R,&(*R).DIR.str,pagenr,nodeptr);
    }
#ifndef COUNTS_OFF
    if ((*R).count.on) {
      (*R).count.DIR.put++;
    }
#endif
  }
}
    
/************************************************************************/

void PutGetNode(RSTREE R,
                refnode *nodeptr,
                Rpnint pagenr,
                Rint level)

{
  if (level == 0) {
    if ((*R).storkind == pri) {
      WriteLinkRAMPage(&(*R).DATA.str,pagenr,nodeptr);
    }
    else if ((*R).storkind == lru) {
      WriteLinkLRUPage(R,&(*R).DATA.str,pagenr,nodeptr);
    }
    else {
      WritePage(R,&(*R).DATA.str,pagenr,*nodeptr);
    }
#ifndef COUNTS_OFF
    if ((*R).count.on) {
      (*R).count.DATA.put++;
    }
#endif
  }
  else {
    if ((*R).storkind == pri) {
      WriteLinkRAMPage(&(*R).DIR.str,pagenr,nodeptr);
    }
    else if ((*R).storkind == lru) {
      WriteLinkLRUPage(R,&(*R).DIR.str,pagenr,nodeptr);
    }
    else {
      WritePage(R,&(*R).DIR.str,pagenr,*nodeptr);
    }
#ifndef COUNTS_OFF
    if ((*R).count.on) {
      (*R).count.DIR.put++;
    }
#endif
  }
}
    
/************************************************************************/

void GetNode(RSTREE R,
             refnode *nodeptr,
             Rpnint pagenr,
             Rint level)

{
  if (level == 0) {
    if ((*R).storkind == pri) {
      LinkRAMPage(&(*R).DATA.str,pagenr,nodeptr);
    }
    else if ((*R).storkind == lru) {
      LinkLRUPage(R,&(*R).DATA.str,pagenr,nodeptr);
    }
    else {
      ReadPage(R,&(*R).DATA.str,pagenr,*nodeptr);
    }
#ifndef COUNTS_OFF
    if ((*R).count.on) {
      (*R).count.DATA.get++;
    }
#endif
  }
  else {
    if ((*R).storkind == pri) {
      LinkRAMPage(&(*R).DIR.str,pagenr,nodeptr);
    }
    else if ((*R).storkind == lru) {
      LinkLRUPage(R,&(*R).DIR.str,pagenr,nodeptr);
    }
    else {
      ReadPage(R,&(*R).DIR.str,pagenr,*nodeptr);
    }
#ifndef COUNTS_OFF
    if ((*R).count.on) {
      (*R).count.DIR.get++;
    }
#endif
  }
}

/************************************************************************/

void NewNode(RSTREE R,
             Rint level)

{
  void *ptr;
  
#ifndef COUNTS_OFF
  if ((*R).count.on) {
    if (level == 0) {
      (*R).count.DATA.demand++;
    }
    else {
      (*R).count.DIR.demand++;
    }
  }
#endif
  if ((*R).L[level].Modif) {
    PutNode(R,(*R).L[level].N,(*R).L[level].P,level);
    (*R).L[level].Modif= FALSE;
  }
  else {
    UnlinkPage(R,(*R).L[level].P,level);
  }
  ptr= (*R).L[level+1].N;
  ptr+= (*R).Pent0 + (*R).L[level+1].E * (*R).DIR.entLen + (*R).entPptts;
  (*R).L[level].P= *(refptrtosub)ptr;
  GetNode(R,&(*R).L[level].N,(*R).L[level].P,level);
}

/************************************************************************/

static void NewReorgNode(RSTREE R, Rint level)

{
  /* demands are explicitly counted in the calling functions */
  
  if ((*R).L1[level].Modif) {
    PutNode(R,(*R).L1[level].N,(*R).L1[level].P,level);
    (*R).L1[level].Modif= FALSE;
  }
  else {
    UnlinkPage(R,(*R).L1[level].P,level);
  }
  (*R).L1[level].P= (*R).LRrg[level].P;
  GetNode(R,&(*R).L1[level].N,(*R).L1[level].P,level);
}

/************************************************************************/

static void LinkHelpToReorg(RSTREE R, Rint level)

{
  refnode keepPtr;
  
  if ((*R).L1[level].Modif) {
    PutNode(R,(*R).L1[level].N,(*R).L1[level].P,level);
    (*R).L1[level].Modif= FALSE;
  }
  else {
    UnlinkPage(R,(*R).L1[level].P,level);
  }
  (*R).L1[level].P= (*R).LRrg[level].P;
  keepPtr= (*R).L1[level].N;
  (*R).L1[level].N= (*R).helpnode;
  (*R).helpnode= keepPtr;
}

/************************************************************************/

void ExtendPath(RSTREE R,
                Rint ind,
                Rint level)

{
  void *ptr;
  Rint next= level - 1;
  
  ptr= (*R).L[level].N;
  ptr+= (*R).Pent0 + ind * (*R).DIR.entLen + (*R).entPptts;
#ifndef COUNTS_OFF
  if ((*R).count.on) {
    if (next == 0) {
      (*R).count.DATA.demand++;
    }
    else {
      (*R).count.DIR.demand++;
    }
  }
#endif
  (*R).L[level].E= ind;
  if (*(refptrtosub)ptr != (*R).L[next].P) {
    if ((*R).L[next].Modif) {
      PutNode(R,(*R).L[next].N,(*R).L[next].P,next);
      (*R).L[next].Modif= FALSE;
    }
    else {
      UnlinkPage(R,(*R).L[next].P,next);
    }
    (*R).L[next].P= *(refptrtosub)ptr;
    GetNode(R,&(*R).L[next].N,(*R).L[next].P,next);
  }
}

/************************************************************************/

void ExtendReorgPath(RSTREE R,
                     typlevel Path[],
                     Rint ind,
                     Rint level,
                     typlevel *nextPath[])

{
  void *ptr;
  Rint next= level - 1;
  
#ifndef COUNTS_OFF
  if ((*R).count.on) {
    if (next == 0) {
      (*R).count.DATA.demand++;
    }
    else {
      (*R).count.DIR.demand++;
    }
  }
#endif
  (*R).LRrg[level].E= ind;
  ptr= Path[level].N;
  ptr+= (*R).Pent0 + ind * (*R).DIR.entLen + (*R).entPptts;
  (*R).LRrg[next].P= *(refptrtosub)ptr;
  
  if ((*R).L[next].P == (*R).LRrg[next].P) {     /* in N available? */
    *nextPath= (*R).L;
  }
  else {
    *nextPath= (*R).L1;
    if ((*R).L1[next].P != (*R).LRrg[next].P) {
      NewReorgNode(R,next);
    }
  }
}

/************************************************************************/

void GetPageNr(RSTREE R, Rpnint *pagenr, Rint level)

{ 
  refparameters par;
  refpagedir dpd;
  
  par= &(*R).parameters._;
  
  if (level == 0) {
  
    dpd= &(*R).DATA.PA.pagedir._;
    
    if ((*dpd).nofnumbers == 0) {
      if ((*dpd).childnr == FIRST_PD_BL_NR) {
        (*dpd).number[0]++;
        *pagenr= (*dpd).number[0];
        if ((*R).storkind == pri) {
          if (*pagenr >= (*R).DATA.str.RAM.pagelimit) {
            (*R).DATA.str.RAM.locked= TRUE;
          }
        }
      }
      else {
        if ((*R).storkind == pri) {
          ReadRAMPage(&(*R).DATA.PA.str,(*dpd).childnr,&(*R).DATA.PA.pagedir);
        }
        else {
          ReadPage(R,&(*R).DATA.PA.str,(*dpd).childnr,&(*R).DATA.PA.pagedir);
        }
        (*dpd).childnr--;
        *pagenr= (*dpd).number[MAX_PG_NRS];
        (*dpd).nofnumbers= MAX_PG_NRS - 1;
      }
    }
    else {
      *pagenr= (*dpd).number[(*dpd).nofnumbers];
      (*dpd).nofnumbers--;
    }
    if ((*R).storkind == lru) {
      EstablishLRUPage(R,&(*R).DATA.str,*pagenr);
    }
    (*par).DATA.pagecount++;
  }
  else {
  
    dpd= &(*R).DIR.PA.pagedir._;
    
    if ((*dpd).nofnumbers == 0) {
      if ((*dpd).childnr == FIRST_PD_BL_NR) {
        (*dpd).number[0]++;
        *pagenr= (*dpd).number[0];
        if ((*R).storkind == pri) {
          if (*pagenr >= (*R).DIR.str.RAM.pagelimit) {
            (*R).DIR.str.RAM.locked= TRUE;
          }
        }
      }
      else {
        if ((*R).storkind == pri) {
          ReadRAMPage(&(*R).DIR.PA.str,(*dpd).childnr,&(*R).DIR.PA.pagedir);
        }
        else {
          ReadPage(R,&(*R).DIR.PA.str,(*dpd).childnr,&(*R).DIR.PA.pagedir);
        }
        (*dpd).childnr--;
        *pagenr= (*dpd).number[MAX_PG_NRS];
        (*dpd).nofnumbers= MAX_PG_NRS - 1;
      }
    }
    else {
      *pagenr= (*dpd).number[(*dpd).nofnumbers];
      (*dpd).nofnumbers--;
    }
    if ((*R).storkind == lru) {
      EstablishLRUPage(R,&(*R).DIR.str,*pagenr);
    }
    (*par).DIR.pagecount++;
  }
  (*par).PageCount[level]++;
}

/************************************************************************/

static boolean FreePageNr(RSTREE R, Rpnint *pagenr, Rint level)

{ 
  refpagedir dpd;
    
  if (level == 0) {
  
    dpd= &(*R).DATA.PA.pagedir._;
    
    if ((*dpd).nofnumbers == 0) {
      if ((*dpd).childnr == FIRST_PD_BL_NR) {
        return FALSE;
      }
      else {
        if ((*R).storkind == pri) {
          ReadRAMPage(&(*R).DATA.PA.str,(*dpd).childnr,&(*R).DATA.PA.pagedir);
        }
        else {
          ReadPage(R,&(*R).DATA.PA.str,(*dpd).childnr,&(*R).DATA.PA.pagedir);
        }
        (*dpd).childnr--;
        *pagenr= (*dpd).number[MAX_PG_NRS];
        (*dpd).nofnumbers= MAX_PG_NRS - 1;
      }
    }
    else {
      *pagenr= (*dpd).number[(*dpd).nofnumbers];
      (*dpd).nofnumbers--;
    }
  }
  else {
  
    dpd= &(*R).DIR.PA.pagedir._;
    
    if ((*dpd).nofnumbers == 0) {
      if ((*dpd).childnr == FIRST_PD_BL_NR) {
        return FALSE;
      }
      else {
        if ((*R).storkind == pri) {
          ReadRAMPage(&(*R).DIR.PA.str,(*dpd).childnr,&(*R).DIR.PA.pagedir);
        }
        else {
          ReadPage(R,&(*R).DIR.PA.str,(*dpd).childnr,&(*R).DIR.PA.pagedir);
        }
        (*dpd).childnr--;
        *pagenr= (*dpd).number[MAX_PG_NRS];
        (*dpd).nofnumbers= MAX_PG_NRS - 1;
      }
    }
    else {
      *pagenr= (*dpd).number[(*dpd).nofnumbers];
      (*dpd).nofnumbers--;
    }
  }
  return TRUE;
}

/************************************************************************/

void PutPageNr(RSTREE R, Rpnint pagenr, Rint level)

{
  refparameters par;
  refpagedir dpd;
  
  par= &(*R).parameters._;
  
  if (level == 0) {
  
    dpd= &(*R).DATA.PA.pagedir._;
    
    if ((*dpd).nofnumbers == MAX_PG_NRS) {
      (*dpd).childnr++;
      if ((*R).storkind == pri) {
        WriteRAMPage(&(*R).DATA.PA.str,(*dpd).childnr,&(*R).DATA.PA.pagedir);
      }
      else {
        WritePage(R,&(*R).DATA.PA.str,(*dpd).childnr,&(*R).DATA.PA.pagedir);
      }
      (*dpd).nofnumbers= 1;
      (*dpd).number[1]= pagenr;
    }
    else {
      (*dpd).nofnumbers++;
      (*dpd).number[(*dpd).nofnumbers]= pagenr;
    }
    if ((*R).storkind == lru) {
      DeleteLRUPage(R,&(*R).DATA.str,pagenr);
    }
    (*par).DATA.pagecount--;
  }
  else {
  
    dpd= &(*R).DIR.PA.pagedir._;
    
    if ((*dpd).nofnumbers == MAX_PG_NRS) {
      (*dpd).childnr++;
      if ((*R).storkind == pri) {
        WriteRAMPage(&(*R).DIR.PA.str,(*dpd).childnr,&(*R).DIR.PA.pagedir);
      }
      else {
        WritePage(R,&(*R).DIR.PA.str,(*dpd).childnr,&(*R).DIR.PA.pagedir);
      }
      (*dpd).nofnumbers= 1;
      (*dpd).number[1]= pagenr;
    }
    else {
      (*dpd).nofnumbers++;
      (*dpd).number[(*dpd).nofnumbers]= pagenr;
    }
    if ((*R).storkind == lru) {
      DeleteLRUPage(R,&(*R).DIR.str,pagenr);
    }
    (*par).DIR.pagecount--;
  }
  (*par).PageCount[level]--;
}

/************************************************************************/

static Rpnint SortedFreePageNrs(RSTREE R, Rint level, Rpnint gaps[])

{
  Rpnint pagenr, i;
  
  i= 0;
  while (FreePageNr(R,&pagenr,level)) {
    gaps[i]= pagenr;
    i++;
  }
  QSortRpnints(0,i-1,gaps);
  return i;
}

/************************************************************************/
/* In this function end may count down to -1, thus end should be signed,
   which Rpnint may be not. ==> Set begin and end to type Rlint. */
   
static boolean GapForLastPage(Rpnint gaps[],
                              Rlint *begin,
                              Rlint *end,
                              Rpnint *freep,
                              Rpnint *lastp)

{
  if (*begin <= *end) {
    *freep= gaps[*begin];
    while (*lastp == gaps[*end] && *begin <= *end) {
      (*lastp)--; (*end)--;
    }
    (*begin)++;
    return *lastp > *freep;
  }
  else {
    return FALSE;
  }
}

/************************************************************************/

static void RestorePage(RSTREE R,
                        boolean isdata,
                        typinterval *rect,
                        Rpnint lastp,
                        Rpnint freep,
                        boolean pagelvl_known,
                        Rint known_pagelvl)

{
  void *ptr;
  Rint ptrlvl, pagelvl;
  typlevel *alterPath;
  Rint level= (*R).parameters._.rootlvl;
  boolean found= FALSE;
  
  if (isdata) {
    XstsDataPageNr(R,
                   (*R).L,
                   &alterPath,
                   level,
                   rect,
                   lastp,
                   &found,
                   &ptrlvl);
  }
  else {
    XstsDirPageNr(R,
                  (*R).L,
                  &alterPath,
                  level,
                  rect,
                  lastp,
                  pagelvl_known,
                  known_pagelvl,
                  &found,
                  &ptrlvl);
  }
  if (!found) {
    fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
    fprintf(stderr,"%s\n","RestorePage 1");
    (*R).RSTDone= FALSE;
  }
  
  pagelvl= ptrlvl - 1;
  if (pagelvl_known) {
    if (known_pagelvl != pagelvl) {
      fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
      fprintf(stderr,"%s\n","RestorePage 2");
      (*R).RSTDone= FALSE;
    }
  }
  else {
    /* left over from ReorgDirMedium: */
    /* fetch node(lastp) into the concerning level: */
    (*R).LRrg[pagelvl].P= lastp;
    LinkHelpToReorg(R,pagelvl);
  }
  if ((*R).L[ptrlvl].P == (*R).LRrg[ptrlvl].P || ptrlvl == (*R).parameters._.rootlvl) {
    ptr= (*R).L[ptrlvl].N;
    ptr+= (*R).Pent0 + (*R).LRrg[ptrlvl].E * (*R).DIR.entLen + (*R).entPptts;
    *(refptrtosub)ptr= freep;
    (*R).L[ptrlvl].Modif= TRUE;
  }
  else {
    ptr= (*R).L1[ptrlvl].N;
    ptr+= (*R).Pent0 + (*R).LRrg[ptrlvl].E * (*R).DIR.entLen + (*R).entPptts;
    *(refptrtosub)ptr= freep;
    (*R).L1[ptrlvl].Modif= TRUE;
  }
  
  if ((*R).storkind == lru) {
    if (isdata) {
      EstablishLRUPage(R,&(*R).DATA.str,freep);
    }
    else {
      EstablishLRUPage(R,&(*R).DIR.str,freep);
    }
  }
  
  if ((*R).L[pagelvl].P == (*R).LRrg[pagelvl].P) {
    (*R).L[pagelvl].P= freep;
    if ((*R).buflinked) {
      /* copy the node to the new position, then link the new position: */
      PutGetNode(R,&(*R).L[pagelvl].N,freep,pagelvl);
      (*R).L[pagelvl].Modif= FALSE;
    }
    else {
      (*R).L[pagelvl].Modif= TRUE;
    }
  }
  else {
    (*R).L1[pagelvl].P= freep;
    if ((*R).buflinked) {
      /* copy the node to the new position, then link the new position: */
      PutGetNode(R,&(*R).L1[pagelvl].N,freep,pagelvl);
      (*R).L1[pagelvl].Modif= FALSE;
    }
    else {
      (*R).L1[pagelvl].Modif= TRUE;
    }
  }
  
  if ((*R).storkind == lru) {
    if (isdata) {
      DeleteLRUPage(R,&(*R).DATA.str,lastp);
    }
    else {
      DeleteLRUPage(R,&(*R).DIR.str,lastp);
    }
  }
}

/************************************************************************/

static Rpnint ReorgDataMedium(RSTREE R)

{
  typinterval *rect;
  Rlint firstgap, lastgap;
  Rpnint numbnrs, numbgaps, lastp, freep;
  Rpnint *gaps;
  refpagedir dpd= &(*R).DATA.PA.pagedir._;
  
  numbnrs= (*dpd).nofnumbers + ((*dpd).childnr - FIRST_PD_BL_NR) * MAX_PG_NRS;
  if (numbnrs == 0) {
    return 0;
  }
  
  gaps= allocM(numbnrs * sizeof(Rpnint));
  numbgaps= SortedFreePageNrs(R,0,gaps);
  if (numbnrs != numbgaps) {
    fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
    fprintf(stderr,"%s\n","ReorgDataMedium 1");
    freeM(&gaps);
    (*R).RSTDone= FALSE;
    return 0;
  }
  lastp= (*R).DATA.PA.pagedir._.number[0];
  firstgap= 0;
  lastgap= numbgaps - 1;
  rect= allocM((*R).SIZErect);
  while (GapForLastPage(gaps,&firstgap,&lastgap,&freep,&lastp)) {
#ifndef COUNTS_OFF
    if ((*R).count.on) {
      (*R).count.DATA.demand++;
    }
#endif
    (*R).LRrg[0].P= lastp;
    if ((*R).L[0].P == lastp) {
      EvalDataNodesMBB(R,(*R).L[0].N,rect);
    }
    else if ((*R).L1[0].P == lastp) {
      EvalDataNodesMBB(R,(*R).L1[0].N,rect);
    }
    else {
      NewReorgNode(R,0);
      EvalDataNodesMBB(R,(*R).L1[0].N,rect);
    }
    RestorePage(R,TRUE,rect,lastp,freep,TRUE,0);
    if (! (*R).RSTDone) {
      freeM(&rect);
      freeM(&gaps);
      return numbgaps; /* break ... */
    }
    lastp--;
    (*R).DATA.PA.pagedir._.number[0]= lastp; /* ... or follow */
  }
  /* Although returning FALSE GapForLastPage may modify lastp, thus: */
  (*R).DATA.PA.pagedir._.number[0]= lastp;
  freeM(&rect);
  freeM(&gaps);
  return numbgaps;
}

/************************************************************************/

static Rpnint ReorgDirMedium(RSTREE R)

{
  typinterval *rect;
  boolean found;
  Rlint firstgap, lastgap;
  Rpnint numbnrs, numbgaps, lastp, freep;
  Rpnint *gaps;
  Rint pagelvl, lv;
  refpagedir dpd= &(*R).DIR.PA.pagedir._;
  
  numbnrs= (*dpd).nofnumbers + ((*dpd).childnr - FIRST_PD_BL_NR) * MAX_PG_NRS;
  if (numbnrs == 0) {
    return 0;
  }
  
  gaps= allocM(numbnrs * sizeof(Rpnint));
  numbgaps= SortedFreePageNrs(R,PATH_RANGE,gaps);
  if (numbnrs != numbgaps) {
    fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
    fprintf(stderr,"%s\n","ReorgDirMedium 1");
    (*R).RSTDone= FALSE;
    freeM(&gaps);
    return 0;
  }
  lastp= (*R).DIR.PA.pagedir._.number[0];
  firstgap= 0;
  lastgap= numbgaps - 1;
  rect= allocM((*R).SIZErect);
  while (GapForLastPage(gaps,&firstgap,&lastgap,&freep,&lastp)) {
#ifndef COUNTS_OFF
    if ((*R).count.on) {
      (*R).count.DIR.demand++;
    }
#endif
    lv= 1; found= FALSE;
    while (! found && lv < (*R).parameters._.rootlvl) {
      if ((*R).L[lv].P == lastp) {
        found= TRUE;
        pagelvl= lv;
        (*R).LRrg[lv].P= lastp;
        EvalDirNodesMBB(R,(*R).L[lv].N,rect);
      }
      lv++;
    }
    lv= 1;
    while (! found && lv < (*R).parameters._.rootlvl) {
      if ((*R).L1[lv].P == lastp) {
        found= TRUE;
        pagelvl= lv;
        (*R).LRrg[lv].P= lastp;
        EvalDirNodesMBB(R,(*R).L1[lv].N,rect);
      }
      lv++;
    }
    if (! found) {
      pagelvl= -1;
      GetNode(R,&(*R).helpnode,lastp,PATH_RANGE);
      EvalDirNodesMBB(R,(*R).helpnode,rect);
    }
    RestorePage(R,FALSE,rect,lastp,freep,found,pagelvl);
    if (! (*R).RSTDone) {
      freeM(&rect);
      freeM(&gaps);
      return numbgaps; /* break ... */
    }
    lastp--;
    (*R).DIR.PA.pagedir._.number[0]= lastp; /* ... or follow */
  }
  /* Although returning FALSE GapForLastPage may modify lastp, thus: */
  (*R).DIR.PA.pagedir._.number[0]= lastp;
  freeM(&rect);
  freeM(&gaps);
  return numbgaps;
}

/************************************************************************/

void ReorgMedia(RSTREE R,
                boolean shortest,
                boolean *reorgactiv,
                boolean *truncactiv)

{
  Rpnint data_numbgaps, dir_numbgaps;
  Rint lv;
  
  for (lv= 0; lv <= (*R).parameters._.rootlvl; lv++) {
    (*R).L1[lv].P= EMPTY_BL;
    (*R).LRrg[lv].P= EMPTY_BL;
    (*R).LRrg[lv].E= -1;
  }
  
  data_numbgaps= ReorgDataMedium(R);
  dir_numbgaps= ReorgDirMedium(R);
  *reorgactiv= data_numbgaps > 0 || dir_numbgaps > 0;
  for (lv= 0; lv < (*R).parameters._.rootlvl; lv++) {
    if ((*R).L1[lv].Modif) {
      PutNode(R,(*R).L1[lv].N,(*R).L1[lv].P,lv);
      (*R).L1[lv].Modif= FALSE;
    }
    else {
      UnlinkPage(R,(*R).L1[lv].P,lv);
    }
  }
  if ((*R).storkind == pri) {
    *truncactiv= FALSE;
  }
  else {
    *truncactiv= TruncateMedia(R,shortest);
  }
}

/************************************************************************/

static boolean TruncateMedia(RSTREE R, boolean shortest)

{
  Rpint data_need, dataPD_need, dir_need, dirPD_need,
        data_eval, dataPD_eval, dir_eval, dirPD_eval;
  boolean truncated= FALSE;
  
  data_need= ((*R).DATA.PA.pagedir._.number[0] + 1) * (*R).DATA.str.psize;
  dataPD_need= (FIRST_PD_BL_NR + 1) * (*R).DATA.PA.str.psize;
  dir_need= ((*R).DIR.PA.pagedir._.number[0] + 1) * (*R).DIR.str.psize;
  dirPD_need= (FIRST_PD_BL_NR + 1) * (*R).DIR.PA.str.psize;
  
  if (shortest) {
    data_eval= LenOfF((*R).DATA.str.f);     /* LenOfF returns -1 on error */
    dataPD_eval= LenOfF((*R).DATA.PA.str.f);
    dir_eval= LenOfF((*R).DIR.str.f);
    dirPD_eval= LenOfF((*R).DIR.PA.str.f);
  }
  else {
    data_eval= LenOfF((*R).DATA.str.f) / FileReductionFactor;
    dataPD_eval= LenOfF((*R).DATA.PA.str.f) / FileReductionFactor;
    dir_eval= LenOfF((*R).DIR.str.f) / FileReductionFactor;
    dirPD_eval= LenOfF((*R).DIR.PA.str.f) / FileReductionFactor;
  }
  if (data_need < data_eval) {     /* never TRUE on LenOfF error */
    truncated= TRUE;
    if (! TruncateF((*R).DATA.str.f,data_need)) {
      (*R).RSTDone= FALSE;
      return truncated;
    }
  }
  if (dataPD_need < dataPD_eval) {
    truncated= TRUE;
    if (! TruncateF((*R).DATA.PA.str.f,dataPD_need)) {
      (*R).RSTDone= FALSE;
      return truncated;
    }
  }
  if (dir_need < dir_eval) {
    truncated= TRUE;
    if (! TruncateF((*R).DIR.str.f,dir_need)) {
      (*R).RSTDone= FALSE;
      return truncated;
    }
  }
  if (dirPD_need < dirPD_eval) {
    truncated= TRUE;
    if (! TruncateF((*R).DIR.PA.str.f,dirPD_need)) {
      (*R).RSTDone= FALSE;
      return truncated;
    }
  }
  return truncated;
}

/************************************************************************/
/* PathLRUConsistence checks if locks of pages in the LRU buffer exactly
   correspond to the pages in the path. */

boolean PathLRUConsistence(RSTREE R)

{
  char s[80];
  Rint i;
  Rpnint remlocks;
  boolean consistent;
  t_FPB pageLocks;
  FPst filePage;
  
  consistent= TRUE;
  
  pageLocks= LRUPagesLocked((*R).LRU,&(*R).RSTDone);
  
  /* dir: simulate exclusion of 1 lock for each non empty page of the path: */
  /*      this way find multiple locked pages: */
  filePage.f= (*R).DIR.str.f;
  for (i= (*R).parameters._.rootlvl; i >= 1; i--) {
    if ((*R).L[i].P != EMPTY_BL) {
      filePage.p= (*R).L[i].P;
      remlocks= FPB_Exclude(&pageLocks,filePage,1);
      if (remlocks != 0) {
        fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
        fprintf(stderr,"%s\n","PathLRUConsistence 1:");
        fprintf(stderr,strans("Lock inconsistency: level/page: %I/%N\n",s),i,(*R).L[i].P);
        fprintf(stderr,strans("Remaining locks: %I\n",s),remlocks);
        fprintf(stderr,"%s\n","after simulating exclusion of 1 lock.");
        consistent= FALSE;
      }
    }
  }
  /* data: same as for the directory: */
  if ((*R).L[0].P != EMPTY_BL) {
    filePage.f= (*R).DATA.str.f;
    filePage.p= (*R).L[0].P;
    remlocks= FPB_Exclude(&pageLocks,filePage,1);
    if (remlocks != 0) {
      fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
      fprintf(stderr,"%s\n","PathLRUConsistence 2:");
      fprintf(stderr,strans("Lock inconsistency: level/page: 0/%N\n",s),(*R).L[i].P);
      fprintf(stderr,strans("Remaining locks: %I\n",s),remlocks);
      fprintf(stderr,"%s\n","after simulating exclusion of 1 lock.");
      consistent= FALSE;
    }
  }
  
  /* check for remaining locks of other pages of the corresponding files: */
  if (FPB_NumbElems(pageLocks) != 0) {
    do {
      remlocks= FPB_Pop(&pageLocks,&filePage);
      if (filePage.f == (*R).DIR.str.f || filePage.f == (*R).DATA.str.f) {
        fprintf(stderr,"%s\n","RST: FATAL INTERNAL ERROR");
        fprintf(stderr,"%s\n","PathLRUConsistence 3:");
        fprintf(stderr,strans("%s %N* f%I p%N\n",s),"Redundant lock:",remlocks,filePage.f,filePage.p);
        consistent= FALSE;
      }
    } while (remlocks != 0);
  }
  
  return consistent;
}

/************************************************************************/
