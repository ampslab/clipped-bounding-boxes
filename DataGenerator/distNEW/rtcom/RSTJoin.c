/* ----- RSTJoin.c ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//


#include "RSTJoin.h"
#include "RSTUtil.h"
#include "RSTPageInOut.h"
#include "RSTMemAlloc.h"


/* declarations */

static void XJnRgnCnt(RSTREE R, RSTREE Rx,
                      boolean order,
                      Rint level,
                      const typinterval *qRects,
                      Rint qRectQty,
                      void *qPtr,
                      QueryFunc DataQuery,
                      const typinterval *rectOther,
                      JoinFunc DirJoin,
                      JoinFunc DataJoin,
                      Rlint *keysqualifying);

static void XJnRgnQuery(RSTREE R, RSTREE Rx,
                        boolean order,
                        Rint level,
                        const typinterval *qRects,
                        Rint qRectQty,
                        void *qPtr,
                        QueryFunc DataQuery,
                        const typinterval *rectOther,
                        refinfo refInfoOther,
                        JoinFunc DirJoin,
                        JoinFunc DataJoin,
                        void *mPtr,
                        boolean *finish,
                        JoinManageFunc Manage);

static void XSpJnRgnCnt(RSTREE R, RSTREE Rx,
                        boolean order,
                        Rint level,
                        const typinterval *qRects,
                        Rint qRectQty,
                        void *qPtr,
                        QueryFunc DataQuery,
                        const typinterval *rectOther,
                        RectCompFunc DirSpJoin,
                        JoinFunc DataJoin,
                        Rlint *keysqualifying);

static void XSpJnRgnQuery(RSTREE R, RSTREE Rx,
                          boolean order,
                          Rint level,
                          const typinterval *qRects,
                          Rint qRectQty,
                          void *qPtr,
                          QueryFunc DataQuery,
                          const typinterval *rectOther,
                          refinfo refInfoOther,
                          RectCompFunc DirSpJoin,
                          JoinFunc DataJoin,
                          void *mPtr,
                          boolean *finish,
                          JoinManageFunc Manage);

static void SpJnRgnCnt(RSTREE R, RSTREE Rx,
                       boolean order,
                       Rint level,
                       typinterval *rectOther,
                       Rlint *keysqualifying);

static void SpJnRgnQuery(RSTREE R, RSTREE Rx,
                         boolean order,
                         Rint level,
                         typinterval *rectOther,
                         refinfo refInfoOther,
                         void *mPtr,
                         boolean *finish,
                         JoinManageFunc Manage);


/************************************************************************/

void XJnCnt(RSTREE R1,
            Rint level1,
            const typinterval *R1qRects,
            Rint R1qRectQty,
            void *R1qPtr,
            QueryFunc Dir1Query,
            QueryFunc Data1Query,
            RSTREE R2,
            Rint level2,
            const typinterval *R2qRects,
            Rint R2qRectQty,
            void *R2qPtr,
            QueryFunc Dir2Query,
            QueryFunc Data2Query,
            JoinFunc DirJoin,
            JoinFunc DataJoin,
            Rlint *keysqualifying,
            Rlint *cntlmt)

{
  char s[80];
  refnode DIN1, DIN2;
  void *ptrDIN1i, *ptrDIN2j;
  Rint entLenDIN1, entLenDIN2;
  refnode DAN;
  void *ptrDANi;
  Rint entLenDAN;
  Rlint downqualifying;
  Rint i, j;
  
  if (level1 != 0 && level2 != 0) {
    
    entLenDIN1= (*R1).DIR.entLen;
    entLenDIN2= (*R2).DIR.entLen;
    DIN1= (*R1).L[level1].N;
    DIN2= (*R2).L[level2].N;
    
    ptrDIN1i= DIN1;
    ptrDIN1i+= (*R1).Pent0;
    for (i= 0; i < (*DIN1).s.nofentries; i++) {
      ptrDIN2j= DIN2;
      ptrDIN2j+= (*R2).Pent0;
      for (j= 0; j < (*DIN2).s.nofentries; j++) {
        if (Dir1Query((t_RT)R1,(*R1).numbofdim,ptrDIN1i,R1qRects,R1qRectQty,R1qPtr) && Dir2Query((t_RT)R2,(*R2).numbofdim,ptrDIN2j,R2qRects,R2qRectQty,R2qPtr)) {
          if (DirJoin((t_RT)R1,(t_RT)R2,(*R1).numbofdim,ptrDIN1i,ptrDIN2j)) {
            ExtendPath(R1,i,level1);
            ExtendPath(R2,j,level2);
            XJnCnt(R1,level1-1,R1qRects,R1qRectQty,R1qPtr,Dir1Query,Data1Query,
                   R2,level2-1,R2qRects,R2qRectQty,R2qPtr,Dir2Query,Data2Query,
                   DirJoin,DataJoin,
                   keysqualifying,cntlmt);
          }
        }
        ptrDIN2j+= entLenDIN2;
      }
      ptrDIN1i+= entLenDIN1;
    }
  }
  else {
    if (level1 == 0) {
    
      entLenDAN= (*R1).DATA.entLen;
      DAN= (*R1).L[level1].N;
      
      ptrDANi= DAN;
      ptrDANi+= (*R1).Pent0;
      for (i= 0; i < (*DAN).s.nofentries; i++) {
        if (Data1Query((t_RT)R1,(*R1).numbofdim,ptrDANi,R1qRects,R1qRectQty,R1qPtr)) {
          downqualifying= 0;
          XJnRgnCnt(R2,R1,FALSE,level2,
                    R2qRects,R2qRectQty,R2qPtr,Data2Query,
                    ptrDANi,DirJoin,DataJoin,
                    &downqualifying);
          *keysqualifying+= downqualifying;
          if (VerboseJoinCount) {
            if (*keysqualifying > *cntlmt) {
              fprintf(stdout,strans("%s%10L%s\n",s),"More than",*cntlmt," record pairs.");
              if (*cntlmt == 0) {
                *cntlmt= 1;
              }
              else {
                *cntlmt*= 2;
              }
            }
          }
        }
        ptrDANi+= entLenDAN;
      }
    }
    else { /* level2 == 0 */
      
      entLenDAN= (*R2).DATA.entLen;
      DAN= (*R2).L[level2].N;
      
      ptrDANi= DAN;
      ptrDANi+= (*R2).Pent0;
      for (i= 0; i < (*DAN).s.nofentries; i++) {
        if (Data2Query((t_RT)R2,(*R2).numbofdim,ptrDANi,R2qRects,R2qRectQty,R2qPtr)) {
          downqualifying= 0;
          XJnRgnCnt(R1,R2,TRUE,level1,
                    R1qRects,R1qRectQty,R1qPtr,Data1Query,
                    ptrDANi,DirJoin,DataJoin,
                    &downqualifying);
          *keysqualifying+= downqualifying;
          if (VerboseJoinCount) {
            if (*keysqualifying > *cntlmt) {
              fprintf(stdout,strans("%s%10L%s\n",s),"More than",*cntlmt," record pairs.");
              if (*cntlmt == 0) {
                *cntlmt= 1;
              }
              else {
                *cntlmt*= 2;
              }
            }
          }
        }
        ptrDANi+= entLenDAN;
      }
    }
  }
}

/************************************************************************/

static void XJnRgnCnt(RSTREE R, RSTREE Rx,
                      boolean order,
                      Rint level,
                      const typinterval *qRects,
                      Rint qRectQty,
                      void *qPtr,
                      QueryFunc DataQuery,
                      const typinterval *rectOther,
                      JoinFunc DirJoin,
                      JoinFunc DataJoin,
                      Rlint *keysqualifying)

{
  refnode DIN;
  void *ptrDIN;
  Rint entLenDIN;
  refnode DAN;
  void *ptrDANi;
  Rint entLenDAN;
  Rint i;
  
  
  if (level != 0) {
  
    entLenDIN= (*R).DIR.entLen;
    DIN= (*R).L[level].N;
    ptrDIN= DIN;
    ptrDIN+= (*R).Pent0;
    
    if (order) {
      for (i= 0; i < (*DIN).s.nofentries; i++) {
        if (DirJoin((t_RT)R,(t_RT)Rx,(*R).numbofdim,ptrDIN,rectOther)) {
          ExtendPath(R,i,level);
          XJnRgnCnt(R,Rx,order,level-1,
                    qRects,qRectQty,qPtr,DataQuery,
                    rectOther,DirJoin,DataJoin,
                    keysqualifying);
        }
        ptrDIN+= entLenDIN;
      }
    }
    else {
      for (i= 0; i < (*DIN).s.nofentries; i++) {
        if (DirJoin((t_RT)Rx,(t_RT)R,(*Rx).numbofdim,rectOther,ptrDIN)) {
          ExtendPath(R,i,level);
          XJnRgnCnt(R,Rx,order,level-1,
                    qRects,qRectQty,qPtr,DataQuery,
                    rectOther,DirJoin,DataJoin,
                    keysqualifying);
        }
        ptrDIN+= entLenDIN;
      }
    }
  }
  else {
  
    DAN= (*R).L[level].N;
    ptrDANi= DAN;
    ptrDANi+= (*R).Pent0;
    entLenDAN= (*R).DATA.entLen;
    
    if (order) {
      for (i= 0; i < (*DAN).s.nofentries; i++) {
        if (DataQuery((t_RT)R,(*R).numbofdim,ptrDANi,qRects,qRectQty,qPtr)) {
          if (DataJoin((t_RT)R,(t_RT)Rx,(*R).numbofdim,ptrDANi,rectOther)) {
            (*R).L[level].E= i;
            (*keysqualifying)++;
          }
        }
        ptrDANi+= entLenDAN;
      }
    }
    else {
      for (i= 0; i < (*DAN).s.nofentries; i++) {
        if (DataQuery((t_RT)Rx,(*Rx).numbofdim,ptrDANi,qRects,qRectQty,qPtr)) {
          if (DataJoin((t_RT)Rx,(t_RT)R,(*Rx).numbofdim,rectOther,ptrDANi)) {
            (*R).L[level].E= i;
            (*keysqualifying)++;
          }
        }
        ptrDANi+= entLenDAN;
      }
    }
  }
}

/************************************************************************/

void XJn(RSTREE R1,
         Rint level1,
         const typinterval *R1qRects,
         Rint R1qRectQty,
         void *R1qPtr,
         QueryFunc Dir1Query,
         QueryFunc Data1Query,
         RSTREE R2,
         Rint level2,
         const typinterval *R2qRects,
         Rint R2qRectQty,
         void *R2qPtr,
         QueryFunc Dir2Query,
         QueryFunc Data2Query,
         JoinFunc DirJoin,
         JoinFunc DataJoin,
         void *mPtr,
         boolean *finish,
         JoinManageFunc Manage)

{
  refnode DIN1, DIN2;
  void *ptrDIN1i, *ptrDIN2j;
  Rint entLenDIN1, entLenDIN2;
  refnode DAN;
  void *ptrDANi, *ptrDANinfo;
  Rint entLenDAN;
  Rint i, j;
    
  if (level1 != 0 && level2 != 0) {
    
    entLenDIN1= (*R1).DIR.entLen;
    entLenDIN2= (*R2).DIR.entLen;
    DIN1= (*R1).L[level1].N;
    DIN2= (*R2).L[level2].N;
    
    ptrDIN1i= DIN1;
    ptrDIN1i+= (*R1).Pent0;
    for (i= 0; i < (*DIN1).s.nofentries; i++) {
      if (*finish) {return;}
      ptrDIN2j= DIN2;
      ptrDIN2j+= (*R2).Pent0;
      for (j= 0; j < (*DIN2).s.nofentries; j++) {
        if (*finish) {return;}
        if (Dir1Query((t_RT)R1,(*R1).numbofdim,ptrDIN1i,R1qRects,R1qRectQty,R1qPtr) && Dir2Query((t_RT)R2,(*R2).numbofdim,ptrDIN2j,R2qRects,R2qRectQty,R2qPtr)) {
          if (DirJoin((t_RT)R1,(t_RT)R2,(*R1).numbofdim,ptrDIN1i,ptrDIN2j)) {
            ExtendPath(R1,i,level1);
            ExtendPath(R2,j,level2);
            XJn(R1,level1-1,R1qRects,R1qRectQty,R1qPtr,Dir1Query,Data1Query,
                R2,level2-1,R2qRects,R2qRectQty,R2qPtr,Dir2Query,Data2Query,
                DirJoin,DataJoin,
                mPtr,finish,Manage);
          }
        }
        ptrDIN2j+= entLenDIN2;
      }
      ptrDIN1i+= entLenDIN1;
    }
  }
  else {
    if (level1 == 0) {
      
      entLenDAN= (*R1).DATA.entLen;
      DAN= (*R1).L[level1].N;
      
      ptrDANi= DAN;
      ptrDANi+= (*R1).Pent0;
      ptrDANinfo= ptrDANi + (*R1).entPinfo;
      for (i= 0; i < (*DAN).s.nofentries; i++) {
        if (*finish) {return;}
        if (Data1Query((t_RT)R1,(*R1).numbofdim,ptrDANi,R1qRects,R1qRectQty,R1qPtr)) {
          XJnRgnQuery(R2,R1,FALSE,level2,R2qRects,R2qRectQty,R2qPtr,Data2Query,
                      ptrDANi,ptrDANinfo,
                      DirJoin,DataJoin,
                      mPtr,finish,Manage);
        }
        ptrDANi+= entLenDAN;
        ptrDANinfo+= entLenDAN;
      }
    }
    else { /* level2 == 0 */
      
      entLenDAN= (*R2).DATA.entLen;
      DAN= (*R2).L[level2].N;
      
      ptrDANi= DAN;
      ptrDANi+= (*R2).Pent0;
      ptrDANinfo= ptrDANi + (*R2).entPinfo;
      for (i= 0; i < (*DAN).s.nofentries; i++) {
        if (*finish) {return;}
        if (Data2Query((t_RT)R2,(*R2).numbofdim,ptrDANi,R2qRects,R2qRectQty,R2qPtr)) {
          XJnRgnQuery(R1,R2,TRUE,level1,R1qRects,R1qRectQty,R1qPtr,Data1Query,
                      ptrDANi,ptrDANinfo,
                      DirJoin,DataJoin,
                      mPtr,finish,Manage);
        }
        ptrDANi+= entLenDAN;
        ptrDANinfo+= entLenDAN;
      }
    }
  }
}

/************************************************************************/

static void XJnRgnQuery(RSTREE R, RSTREE Rx,
                        boolean order,
                        Rint level,
                        const typinterval *qRects,
                        Rint qRectQty,
                        void *qPtr,
                        QueryFunc DataQuery,
                        const typinterval *rectOther,
                        refinfo refInfoOther,
                        JoinFunc DirJoin,
                        JoinFunc DataJoin,
                        void *mPtr,
                        boolean *finish,
                        JoinManageFunc Manage)
{
  refnode DIN;
  void *ptrDIN;
  Rint entLenDIN;
  refnode DAN;
  void *ptrDANi;
  void *ptrDANinfo;
  Rint entLenDAN;
  Rint i;
  typinterval *rectR= allocM((*R).SIZErect);
  typinterval *rectRx= allocM((*Rx).SIZErect);
  refinfo infoR= allocM((*R).SIZEinfo);
  refinfo infoRx= allocM((*Rx).SIZEinfo);
  
  if (level != 0) {
    
    entLenDIN= (*R).DIR.entLen;
    DIN= (*R).L[level].N;
    ptrDIN= DIN;
    ptrDIN+= (*R).Pent0;
    
    if (order) {
      for (i= 0; i < (*DIN).s.nofentries; i++) {
        if (*finish) {return;}
        if (DirJoin((t_RT)R,(t_RT)Rx,(*R).numbofdim,ptrDIN,rectOther)) {
          ExtendPath(R,i,level);
          XJnRgnQuery(R,Rx,order,level-1,qRects,qRectQty,qPtr,DataQuery,
                      rectOther,refInfoOther,DirJoin,DataJoin,
                      mPtr,finish,Manage);
        }
        ptrDIN+= entLenDIN;
      }
    }
    else {
      for (i= 0; i < (*DIN).s.nofentries; i++) {
        if (*finish) {return;}
        if (DirJoin((t_RT)Rx,(t_RT)R,(*Rx).numbofdim,rectOther,ptrDIN)) {
          ExtendPath(R,i,level);
          XJnRgnQuery(R,Rx,order,level-1,qRects,qRectQty,qPtr,DataQuery,
                      rectOther,refInfoOther,DirJoin,DataJoin,
                      mPtr,finish,Manage);
        }
        ptrDIN+= entLenDIN;
      }
    }
  }
  else {
    
    entLenDAN= (*R).DATA.entLen;
    DAN= (*R).L[level].N;
    ptrDANi= DAN;
    ptrDANi+= (*R).Pent0;
    ptrDANinfo= ptrDANi + (*R).entPinfo;
    
    if (order) {
      for (i= 0; i < (*DAN).s.nofentries; i++) {
        if (*finish) {return;}
        if (DataQuery((t_RT)R,(*R).numbofdim,ptrDANi,qRects,qRectQty,qPtr)) {
          if (DataJoin((t_RT)R,(t_RT)Rx,(*R).numbofdim,ptrDANi,rectOther)) {
            (*R).L[level].E= i;
            CopyRect((*R).numbofdim,ptrDANi,rectR);
            CopyRect((*Rx).numbofdim,rectOther,rectRx);
            memcpy(infoR,ptrDANinfo,(*R).SIZEinfo);
            memcpy(infoRx,refInfoOther,(*Rx).SIZEinfo);
            Manage((t_RT)R,(t_RT)Rx,(*R).numbofdim,rectR,rectRx,infoR,infoRx,(*R).SIZEinfo,(*Rx).SIZEinfo,mPtr,finish);
          }
        }
        ptrDANi+= entLenDAN;
        ptrDANinfo+= entLenDAN;
      }
    }
    else {
      for (i= 0; i < (*DAN).s.nofentries; i++) {
        if (*finish) {return;}
        if (DataQuery((t_RT)Rx,(*Rx).numbofdim,ptrDANi,qRects,qRectQty,qPtr)) {
          if (DataJoin((t_RT)Rx,(t_RT)R,(*Rx).numbofdim,rectOther,ptrDANi)) {
            (*R).L[level].E= i;
            CopyRect((*R).numbofdim,ptrDANi,rectR);
            CopyRect((*Rx).numbofdim,rectOther,rectRx);
            memcpy(infoR,ptrDANinfo,(*R).SIZEinfo);
            memcpy(infoRx,refInfoOther,(*Rx).SIZEinfo);
            Manage((t_RT)Rx,(t_RT)R,(*Rx).numbofdim,rectRx,rectR,infoRx,infoR,(*Rx).SIZEinfo,(*R).SIZEinfo,mPtr,finish);
          }
        }
        ptrDANi+= entLenDAN;
        ptrDANinfo+= entLenDAN;
      }
    }
  }
  freeM(&rectR);
  freeM(&rectRx);
  freeM(&infoR);
  freeM(&infoRx);
}

/************************************************************************/
/* internal comparison counting in R1 only */

void XSpJnCnt(RSTREE R1,
              Rint level1,
              const typinterval *R1qRects,
              Rint R1qRectQty,
              void *R1qPtr,
              QueryFunc Dir1Query,
              QueryFunc Data1Query,
              RSTREE R2,
              Rint level2,
              const typinterval *R2qRects,
              Rint R2qRectQty,
              void *R2qPtr,
              QueryFunc Dir2Query,
              QueryFunc Data2Query,
              typinterval *interRect,
              RectCompFunc DirSpJoin,
              JoinFunc DataJoin,
              Rlint *keysqualifying,
              Rlint *cntlmt)

{
  char s[80];
  refnode DIN1, DIN2;
  void *ptrDIN1i, *ptrDIN2j;
  Rint entLenDIN1, entLenDIN2;
  refnode DAN;
  void *ptrDANi;
  Rint entLenDAN;
  Rlint downqualifying;
  IndexArray I1= allocM((*R1).IndArrLen);
  IndexArray I2= allocM((*R2).IndArrLen);
  Rint i, j;
  
  if (level1 != 0 && level2 != 0) {
    
    entLenDIN1= (*R1).DIR.entLen;
    entLenDIN2= (*R2).DIR.entLen;
    DIN1= (*R1).L[level1].N;
    DIN2= (*R2).L[level2].N;
    
    ptrDIN1i= DIN1;
    ptrDIN1i+= (*R1).Pent0;
    for (i= 0; i < (*DIN1).s.nofentries; i++) {
      I1[i]= DirOvlps(R1,ptrDIN1i,interRect);
      ptrDIN1i+= entLenDIN1;
    }
    ptrDIN2j= DIN2;
    ptrDIN2j+= (*R2).Pent0;
    for (j= 0; j < (*DIN2).s.nofentries; j++) {
      I2[j]= DirOvlps(R1,ptrDIN2j,interRect);
      ptrDIN2j+= entLenDIN2;
    }
    
    ptrDIN1i= DIN1;
    ptrDIN1i+= (*R1).Pent0;
    for (i= 0; i < (*DIN1).s.nofentries; i++) {
      if (I1[i]) {
        ptrDIN2j= DIN2;
        ptrDIN2j+= (*R2).Pent0;
        for (j= 0; j < (*DIN2).s.nofentries; j++) {
          if (I2[j]) {
            if (Dir1Query((t_RT)R1,(*R1).numbofdim,ptrDIN1i,R1qRects,R1qRectQty,R1qPtr) && Dir2Query((t_RT)R2,(*R2).numbofdim,ptrDIN2j,R2qRects,R2qRectQty,R2qPtr)) {
              if (DirSpJoin(R1,ptrDIN1i,ptrDIN2j)) {
                GetOvlpRect((*R1).numbofdim,ptrDIN1i,ptrDIN2j,interRect);
                ExtendPath(R1,i,level1);
                ExtendPath(R2,j,level2);
                XSpJnCnt(R1,level1-1,R1qRects,R1qRectQty,R1qPtr,Dir1Query,Data1Query,
                         R2,level2-1,R2qRects,R2qRectQty,R2qPtr,Dir2Query,Data2Query,
                         interRect,
                         DirSpJoin,DataJoin,
                         keysqualifying,cntlmt);
              }
            }
          }
          ptrDIN2j+= entLenDIN2;
        }
      }
      ptrDIN1i+= entLenDIN1;
    }
  }
  else if (level1 == 0) {
    
    entLenDAN= (*R1).DATA.entLen;
    DAN= (*R1).L[level1].N;
    
    ptrDANi= DAN;
    ptrDANi+= (*R1).Pent0;
    for (i= 0; i < (*DAN).s.nofentries; i++) {
      if (DataOvlps(R1,ptrDANi,interRect)) {
        if (Data1Query((t_RT)R1,(*R1).numbofdim,ptrDANi,R1qRects,R1qRectQty,R1qPtr)) {
          downqualifying= 0;
          XSpJnRgnCnt(R2,R1,FALSE,level2,
                      R2qRects,R2qRectQty,R2qPtr,Data2Query,
                      ptrDANi,DirSpJoin,DataJoin,
                      &downqualifying);
          *keysqualifying+= downqualifying;
          if (VerboseJoinCount) {
            if (*keysqualifying > *cntlmt) {
              fprintf(stdout,strans("%s%10L%s\n",s),"More than",*cntlmt," record pairs.");
              if (*cntlmt == 0) {
                *cntlmt= 1;
              }
              else {
                *cntlmt*= 2;
              }
            }
          }
        }
      }
      ptrDANi+= entLenDAN;
    }
  }
  else { /* level2 == 0 */
    
    entLenDAN= (*R2).DATA.entLen;
    DAN= (*R2).L[level2].N;
    
    ptrDANi= DAN;
    ptrDANi+= (*R2).Pent0;
    for (i= 0; i < (*DAN).s.nofentries; i++) {
      if (DataOvlps(R1,ptrDANi,interRect)) {
        if (Data2Query((t_RT)R2,(*R2).numbofdim,ptrDANi,R2qRects,R2qRectQty,R2qPtr)) {
          downqualifying= 0;
          XSpJnRgnCnt(R1,R2,TRUE,level1,
                      R1qRects,R1qRectQty,R1qPtr,Data1Query,
                      ptrDANi,DirSpJoin,DataJoin,
                      &downqualifying);
          *keysqualifying+= downqualifying;
          if (VerboseJoinCount) {
            if (*keysqualifying > *cntlmt) {
              fprintf(stdout,strans("%s%10L%s\n",s),"More than",*cntlmt," record pairs.");
              if (*cntlmt == 0) {
                *cntlmt= 1;
              }
              else {
                *cntlmt*= 2;
              }
            }
          }
        }
      }
      ptrDANi+= entLenDAN;
    }
  }
  freeM(&I1);
  freeM(&I2);
}

/************************************************************************/
/* internal comparison counting in R1(XSpJnCnt) only */

static void XSpJnRgnCnt(RSTREE R, RSTREE Rx,
                        boolean order,
                        Rint level,
                        const typinterval *qRects,
                        Rint qRectQty,
                        void *qPtr,
                        QueryFunc DataQuery,
                        const typinterval *rectOther,
                        RectCompFunc DirSpJoin,
                        JoinFunc DataJoin,
                        Rlint *keysqualifying)

{
  refnode DIN;
  void *ptrDIN;
  Rint entLenDIN;
  refnode DAN;
  void *ptrDANi;
  Rint entLenDAN;
  Rint i;
  
  
  if (level != 0) {
  
    entLenDIN= (*R).DIR.entLen;
    DIN= (*R).L[level].N;
    ptrDIN= DIN;
    ptrDIN+= (*R).Pent0;
    
    if (order) {
      for (i= 0; i < (*DIN).s.nofentries; i++) {
        if (DirSpJoin(R,ptrDIN,rectOther)) {
          ExtendPath(R,i,level);
          XSpJnRgnCnt(R,Rx,order,level-1,
                      qRects,qRectQty,qPtr,DataQuery,
                      rectOther,DirSpJoin,DataJoin,
                      keysqualifying);
        }
        ptrDIN+= entLenDIN;
      }
    }
    else {
      for (i= 0; i < (*DIN).s.nofentries; i++) {
        if (DirSpJoin(Rx,rectOther,ptrDIN)) {
          ExtendPath(R,i,level);
          XSpJnRgnCnt(R,Rx,order,level-1,
                      qRects,qRectQty,qPtr,DataQuery,
                      rectOther,DirSpJoin,DataJoin,
                      keysqualifying);
        }
        ptrDIN+= entLenDIN;
      }
    }
  }
  else {
  
    entLenDAN= (*R).DATA.entLen;
    DAN= (*R).L[level].N;
    ptrDANi= DAN;
    ptrDANi+= (*R).Pent0;
    
    if (order) {
      for (i= 0; i < (*DAN).s.nofentries; i++) {
        if (DataQuery((t_RT)R,(*R).numbofdim,ptrDANi,qRects,qRectQty,qPtr)) {
          if (DataJoin((t_RT)R,(t_RT)Rx,(*R).numbofdim,ptrDANi,rectOther)) {
            (*R).L[level].E= i;
            (*keysqualifying)++;
          }
        }
        ptrDANi+= entLenDAN;
      }
    }
    else {
      for (i= 0; i < (*DAN).s.nofentries; i++) {
        if (DataQuery((t_RT)Rx,(*Rx).numbofdim,ptrDANi,qRects,qRectQty,qPtr)) {
          if (DataJoin((t_RT)Rx,(t_RT)R,(*Rx).numbofdim,rectOther,ptrDANi)) {
            (*R).L[level].E= i;
            (*keysqualifying)++;
          }
        }
        ptrDANi+= entLenDAN;
      }
    }
  }
}

/************************************************************************/
/* internal comparison counting in R1 only */

void XSpJn(RSTREE R1,
           Rint level1,
           const typinterval *R1qRects,
           Rint R1qRectQty,
           void *R1qPtr,
           QueryFunc Dir1Query,
           QueryFunc Data1Query,
           RSTREE R2,
           Rint level2,
           const typinterval *R2qRects,
           Rint R2qRectQty,
           void *R2qPtr,
           QueryFunc Dir2Query,
           QueryFunc Data2Query,
           typinterval *interRect,
           RectCompFunc DirSpJoin,
           JoinFunc DataJoin,
           void *mPtr,
           boolean *finish,
           JoinManageFunc Manage)

{
  refnode DIN1, DIN2;
  void *ptrDIN1i, *ptrDIN2j;
  Rint entLenDIN1, entLenDIN2;
  refnode DAN;
  void *ptrDANi;
  void *ptrDANinfo;
  Rint entLenDAN;
  IndexArray I1= allocM((*R1).IndArrLen);
  IndexArray I2= allocM((*R2).IndArrLen);
  Rint i, j;
    
  if (level1 != 0 && level2 != 0) {
    
    entLenDIN1= (*R1).DIR.entLen;
    entLenDIN2= (*R2).DIR.entLen;
    DIN1= (*R1).L[level1].N;
    DIN2= (*R2).L[level2].N;
    
    ptrDIN1i= DIN1;
    ptrDIN1i+= (*R1).Pent0;
    for (i= 0; i < (*DIN1).s.nofentries; i++) {
      I1[i]= DirOvlps(R1,ptrDIN1i,interRect);
      ptrDIN1i+= entLenDIN1;
    }
    ptrDIN2j= DIN2;
    ptrDIN2j+= (*R2).Pent0;
    for (j= 0; j < (*DIN2).s.nofentries; j++) {
      I2[j]= DirOvlps(R1,ptrDIN2j,interRect);
      ptrDIN2j+= entLenDIN2;
    }
    
    ptrDIN1i= DIN1;
    ptrDIN1i+= (*R1).Pent0;
    for (i= 0; i < (*DIN1).s.nofentries; i++) {
      if (*finish) {return;}
      if (I1[i]) {
        ptrDIN2j= DIN2;
        ptrDIN2j+= (*R2).Pent0;
        for (j= 0; j < (*DIN2).s.nofentries; j++) {
          if (*finish) {return;}
          if (I2[j]) {
            if (Dir1Query((t_RT)R1,(*R1).numbofdim,ptrDIN1i,R1qRects,R1qRectQty,R1qPtr) && Dir2Query((t_RT)R2,(*R2).numbofdim,ptrDIN2j,R2qRects,R2qRectQty,R2qPtr)) {
              if (DirSpJoin(R1,ptrDIN1i,ptrDIN2j)) {
                GetOvlpRect((*R1).numbofdim,ptrDIN1i,ptrDIN2j,interRect);
                ExtendPath(R1,i,level1);
                ExtendPath(R2,j,level2);
                XSpJn(R1,level1-1,R1qRects,R1qRectQty,R1qPtr,Dir1Query,Data1Query,
                      R2,level2-1,R2qRects,R2qRectQty,R2qPtr,Dir2Query,Data2Query,
                      interRect,
                      DirSpJoin,DataJoin,
                      mPtr,finish,Manage);
              }
            }
          }
          ptrDIN2j+= entLenDIN2;
        }
      }
      ptrDIN1i+= entLenDIN1;
    }
  }
  else if (level1 == 0) {
      
    entLenDAN= (*R1).DATA.entLen;
    DAN= (*R1).L[level1].N;
    
    ptrDANi= DAN;
    ptrDANi+= (*R1).Pent0;
    ptrDANinfo= ptrDANi + (*R1).entPinfo;
    for (i= 0; i < (*DAN).s.nofentries; i++) {
      if (*finish) {return;}
      if (DataOvlps(R1,ptrDANi,interRect)) {
        if (Data1Query((t_RT)R1,(*R1).numbofdim,ptrDANi,R1qRects,R1qRectQty,R1qPtr)) {
          XSpJnRgnQuery(R2,R1,FALSE,level2,R2qRects,R2qRectQty,R2qPtr,Data2Query,
                        ptrDANi,ptrDANinfo,
                        DirSpJoin,DataJoin,
                        mPtr,finish,Manage);
        }
      }
      ptrDANi+= entLenDAN;
      ptrDANinfo+= entLenDAN;
    }
  }
  else { /* level2 == 0 */
    
    entLenDAN= (*R2).DATA.entLen;
    DAN= (*R2).L[level2].N;
    
    ptrDANi= DAN;
    ptrDANi+= (*R2).Pent0;
    ptrDANinfo= ptrDANi + (*R2).entPinfo;
    for (i= 0; i < (*DAN).s.nofentries; i++) {
      if (*finish) {return;}
      if (DataOvlps(R1,ptrDANi,interRect)) {
        if (Data2Query((t_RT)R2,(*R2).numbofdim,ptrDANi,R2qRects,R2qRectQty,R2qPtr)) {
          XSpJnRgnQuery(R1,R2,TRUE,level1,R1qRects,R1qRectQty,R1qPtr,Data1Query,
                        ptrDANi,ptrDANinfo,
                        DirSpJoin,DataJoin,
                        mPtr,finish,Manage);
        }
      }
      ptrDANi+= entLenDAN;
      ptrDANinfo+= entLenDAN;
    }
  }
  freeM(&I1);
  freeM(&I2);
}

/************************************************************************/
/* internal comparison counting in R1(XSpJn) only */

static void XSpJnRgnQuery(RSTREE R, RSTREE Rx,
                          boolean order,
                          Rint level,
                          const typinterval *qRects,
                          Rint qRectQty,
                          void *qPtr,
                          QueryFunc DataQuery,
                          const typinterval *rectOther,
                          refinfo refInfoOther,
                          RectCompFunc DirSpJoin,
                          JoinFunc DataJoin,
                          void *mPtr,
                          boolean *finish,
                          JoinManageFunc Manage)
{
  refnode DIN;
  void *ptrDIN;
  Rint entLenDIN;
  refnode DAN;
  void *ptrDANi;
  void *ptrDANinfo;
  Rint entLenDAN;
  Rint i;
  typinterval *rectR= allocM((*R).SIZErect);
  typinterval *rectRx= allocM((*Rx).SIZErect);
  refinfo infoR= allocM((*R).SIZEinfo);
  refinfo infoRx= allocM((*Rx).SIZEinfo);
  
  if (level != 0) {
    
    entLenDIN= (*R).DIR.entLen;
    DIN= (*R).L[level].N;
    ptrDIN= DIN;
    ptrDIN+= (*R).Pent0;

    if (order) {
      for (i= 0; i < (*DIN).s.nofentries; i++) {
        if (*finish) {return;}
        if (DirSpJoin(R,ptrDIN,rectOther)) {
          ExtendPath(R,i,level);
          XSpJnRgnQuery(R,Rx,order,level-1,qRects,qRectQty,qPtr,DataQuery,
                        rectOther,refInfoOther,DirSpJoin,DataJoin,
                        mPtr,finish,Manage);
        }
        ptrDIN+= entLenDIN;
      }
    }
    else {
      for (i= 0; i < (*DIN).s.nofentries; i++) {
        if (*finish) {return;}
        if (DirSpJoin(Rx,rectOther,ptrDIN)) {
          ExtendPath(R,i,level);
          XSpJnRgnQuery(R,Rx,order,level-1,qRects,qRectQty,qPtr,DataQuery,
                        rectOther,refInfoOther,DirSpJoin,DataJoin,
                        mPtr,finish,Manage);
        }
        ptrDIN+= entLenDIN;
      }
    }
  }
  else {
    
    entLenDAN= (*R).DATA.entLen;
    DAN= (*R).L[level].N;
    ptrDANi= DAN;
    ptrDANi+= (*R).Pent0;
    ptrDANinfo= ptrDANi + (*R).entPinfo;
    
    if (order) {
      for (i= 0; i < (*DAN).s.nofentries; i++) {
        if (*finish) {return;}
        if (DataQuery((t_RT)R,(*R).numbofdim,ptrDANi,qRects,qRectQty,qPtr)) {
          if (DataJoin((t_RT)R,(t_RT)Rx,(*R).numbofdim,ptrDANi,rectOther)) {
            (*R).L[level].E= i;
            CopyRect((*R).numbofdim,ptrDANi,rectR);
            CopyRect((*Rx).numbofdim,rectOther,rectRx);
            memcpy(infoR,ptrDANinfo,(*R).SIZEinfo);
            memcpy(infoRx,refInfoOther,(*Rx).SIZEinfo);
            Manage((t_RT)R,(t_RT)Rx,(*R).numbofdim,rectR,rectRx,infoR,infoRx,(*R).SIZEinfo,(*Rx).SIZEinfo,mPtr,finish);
          }
        }
        ptrDANi+= entLenDAN;
        ptrDANinfo+= entLenDAN;
      }
    }
    else {
      for (i= 0; i < (*DAN).s.nofentries; i++) {
        if (*finish) {return;}
        if (DataQuery((t_RT)Rx,(*Rx).numbofdim,ptrDANi,qRects,qRectQty,qPtr)) {
          if (DataJoin((t_RT)Rx,(t_RT)R,(*Rx).numbofdim,rectOther,ptrDANi)) {
            (*R).L[level].E= i;
            CopyRect((*R).numbofdim,ptrDANi,rectR);
            CopyRect((*Rx).numbofdim,rectOther,rectRx);
            memcpy(infoR,ptrDANinfo,(*R).SIZEinfo);
            memcpy(infoRx,refInfoOther,(*Rx).SIZEinfo);
            Manage((t_RT)Rx,(t_RT)R,(*Rx).numbofdim,rectRx,rectR,infoRx,infoR,(*Rx).SIZEinfo,(*R).SIZEinfo,mPtr,finish);
          }
        }
        ptrDANi+= entLenDAN;
        ptrDANinfo+= entLenDAN;
      }
    }
  }
  freeM(&rectR);
  freeM(&rectRx);
  freeM(&infoR);
  freeM(&infoRx);
}

/************************************************************************/
/* internal comparison counting in R1 only */

void SpJnCnt(RSTREE R1,
             Rint level1,
             RSTREE R2,
             Rint level2,
             typinterval *interRect,
             Rlint *keysqualifying,
             Rlint *cntlmt)

{
  char s[80];
  refnode DIN1, DIN2;
  void *ptrDIN1i, *ptrDIN2j;
  Rint entLenDIN1, entLenDIN2;
  refnode DAN;
  void *ptrDANi;
  Rint entLenDAN;
  Rlint downqualifying;
  IndexArray I1= allocM((*R1).IndArrLen);
  IndexArray I2= allocM((*R2).IndArrLen);
  Rint i, j;
  
  if (level1 != 0 && level2 != 0) {
    
    entLenDIN1= (*R1).DIR.entLen;
    entLenDIN2= (*R2).DIR.entLen;
    DIN1= (*R1).L[level1].N;
    DIN2= (*R2).L[level2].N;
    
    ptrDIN1i= DIN1;
    ptrDIN1i+= (*R1).Pent0;
    for (i= 0; i < (*DIN1).s.nofentries; i++) {
      I1[i]= DirOvlps(R1,ptrDIN1i,interRect);
      ptrDIN1i+= entLenDIN1;
    }
    ptrDIN2j= DIN2;
    ptrDIN2j+= (*R2).Pent0;
    for (j= 0; j < (*DIN2).s.nofentries; j++) {
      I2[j]= DirOvlps(R1,ptrDIN2j,interRect);
      ptrDIN2j+= entLenDIN2;
    }
    
    ptrDIN1i= DIN1;
    ptrDIN1i+= (*R1).Pent0;
    for (i= 0; i < (*DIN1).s.nofentries; i++) {
      if (I1[i]) {
        ptrDIN2j= DIN2;
        ptrDIN2j+= (*R2).Pent0;
        for (j= 0; j < (*DIN2).s.nofentries; j++) {
          if (I2[j]) {
            if (DirOvlps(R1,ptrDIN1i,ptrDIN2j)) {
              GetOvlpRect((*R1).numbofdim,ptrDIN1i,ptrDIN2j,interRect);
              ExtendPath(R1,i,level1);
              ExtendPath(R2,j,level2);
              SpJnCnt(R1,level1-1,
                      R2,level2-1,
                      interRect,
                      keysqualifying,cntlmt);
            }
          }
          ptrDIN2j+= entLenDIN2;
        }
      }
      ptrDIN1i+= entLenDIN1;
    }
  }
  else if (level1 == 0) {
    
    entLenDAN= (*R1).DATA.entLen;
    DAN= (*R1).L[level1].N;
    
    ptrDANi= DAN;
    ptrDANi+= (*R1).Pent0;
    for (i= 0; i < (*DAN).s.nofentries; i++) {
      if (DataOvlps(R1,ptrDANi,interRect)) {
        downqualifying= 0;
        SpJnRgnCnt(R2,R1,FALSE,level2,
                   ptrDANi,
                   &downqualifying);
        *keysqualifying+= downqualifying;
        if (VerboseJoinCount) {
          if (*keysqualifying > *cntlmt) {
            fprintf(stdout,strans("%s%10L%s\n",s),"More than",*cntlmt," record pairs.");
            if (*cntlmt == 0) {
              *cntlmt= 1;
            }
            else {
              *cntlmt*= 2;
            }
          }
        }
      }
      ptrDANi+= entLenDAN;
    }
  }
  else { /* level2 == 0 */
    
    entLenDAN= (*R2).DATA.entLen;
    DAN= (*R2).L[level2].N;
    
    ptrDANi= DAN;
    ptrDANi+= (*R2).Pent0;
    for (i= 0; i < (*DAN).s.nofentries; i++) {
      if (DataOvlps(R1,ptrDANi,interRect)) {
        downqualifying= 0;
        SpJnRgnCnt(R1,R2,TRUE,level1,
                   ptrDANi,
                   &downqualifying);
        *keysqualifying+= downqualifying;
        if (VerboseJoinCount) {
          if (*keysqualifying > *cntlmt) {
            fprintf(stdout,strans("%s%10L%s\n",s),"More than",*cntlmt," record pairs.");
            if (*cntlmt == 0) {
              *cntlmt= 1;
            }
            else {
              *cntlmt*= 2;
            }
          }
        }
      }
      ptrDANi+= entLenDAN;
    }
  }
  freeM(&I1);
  freeM(&I2);
}

/************************************************************************/
/* internal comparison counting in R1(SpJnCnt) only */

static void SpJnRgnCnt(RSTREE R, RSTREE Rx,
                       boolean order,
                       Rint level,
                       typinterval *rectOther,
                       Rlint *keysqualifying)

{
  refnode DIN;
  void *ptrDINi;
  Rint entLenDIN;
  refnode DAN;
  void *ptrDANi;
  Rint entLenDAN;
  Rint i;
  
  
  if (level != 0) {
  
    entLenDIN= (*R).DIR.entLen;
    DIN= (*R).L[level].N;
    ptrDINi= DIN;
    ptrDINi+= (*R).Pent0;

    if (order) {
      for (i= 0; i < (*DIN).s.nofentries; i++) {
        if (DirOvlps(R,ptrDINi,rectOther)) {
          ExtendPath(R,i,level);
          SpJnRgnCnt(R,Rx,order,level-1,
                     rectOther,
                     keysqualifying);
        }
        ptrDINi+= entLenDIN;
      }
    }
    else {
      for (i= 0; i < (*DIN).s.nofentries; i++) {
        if (DirOvlps(Rx,rectOther,ptrDINi)) {
          ExtendPath(R,i,level);
          SpJnRgnCnt(R,Rx,order,level-1,
                     rectOther,
                     keysqualifying);
        }
        ptrDINi+= entLenDIN;
      }
    }
  }
  else {
  
    entLenDAN= (*R).DATA.entLen;
    DAN= (*R).L[level].N;
    ptrDANi= DAN;
    ptrDANi+= (*R).Pent0;
    
    if (order) {
      for (i= 0; i < (*DAN).s.nofentries; i++) {
        if (DataOvlps(R,ptrDANi,rectOther)) {
          (*R).L[level].E= i;
          (*keysqualifying)++;
        }
        ptrDANi+= entLenDAN;
      }
    }
    else {
      for (i= 0; i < (*DAN).s.nofentries; i++) {
        if (DataOvlps(Rx,rectOther,ptrDANi)) {
          (*R).L[level].E= i;
          (*keysqualifying)++;
        }
        ptrDANi+= entLenDAN;
      }
    }
  }
}

/************************************************************************/
/* internal comparison counting in R1 only */

void SpJn(RSTREE R1,
          Rint level1,
          RSTREE R2,
          Rint level2,
          typinterval *interRect,
          void *mPtr,
          boolean *finish,
          JoinManageFunc Manage)

{
  refnode DIN1, DIN2;
  void *ptrDIN1i, *ptrDIN2j;
  Rint entLenDIN1, entLenDIN2;
  refnode DAN;
  void *ptrDANi;
  void *ptrDANinfo;
  Rint entLenDAN;
  IndexArray I1= allocM((*R1).IndArrLen);
  IndexArray I2= allocM((*R2).IndArrLen);
  Rint i, j;
    
  if (level1 != 0 && level2 != 0) {
    
    entLenDIN1= (*R1).DIR.entLen;
    entLenDIN2= (*R2).DIR.entLen;
    DIN1= (*R1).L[level1].N;
    DIN2= (*R2).L[level2].N;
    
    ptrDIN1i= DIN1;
    ptrDIN1i+= (*R1).Pent0;
    for (i= 0; i < (*DIN1).s.nofentries; i++) {
      I1[i]= DirOvlps(R1,ptrDIN1i,interRect);
      ptrDIN1i+= entLenDIN1;
    }
    ptrDIN2j= DIN2;
    ptrDIN2j+= (*R2).Pent0;
    for (j= 0; j < (*DIN2).s.nofentries; j++) {
      I2[j]= DirOvlps(R1,ptrDIN2j,interRect);
      ptrDIN2j+= entLenDIN2;
    }
    
    ptrDIN1i= DIN1;
    ptrDIN1i+= (*R1).Pent0;
    for (i= 0; i < (*DIN1).s.nofentries; i++) {
      if (*finish) {return;}
      if (I1[i]) {
        ptrDIN2j= DIN2;
        ptrDIN2j+= (*R2).Pent0;
        for (j= 0; j < (*DIN2).s.nofentries; j++) {
          if (*finish) {return;}
          if (I2[j]) {
            if (DirOvlps(R1,ptrDIN1i,ptrDIN2j)) {
              GetOvlpRect((*R1).numbofdim,ptrDIN1i,ptrDIN2j,interRect);
              ExtendPath(R1,i,level1);
              ExtendPath(R2,j,level2);
              SpJn(R1,level1-1,
                   R2,level2-1,
                   interRect,
                   mPtr,finish,Manage);
            }
          }
          ptrDIN2j+= entLenDIN2;
        }
      }
      ptrDIN1i+= entLenDIN1;
    }
  }
  else if (level1 == 0) {
      
    entLenDAN= (*R1).DATA.entLen;
    DAN= (*R1).L[level1].N;
    
    ptrDANi= DAN;
    ptrDANi+= (*R1).Pent0;
    ptrDANinfo= ptrDANi + (*R1).entPinfo;
    for (i= 0; i < (*DAN).s.nofentries; i++) {
      if (*finish) {return;}
      if (DataOvlps(R1,ptrDANi,interRect)) {
        SpJnRgnQuery(R2,R1,FALSE,level2,
                     ptrDANi,ptrDANinfo,
                     mPtr,finish,Manage);
      }
      ptrDANi+= entLenDAN;
      ptrDANinfo+= entLenDAN;
    }
  }
  else { /* level2 == 0 */
    
    entLenDAN= (*R2).DATA.entLen;
    DAN= (*R2).L[level2].N;
    
    ptrDANi= DAN;
    ptrDANi+= (*R2).Pent0;
    ptrDANinfo= ptrDANi + (*R2).entPinfo;
    for (i= 0; i < (*DAN).s.nofentries; i++) {
      if (*finish) {return;}
      if (DataOvlps(R1,ptrDANi,interRect)) {
        SpJnRgnQuery(R1,R2,TRUE,level1,
                     ptrDANi,ptrDANinfo,
                     mPtr,finish,Manage);
      }
      ptrDANi+= entLenDAN;
      ptrDANinfo+= entLenDAN;
    }
  }
  freeM(&I1);
  freeM(&I2);
}

/************************************************************************/
/* internal comparison counting in R1(SpJn) only */

static void SpJnRgnQuery(RSTREE R, RSTREE Rx,
                         boolean order,
                         Rint level,
                         typinterval *rectOther,
                         refinfo refInfoOther,
                         void *mPtr,
                         boolean *finish,
                         JoinManageFunc Manage)
{
  refnode DIN;
  void *ptrDIN;
  Rint entLenDIN;
  refnode DAN;
  void *ptrDANi;
  void *ptrDANinfo;
  Rint entLenDAN;
  Rint i;
  typinterval *rectR= allocM((*R).SIZErect);
  typinterval *rectRx= allocM((*Rx).SIZErect);
  refinfo infoR= allocM((*R).SIZEinfo);
  refinfo infoRx= allocM((*Rx).SIZEinfo);
  
  if (level != 0) {
    
    entLenDIN= (*R).DIR.entLen;
    DIN= (*R).L[level].N;
    ptrDIN= DIN;
    ptrDIN+= (*R).Pent0;

    if (order) {
      for (i= 0; i < (*DIN).s.nofentries; i++) {
        if (*finish) {return;}
        if (DirOvlps(R,ptrDIN,rectOther)) {
          ExtendPath(R,i,level);
          SpJnRgnQuery(R,Rx,order,level-1,
                       rectOther,refInfoOther,
                       mPtr,finish,Manage);
        }
        ptrDIN+= entLenDIN;
      }
    }
    else {
      for (i= 0; i < (*DIN).s.nofentries; i++) {
        if (*finish) {return;}
        if (DirOvlps(Rx,rectOther,ptrDIN)) {
          ExtendPath(R,i,level);
          SpJnRgnQuery(R,Rx,order,level-1,
                       rectOther,refInfoOther,
                       mPtr,finish,Manage);
        }
        ptrDIN+= entLenDIN;
      }
    }
  }
  else {
    
    entLenDAN= (*R).DATA.entLen;
    DAN= (*R).L[level].N;
    ptrDANi= DAN;
    ptrDANi+= (*R).Pent0;
    ptrDANinfo= ptrDANi + (*R).entPinfo;
    
    if (order) {
      for (i= 0; i < (*DAN).s.nofentries; i++) {
        if (*finish) {return;}
        if (DataOvlps(R,ptrDANi,rectOther)) {
          (*R).L[level].E= i;
          CopyRect((*R).numbofdim,ptrDANi,rectR);
          CopyRect((*Rx).numbofdim,rectOther,rectRx);
          memcpy(infoR,ptrDANinfo,(*R).SIZEinfo);
          memcpy(infoRx,refInfoOther,(*Rx).SIZEinfo);
          Manage((t_RT)R,(t_RT)Rx,(*R).numbofdim,rectR,rectRx,infoR,infoRx,(*R).SIZEinfo,(*Rx).SIZEinfo,mPtr,finish);
        }
        ptrDANi+= entLenDAN;
        ptrDANinfo+= entLenDAN;
      }
    }
    else {
      for (i= 0; i < (*DAN).s.nofentries; i++) {
        if (*finish) {return;}
        if (DataOvlps(Rx,rectOther,ptrDANi)) {
          (*R).L[level].E= i;
          CopyRect((*R).numbofdim,ptrDANi,rectR);
          CopyRect((*Rx).numbofdim,rectOther,rectRx);
          memcpy(infoR,ptrDANinfo,(*R).SIZEinfo);
          memcpy(infoRx,refInfoOther,(*Rx).SIZEinfo);
          Manage((t_RT)Rx,(t_RT)R,(*Rx).numbofdim,rectRx,rectR,infoRx,infoR,(*Rx).SIZEinfo,(*R).SIZEinfo,mPtr,finish);
        }
        ptrDANi+= entLenDAN;
        ptrDANinfo+= entLenDAN;
      }
    }
  }
  freeM(&rectR);
  freeM(&rectRx);
  freeM(&infoR);
  freeM(&infoRx);
}

/************************************************************************/
