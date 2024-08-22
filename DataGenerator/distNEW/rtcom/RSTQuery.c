/* ----- RSTQuery.c ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//


#include "RSTQuery.h"
#include "RSTUtil.h"
#include "RSTPageInOut.h"
#include "RSTFileAccess.h"
#include "RSTMemAlloc.h"

/* ------------------------------------------------------------------- */
/* functions repeated here to be available for inlining: */
/* ------------------------------------------------------------------- */
/* identical to Covers() in RSTUtil: */
static boolean Q_Covers(Rint numbOfDim,
                        const typinterval *intvC,
                        const typinterval *intv);
/* identical to RectsEql() in RSTUtil: */
static boolean Q_RectsEql(Rint numbOfDim,
                          const typinterval *intv1,
                          const typinterval *intv2);

/************************************************************************/

boolean RectXsts(RSTREE R,
                 Rint level,
                 const typinterval *rectangle,
                 refinfo info)

{
  refnode n;
  void *ptrNi, *ptrInfo;
  Rint i, nofent, entLen;
  boolean found;
  
  n= (*R).L[level].N;
  nofent= (*n).s.nofentries;
  ptrNi= n;
  ptrNi+= (*R).Pent0;
  
  found= FALSE;
  i= 0;
  if (level != 0) {
    entLen= (*R).DIR.entLen;
    do {
      if (Q_Covers((*R).numbofdim,ptrNi,rectangle)) {
        ExtendPath(R,i,level);
        found= RectXsts(R,level-1,rectangle,info);
      }
      ptrNi+= entLen;
      i++;
    } while (! found && i < nofent);
  }
  else {
    entLen= (*R).DATA.entLen;
    while (! found && i < nofent) {
      found= Q_RectsEql((*R).numbofdim,ptrNi,rectangle);
      if (found) {
        (*R).L[level].E= i;
        ptrInfo= ptrNi + (*R).entPinfo;
        memcpy(info,ptrInfo,(*R).SIZEinfo);
      }
      ptrNi+= entLen;
      i++;
    }
  }
  return found;
}

/************************************************************************/

boolean RecordXsts(RSTREE R,
                   Rint level,
                   const typinterval *rectangle,
                   const typinfo *info,
                   InfoCmpFunc Qualified,
                   void *dPtr)

{
  refnode n;
  void *ptrNi, *ptrInfo;
  Rint i, nofent, entLen;
  boolean found;
  
  n= (*R).L[level].N;
  nofent= (*n).s.nofentries;
  ptrNi= n;
  ptrNi+= (*R).Pent0;
  
  found= FALSE;
  i= 0;
  if (level != 0) {
    entLen= (*R).DIR.entLen;
    do {
      if (Q_Covers((*R).numbofdim,ptrNi,rectangle)) {
        ExtendPath(R,i,level);
        found= RecordXsts(R,level-1,rectangle,info,Qualified,dPtr);
      }
      ptrNi+= entLen;
      i++;
    } while (! found && i < nofent);
  }
  else {
    entLen= (*R).DATA.entLen;
    while (! found && i < nofent) {
      if (Q_RectsEql((*R).numbofdim,ptrNi,rectangle)) {
        ptrInfo= ptrNi + (*R).entPinfo;
        found= info == NULL || Qualified((t_RT)R,ptrInfo,(*R).SIZEinfo,info,dPtr);
        if (found) {
          (*R).L[level].E= i;
        }
      }
      ptrNi+= entLen;
      i++;
    }
  }
  return found;
}

/************************************************************************/

void XstsRgn(RSTREE R,
             Rint level,
             const typinterval *qRects,
             Rint qRectQty,
             void *qPtr,
             QueryFunc DirQuery,
             QueryFunc DataQuery,
             boolean *found)

{
  refnode n;
  void *ptrNi;
  Rint i, nofent, entLen;
  
  n= (*R).L[level].N;
  nofent= (*n).s.nofentries;
  ptrNi= n;
  ptrNi+= (*R).Pent0;
  
  i= 0;
  if (level != 0) {
    entLen= (*R).DIR.entLen;
    do {
      if (DirQuery((t_RT)R,(*R).numbofdim,ptrNi,qRects,qRectQty,qPtr)) {
        ExtendPath(R,i,level);
        XstsRgn(R,level-1,qRects,qRectQty,qPtr,DirQuery,DataQuery,found);
      }
      ptrNi+= entLen;
      i++;
    } while (! *found && i < nofent);
  }
  else {
    entLen= (*R).DATA.entLen;
    while (! *found && i < nofent) {
      if (DataQuery((t_RT)R,(*R).numbofdim,ptrNi,qRects,qRectQty,qPtr)) {
        (*R).L[level].E= i;
        *found= TRUE;
      }
      ptrNi+= entLen;
      i++;
    }
  }
}

/************************************************************************/

void RgnCnt(RSTREE R,
            Rint level,
            const typinterval *qRects,
            Rint qRectQty,
            void *qPtr,
            QueryFunc DirQuery,
            QueryFunc DataQuery,
            Rlint *keysqualifying)

{
  refnode n;
  void *ptrNi;
  Rint i, nofent, entLen;
  
  n= (*R).L[level].N;
  nofent= (*n).s.nofentries;
  ptrNi= n;
  ptrNi+= (*R).Pent0;
  
  i= 0;
  if (level != 0) {
    entLen= (*R).DIR.entLen;
    do {
      if (DirQuery((t_RT)R,(*R).numbofdim,ptrNi,qRects,qRectQty,qPtr)) {
        ExtendPath(R,i,level);
        RgnCnt(R,level-1,qRects,qRectQty,qPtr,DirQuery, DataQuery,keysqualifying);
      }
      ptrNi+= entLen;
      i++;
    } while(i < nofent);
  }
  else {
    entLen= (*R).DATA.entLen;
    while (i < nofent) {
      if (DataQuery((t_RT)R,(*R).numbofdim,ptrNi,qRects,qRectQty,qPtr)) {
        (*R).L[level].E= i;
        (*keysqualifying)++;
      }
      ptrNi+= entLen;
      i++;
    }
  }
}

/************************************************************************/
/* see also RgnQueryFastUnsafe below, directly writing into the node */

void RgnQuery(RSTREE R,
              Rint level,
              const typinterval *qRects,
              Rint qRectQty,
              void *qPtr,
              QueryFunc DirQuery,
              QueryFunc DataQuery,
              void *mPtr,
              boolean *finish,
              QueryManageFunc Manage)

{
  refnode n;
  void *ptrNi, *ptrInfo;
  Rint i, nofent, entLen;
  boolean modify= FALSE;
  
  n= (*R).L[level].N;
  nofent= (*n).s.nofentries;
  ptrNi= n;
  ptrNi+= (*R).Pent0;
  
  i= 0;
  if (level != 0) {
    entLen= (*R).DIR.entLen;
    do {
      if (*finish) {return;}
      
      if (DirQuery((t_RT)R,(*R).numbofdim,ptrNi,qRects,qRectQty,qPtr)) {
        ExtendPath(R,i,level);
        RgnQuery(R,level-1,qRects,qRectQty,qPtr,DirQuery,DataQuery,mPtr,finish,Manage);
      }
      ptrNi+= entLen;
      i++;
    } while(i < nofent);
  }
  else {
    entLen= (*R).DATA.entLen;
    while (i < nofent) {
      if (*finish) {return;}

      if (DataQuery((t_RT)R,(*R).numbofdim,ptrNi,qRects,qRectQty,qPtr)) {
        (*R).L[level].E= i;
        
        CopyRect((*R).numbofdim,ptrNi,(*R).RQA_rect);
        ptrInfo= ptrNi + (*R).entPinfo;
        memcpy((*R).RQA_info,ptrInfo,(*R).SIZEinfo);
        Manage((t_RT)R,
               (*R).numbofdim,
               (*R).RQA_rect,
               (*R).RQA_info,
               (*R).SIZEinfo,
               mPtr,
               &modify,
               finish);
        if (modify) {
          memcpy(ptrInfo,(*R).RQA_info,(*R).SIZEinfo);
          (*R).L[level].Modif= TRUE;
        }
      }
      ptrNi+= entLen;
      i++;
    }
  }
}

/************************************************************************/
/* see also AllFastUnsafe below, directly writing into the node */

void All(RSTREE R,
         Rint level,
         void *mPtr,
         boolean *finish,
         QueryManageFunc Manage)

{
  refnode n;
  void *ptrNi, *ptrInfo;
  Rint i, nofent, entLen;
  boolean modify= FALSE;
  
  n= (*R).L[level].N;
  nofent= (*n).s.nofentries;
  ptrNi= n;
  ptrNi+= (*R).Pent0;
  
  i= 0;
  if (level != 0) {
    entLen= (*R).DIR.entLen;
    do {
      if (*finish) {return;}
      
      ExtendPath(R,i,level);
      All(R,level-1,mPtr,finish,Manage);
      ptrNi+= entLen;
      i++;
    } while(i < nofent);
  }
  else {
    entLen= (*R).DATA.entLen;
    while (i < nofent) {
      if (*finish) {return;}

      (*R).L[level].E= i;
      
      CopyRect((*R).numbofdim,ptrNi,(*R).RQA_rect);
      ptrInfo= ptrNi + (*R).entPinfo;
      memcpy((*R).RQA_info,ptrInfo,(*R).SIZEinfo);
      Manage((t_RT)R,
             (*R).
             numbofdim,
             (*R).RQA_rect,
             (*R).RQA_info,
             (*R).SIZEinfo,
             mPtr,
             &modify,
             finish);
      if (modify) {
        memcpy(ptrInfo,(*R).RQA_info,(*R).SIZEinfo);
        (*R).L[level].Modif= TRUE;
      }
      ptrNi+= entLen;
      i++;
    }
  }
}

/************************************************************************/
/* NOTE: RgnQueryFastUnsafe() via Manage grants direct access to the stored
   data, and thus allows to modify the rectangle (which usually destroys
   consistency). Besides that: A modified information part may be written
   back though this was not intended, i.e. setting parameter "modify" of
   Manage to FALSE may be ineffective. Reason: The flag "Modif" of the
   visited data node may have already been set to TRUE by an insertion
   sequence before the query was started. This is due to the fact that the
   implementation allows different operations (insertion and query here) to
   alternate without having to write back (and clear) the path buffer in the
   meantime. */
   
void RgnQueryFastUnsafe(RSTREE R,
                        Rint level,
                        typinterval *qRects,
                        Rint qRectQty,
                        void *qPtr,
                        QueryFunc DirQuery,
                        QueryFunc DataQuery,
                        void *mPtr,
                        boolean *finish,
                        QueryManageFunc Manage)

{
  refnode n;
  void *ptrNi, *ptrInfo;;
  Rint i, nofent, entLen;
  boolean modify= FALSE;
  
  n= (*R).L[level].N;
  nofent= (*n).s.nofentries;
  ptrNi= n;
  ptrNi+= (*R).Pent0;
  
  i= 0;
  if (level != 0) {
    entLen= (*R).DIR.entLen;
    do {
      if (*finish) {return;}
      
      if (DirQuery((t_RT)R,(*R).numbofdim,ptrNi,qRects,qRectQty,qPtr)) {
        ExtendPath(R,i,level);
        RgnQueryFastUnsafe(R,level-1,qRects,qRectQty,qPtr,DirQuery,DataQuery,mPtr,finish,Manage);
      }
      ptrNi+= entLen;
      i++;
    } while(i < nofent);
  }
  else {
    entLen= (*R).DATA.entLen;
    while (i < nofent) {
      if (*finish) {return;}

      if (DataQuery((t_RT)R,(*R).numbofdim,ptrNi,qRects,qRectQty,qPtr)) {
        (*R).L[level].E= i;
        
        ptrInfo= ptrNi + (*R).entPinfo;
        Manage((t_RT)R,(*R).numbofdim,ptrNi,ptrInfo,(*R).SIZEinfo,mPtr,&modify,finish);
        if (modify) {
          (*R).L[level].Modif= TRUE;
        }
      }
      ptrNi+= entLen;
      i++;
    }
  }
}

/************************************************************************/
/* NOTE: AllFastUnsafe() via Manage grants direct access to the stored
   data, and thus allows to modify the rectangle (which usually destroys
   consistency). Besides that: A modified information part may be written
   back though this was not intended, i.e. setting parameter "modify" of
   Manage to FALSE may be ineffective. Reason: The flag "Modif" of the
   visited data node may have already been set to TRUE by an insertion
   sequence before the query was started. This is due to the fact that the
   implementation allows different operations (insertion and query here) to
   alternate without having to write back (and clear) the path buffer in the
   meantime. */

void AllFastUnsafe(RSTREE R,
                   Rint level,
                   void *mPtr,
                   boolean *finish,
                   QueryManageFunc Manage)

{
  refnode n;
  void *ptrNi, *ptrInfo;
  Rint i, nofent, entLen;
  boolean modify= FALSE;
  
  n= (*R).L[level].N;
  nofent= (*n).s.nofentries;
  ptrNi= n;
  ptrNi+= (*R).Pent0;
  
  i= 0;
  if (level != 0) {
    entLen= (*R).DIR.entLen;
    do {
      if (*finish) {return;}
      
      ExtendPath(R,i,level);
      AllFastUnsafe(R,level-1,mPtr,finish,Manage);
      ptrNi+= entLen;
      i++;
    } while(i < nofent);
  }
  else {
    entLen= (*R).DATA.entLen;
    while (i < nofent) {
      if (*finish) {return;}

      (*R).L[level].E= i;
      
      ptrInfo= ptrNi + (*R).entPinfo;
      Manage((t_RT)R,(*R).numbofdim,ptrNi,ptrInfo,(*R).SIZEinfo,mPtr,&modify,finish);
      if (modify) {
        (*R).L[level].Modif= TRUE;
      }
      ptrNi+= entLen;
      i++;
    }
  }
}

/************************************************************************/

void PushNodePriorQ(DISTQ DQ,
                    RSTREE R,
                    refnode n,
                    Rint level)

{
  Rint i;
  boolean push;
  void *ptrNi, *ptrNinfo, *ptrNptts;
  void *ptrElem0, *ptrElemInfo, *ptrElemPtts;
  refDHelem elem= allocM((*R).DHELen);
  
  ptrNi= n;
  ptrNi+= (*R).Pent0;
  ptrElem0= elem;
  ptrElem0+= (*R).DHEPent0;
  
  (*elem).s.level= level;
  if (level == 0) {
    ptrNinfo= ptrNi + (*R).entPinfo;
    ptrElemInfo= ptrElem0 + (*R).entPinfo;
    for (i= 0; i < (*n).s.nofentries; i++) {
      if ((*DQ).DataFilter((t_RT)R,(*R).numbofdim,ptrNi,(*DQ).fRects,(*DQ).fRectsQty,(*DQ).fPtr)) {
        if ((*DQ).stDist == 0.0) {
          CopyRect((*R).numbofdim,ptrNi,ptrElem0);
          memcpy(ptrElemInfo,ptrNinfo,(*R).SIZEinfo);
          (*elem).s.dist= (*DQ).DataDist(R,(*DQ).CalcDist,(*DQ).qPoint,ptrNi);
          if (! (*DQ).DH_Inst(&(*DQ).H,elem)) {
            setRSTerr(R,"Query.PushNodePriorQ",reAlloc);
            (*R).RSTDone= FALSE;
            freeM(&elem);
            return;
          }
        }
        else {
          (*elem).s.dist= (*DQ).DataDist(R,(*DQ).CalcDist,(*DQ).qPoint,ptrNi);
          if ((*DQ).sort == inc) {
            push= (*elem).s.dist >= (*DQ).stDist;
          }
          else {
            push= (*elem).s.dist <= (*DQ).stDist;
          }
          if (push) {
            CopyRect((*R).numbofdim,ptrNi,ptrElem0);
            memcpy(ptrElemInfo,ptrNinfo,(*R).SIZEinfo);
            if (! (*DQ).DH_Inst(&(*DQ).H,elem)) {
              setRSTerr(R,"Query.PushNodePriorQ",reAlloc);
              (*R).RSTDone= FALSE;
              freeM(&elem);
              return;
            }
          }
        }
      }
      ptrNi+= (*R).DATA.entLen;
      ptrNinfo+= (*R).DATA.entLen;
    }
  }
  else {
    ptrNptts= ptrNi + (*R).entPptts;
    ptrElemPtts= ptrElem0 + (*R).entPptts;
    for (i= 0; i < (*n).s.nofentries; i++) {
      if ((*DQ).DirFilter((t_RT)R,(*R).numbofdim,ptrNi,(*DQ).fRects,(*DQ).fRectsQty,(*DQ).fPtr)) {
        if ((*DQ).stDist == 0.0) {
          push= TRUE;
        }
        else {
          if ((*DQ).sort == inc) {
            push= (*DQ).DirStDist(R,(*DQ).CalcDist,(*DQ).qPoint,ptrNi) >= (*DQ).stDist;
          }
          else {
            push= (*DQ).DirStDist(R,(*DQ).CalcDist,(*DQ).qPoint,ptrNi) <= (*DQ).stDist;
          }
        }
        if (push) {
          CopyRect((*R).numbofdim,ptrNi,ptrElem0);
          memcpy(ptrElemPtts,ptrNptts,SIZEptrtosub);
          (*elem).s.dist= (*DQ).DirDist(R,(*DQ).CalcDist,(*DQ).qPoint,ptrNi);
          if (! (*DQ).DH_Inst(&(*DQ).H,elem)) {
            setRSTerr(R,"Query.PushNodePriorQ",reAlloc);
            (*R).RSTDone= FALSE;
            freeM(&elem);
            return;
          }
        }
      }
      ptrNi+= (*R).DIR.entLen;
      ptrNptts+= (*R).DIR.entLen;
    }
  }
  freeM(&elem);
}

/************************************************************************/

boolean DistQueryNext(DISTQ DQ,
                      RSTREE R,
                      typinterval *rectangle,
                      refinfo info,
                      Rfloat *rawDist)

{
  Rint level;
  void *ptrElem0, *ptrElemInfo, *ptrElemPtts;
  refDHelem elem= allocM((*R).DHELen);
  refnode node= allocM((*R).UNInodeLen);
  refnode nodeptr= node;
  /* depending on the storage type, *nodeptr used or nodeptr is bended!! */
  
  for (;;) {
    if ((*DQ).DH_Pop(&(*DQ).H,elem)) {
      ptrElem0= elem;
      ptrElem0+= (*R).DHEPent0;
      if ((*elem).s.level == 0) {
        CopyRect((*R).numbofdim,ptrElem0,rectangle);
        ptrElemInfo= ptrElem0 + (*R).entPinfo;
        memcpy(info,ptrElemInfo,(*R).SIZEinfo);
        *rawDist= (*elem).s.dist;
        freeM(&node);
        freeM(&elem);
        return TRUE;
      }
      else {
        level= (*elem).s.level - 1;
        ptrElemPtts= ptrElem0 + (*R).entPptts;
        GetNode(R,&nodeptr,*(refptrtosub)ptrElemPtts,level);
        PushNodePriorQ(DQ,R,nodeptr,level);
        UnlinkPage(R,*(refptrtosub)ptrElemPtts,level);
#ifndef COUNTS_OFF
        if ((*R).count.on) {
          if (level == 0) {
            (*R).count.DATA.demand++;
          }
          else {
            (*R).count.DIR.demand++;
          } /* formal count of demands (#demand == #read here) */
        }
#endif
      }
    }
    else {
      freeM(&node);
      freeM(&elem);
      return FALSE;
    }
  }
}

/************************************************************************/

void LvlDmp(RSTREE R,
            Rint lv,
            void *buf,
            Rint bufSz,
            RectConvFunc Convert,
            RSTName nm[],
            void* buffers[])

{
  refnode n;
  void *ptrNi;
  Rint i;
  
  n= (*R).L[lv].N;
  ptrNi= n;
  ptrNi+= (*R).Pent0;
  
  i= 0;
  if (lv > 1) {
    do {
      Convert((t_RT)R,(*R).numbofdim,ptrNi,buf,bufSz);
      //if (! WrBytes(f[lv],buf,bufSz)) {
      if (! WrBufBytes(buffers[lv],buf)) {
        (*R).RSTDone= FALSE;
      }
      ExtendPath(R,i,lv);
      LvlDmp(R,lv-1,buf,bufSz,Convert,nm,buffers);
      i++;
      ptrNi+= (*R).DIR.entLen;
    } while(i < (*n).s.nofentries);
  }
  else {
    do {
      Convert((t_RT)R,(*R).numbofdim,ptrNi,buf,bufSz);
      //if (! WrBytes(f[lv],buf,bufSz)) {
      if (! WrBufBytes(buffers[lv],buf)) {
        (*R).RSTDone= FALSE;
      }
      i++;
      ptrNi+= (*R).DIR.entLen;
    } while(i < (*n).s.nofentries);
  }
}

/************************************************************************/

void RectInLevelCnt(RSTREE R,
                    Rint level,
                    Rint targetlevel,
                    typinterval *rectangle,
                    Rint *keysqualifying)

{
  refnode n;
  void *ptrNi;
  Rint i;
  
  n= (*R).L[level].N;
  ptrNi= n;
  ptrNi+= (*R).Pent0;
  
  i= 0;
  if (level != targetlevel) {
    do {
      if (Q_Covers((*R).numbofdim,ptrNi,rectangle)) {
        ExtendPath(R,i,level);
        RectInLevelCnt(R,level-1,targetlevel,rectangle,keysqualifying);
      }
      i++;
      ptrNi+= (*R).DIR.entLen;
    } while(i < (*n).s.nofentries);
  }
  else {
    if (level != 0) {
      do {
        if (Q_RectsEql((*R).numbofdim,ptrNi,rectangle)) {
          (*R).L[level].E= i;
          (*keysqualifying)++;
        }
        i++;
        ptrNi+= (*R).DIR.entLen;
      } while(i < (*n).s.nofentries);
    }
    else {
      while (i < (*n).s.nofentries) {
        if (Q_RectsEql((*R).numbofdim,ptrNi,rectangle)) {
          (*R).L[level].E= i;
          (*keysqualifying)++;
        }
        i++;
        ptrNi+= (*R).DATA.entLen;
      }
    }
  }
}

/************************************************************************/

void XstsPageNrInLevel(RSTREE R,
                       Rint level,
                       Rint targetlevel,
                       typinterval *rectangle,
                       Rpnint pagenr,
                       boolean *found)

{
  refnode n;
  void *ptrNi;
  Rint i;
  
  n= (*R).L[level].N;
  ptrNi= n;
  
  i= 0;
  if (level != targetlevel) {
    ptrNi+= (*R).Pent0;
    do {
      if (Q_Covers((*R).numbofdim,ptrNi,rectangle)) {
        ExtendPath(R,i,level);
        XstsPageNrInLevel(R,level-1,targetlevel,rectangle,pagenr,found);
      }
      i++;
      ptrNi+= (*R).DIR.entLen;
    } while (! *found && i < (*n).s.nofentries);
  }
  else {
    if (level != 0) {
      ptrNi+= (*R).Pent0 + (*R).entPptts;
      do {
        if (*(refptrtosub)ptrNi == pagenr) {
          (*R).L[level].E= i;
          *found= TRUE;
        }
        i++;
        ptrNi+= (*R).DIR.entLen;
      } while (! *found && i < (*n).s.nofentries);
    }
    else {
      ptrNi+= (*R).Pent0 + (*R).entPinfo;
      /* for test purpose (only?) we have a number in front of the info */
      while (! *found && i < (*n).s.nofentries) {
        if ((*(refinfo)ptrNi).tag == pagenr) {
          (*R).L[level].E= i;
          *found= TRUE;
        }
        i++;
        ptrNi+= (*R).DATA.entLen;
      }
    }
  }
}

/************************************************************************/
/* This function uses the L- and L1-path, and the LRrg-path (P,E) !! */

void XstsDirPageNr(RSTREE R,
                   typlevel Path[],
                   typlevel *nextPath[],
                   Rint level,
                   typinterval *rectangle,
                   Rpnint pagenr,
                   boolean pagelvl_known,
                   Rint known_pagelvl,
                   boolean *found,
                   Rint *levelfound)

{
  Rint i, lastlevel;
  refnode n;
  void *ptrNi, *ptrNiPtts;
  
  n= Path[level].N;
  ptrNi= n;
  ptrNi+= (*R).Pent0;
  ptrNiPtts= ptrNi + (*R).entPptts;
  
  if (pagelvl_known) {
    lastlevel= known_pagelvl + 1;
  }
  else {
    lastlevel= 2;
  }
  i= 0;
  if (level > lastlevel) {
    do {
      if (pagelvl_known) {
        if (Q_Covers((*R).numbofdim,ptrNi,rectangle)) {
          ExtendReorgPath(R,Path,i,level,nextPath);
          XstsDirPageNr(R,
                        *nextPath,
                        nextPath,
                        level-1,
                        rectangle,
                        pagenr,
                        pagelvl_known,
                        known_pagelvl,
                        found,
                        levelfound);
        }
      }
      else {
        if (*(refptrtosub)ptrNiPtts == pagenr) {
          (*R).LRrg[level].E= i;
          *found= TRUE;
          *levelfound= level;
        }
        else if (Q_Covers((*R).numbofdim,ptrNi,rectangle)) {
          ExtendReorgPath(R,Path,i,level,nextPath);
          XstsDirPageNr(R,
                        *nextPath,
                        nextPath,
                        level-1,
                        rectangle,
                        pagenr,
                        pagelvl_known,
                        known_pagelvl,
                        found,
                        levelfound);
        }
      }
      i++;
      ptrNi+= (*R).DIR.entLen;
      ptrNiPtts+= (*R).DIR.entLen;
    } while (! *found && i < (*n).s.nofentries);
  }
  else {
    do {
      if (*(refptrtosub)ptrNiPtts == pagenr) {
        (*R).LRrg[level].E= i;
        *found= TRUE;
        *levelfound= level;
      }
      i++;
      ptrNi+= (*R).DIR.entLen;
      ptrNiPtts+= (*R).DIR.entLen;
    } while (! *found && i < (*n).s.nofentries);
  }
}

/************************************************************************/
/* This function uses the L- and L1-path, and the LRrg-path (P,E) !! */

void XstsDataPageNr(RSTREE R,
                    typlevel Path[],
                    typlevel *nextPath[],
                    Rint level,
                    typinterval *rectangle,
                    Rpnint pagenr,
                    boolean *found,
                    Rint *levelfound)

{
  Rint i;
  refnode n;
  void *ptrNi, *ptrNiPtts;
  
  n= Path[level].N;
  ptrNi= n;
  ptrNi+= (*R).Pent0;
  ptrNiPtts= ptrNi + (*R).entPptts;
  
  i= 0;
  if (level > 1) {
    do {
      if (Q_Covers((*R).numbofdim,ptrNi,rectangle)) {
        ExtendReorgPath(R,Path,i,level,nextPath);
        XstsDataPageNr(R,*nextPath,nextPath,level-1,rectangle,pagenr,found,levelfound);
      }
      i++;
      ptrNi+= (*R).DIR.entLen;
      ptrNiPtts+= (*R).DIR.entLen;
    } while (! *found && i < (*n).s.nofentries);
  }
  else {
    do {
      if (*(refptrtosub)ptrNiPtts == pagenr) {
        (*R).LRrg[level].E= i;
        *found= TRUE;
        *levelfound= level;
      }
      i++;
      ptrNi+= (*R).DIR.entLen;
      ptrNiPtts+= (*R).DIR.entLen;
    } while (! *found && i < (*n).s.nofentries);
  }
}

/* ------------------------------------------------------------------- */
/* functions repeated here to be available for inlining: */
/* ------------------------------------------------------------------- */
/* identical to Covers() in RSTUtil: */

static boolean Q_Covers(Rint numbOfDim,
                        const typinterval *intvC,
                        const typinterval *intv)

{
  Rint d;
  
  d= 0;
  do {
    if ((*intvC).l > (*intv).l || (*intvC).h < (*intv).h) {
      return FALSE;
    }
    d++; intvC++; intv++;
  } while (d < numbOfDim);
  return TRUE;
}

/***********************************************************************/
/* identical to RectsEql() in RSTUtil: */

static boolean Q_RectsEql(Rint numbOfDim,
                          const typinterval *intv1,
                          const typinterval *intv2)

{
  Rint d;
  
  d= 0;
  do {
    if ((*intv1).l != (*intv2).l || (*intv1).h != (*intv2).h) {
      return FALSE;
    }
    d++; intv1++; intv2++;
  } while (d < numbOfDim);
  return TRUE;
}

/***********************************************************************/

