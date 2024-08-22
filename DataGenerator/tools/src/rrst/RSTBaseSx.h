/* -----  RSTBase.h  ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTBase_h
#define __RSTBase_h


/**:   (Private) Basic definitions
       ===========================                                      **/


/**    Implementation:  Norbert Beckmann
              Version:  8.5
                 Date:  06/14                                           **/

/**    Level: bottom.
                                                                        **/


#include "RSTLRUBuf.h"
#include "RSTTypes.h"


/*** Begin  --- naming rules for structured types (laxly followed) ----  ***/
/*
Non array/vector types are called "typ<..>",
  Exceptions:
    Side, rstree, ..
References to non array/vector types are called "ref<..>".
  Exceptions:
    RSTREE, ..
Array/Vector types:
Array/Vector types over [ENTRY_RANGE] are called <..>Array.
Array/Vector types over [PATH_RANGE] are called <..>Path.
Array/Vector types over [SPLIT_OVER_ALL_RANGE] are called <..>AllArray.
Array/Vector types for special purposes:
    RSTName, typfixblock, ..
*/
/***   End  --- naming rules for structured types ----  ***/


/* types needed by constants */

typedef Rlint  typword; /* longest possible one processor access integer */


/* constants */

#define NODE_VERSION_RRST

#define SIZEatomkey ((Rint) sizeof(typatomkey))
#define SIZEinterval ((Rint) sizeof(typinterval))
#define SIZEcoord ((Rint) sizeof(typcoord))
#define SIZERpnint ((Rint) sizeof(Rpnint))
#define SIZEptrtosub SIZERpnint

#define SIZE_FIX_BL 4096
/* SIZE_FIX_BL should be big enough to cover typparameters, mind MAX_DIM */
#define MAX_PG_NRS ((Rint) (SIZE_FIX_BL / sizeof(Rpnint) - 3))
#define PATH_RANGE 42
/* PATH_RANGE limits height of tree to 42; should be sufficient as e.g.
   M=5 --> 3^42 ~= 1.09e20 entries, M=100 --> 67^42 ~= 4.96e76 entries. */
#define MAX_DIM 500
/* for 1D-key R-tree MAX_DIM depends on SIZE_FIX_BL, see also typparameters */
/* here arbitrary */

#define DATA_SUFF ".Data"
#define DIR_PD_SUFF ".DirPD"
#define DATA_PD_SUFF ".DataPD"
#define DESC_SUFF ".Desc"
/* ---  suffixes changed?: adapt SUFF_LEN too!!  --- */
#define SUFF_LEN 8

#define MAIN_MEMORY "MAIN_MEMORY"

#define PARAM_BL_NR 0
#define FIRST_PD_BL_NR (PARAM_BL_NR + 1)
#define ROOT_BL_NR 0
#define EMPTY_BL ((Rpnint) -1)

/* maximum number of data pages (directory pages are irrelevant then): */
#define MAX_PAGES_SECU_RETENT 100 /* max pages security retention */
#define MAX_DATA_PAGES (EMPTY_BL - 1 - MAX_PAGES_SECU_RETENT)


/* types */

typedef union {
              typword  words[1];
              byte     _[1];
              } uWordsBytes;

typedef typatomkey  *refatomkey;
typedef typinterval  *refinterval;
typedef typcoord *refcoord;
typedef Rpnint  *refRpnint, *refptrtosub;

typedef Rfloat       *ValueArray;		/* flexible */
typedef Rint         *IndexArray;		/* flexible */
typedef typinterval  *IntervalArray;	/* serves for rects!	flexible */
typedef typcoord     *CoordArray;	/* serves for points!	flexible */

typedef Rfloat       *ValueAllArray;		/* flexible */
typedef Rint         *IndexAllArray;		/* flexible */

typedef enum {
             low,
             high
             } Side;

typedef enum {
             pri,
             lru,
             sec
             } StorageKind;

/************** Begin: entry scheme ***********************/
//typedef union {
//              Rpnint   ptrtosub;
//              typinfo  info;			/* flexible */
//              } typentryrest, *refentryrest;
//
//typedef struct {
//               typinterval   rect[];		/* flexible */
//               typentryrest  u;		/* flexible */
//               } typentry;			/* universal entry */
/**************   End: entry scheme ***********************/

typedef struct {
               Rshort    nofentries;
               Rshort    splitnoe;
               Rshort    makeSize6;		/* alignment filler, size > 4 */
               typcoord  splitmid[];		/* flexible */
               } typnodeinfo, *refnodeinfo;	/* aligned for typcoord (!) */

typedef struct {
               typnodeinfo  s;
          /*** typentry     entries[];		flexible ***/
               } /* typnode, */ *refnode;	/* universal node */

typedef struct {
               refnode  N;			/* node */
               Rpnint   P;			/* page number */
               Rint     E;			/* entry number */
               boolean  Modif;			/* modified */
               boolean  ReInsert;		/* flag */
               } typlevel, *reflevel;

typedef struct {
               Rpnint  P;
               Rint    E;
               } typpagent, *refpagent;

typedef typlevel   Path[PATH_RANGE];		/* level: [0 .. rootlvl] */
typedef typpagent  PagEntPath[PATH_RANGE];	/* level: [0 .. rootlvl] */
typedef Rint       RintPath[PATH_RANGE];	/* level: [0 .. rootlvl] */
typedef Rpnint     RpnintPath[PATH_RANGE];	/* level: [0 .. rootlvl] */
typedef RSTName    NamePath[PATH_RANGE];	/* level: [0 .. rootlvl] */
typedef File       FilePath[PATH_RANGE];	/* level: [0 .. rootlvl] */
typedef void*      PtrPath[PATH_RANGE];		/* level: [0 .. rootlvl] */

typedef byte  typfixblock[SIZE_FIX_BL];

typedef struct {
               Rint    entriesoffset;
               Rint    entrylen;
               Rint    pagelen;
               Rint    M, EqvWords_Mp1, EqvWords_Dummy;
               Rpnint  pagecount;
               } typddparams, *refddparams;

/* CAUTION concerning typparameters:
   Types with sizes, greater than sizeof(int), have to start properly aligned,
   to ensure read-in compatibility between 32 bit and 64 bit compilation.
   The current design of this struct considers pure int structs as if they
   were sequences of single int elements and starts greater units at even
   sizeof(int) positions.
   - The design may have to be modified platform dependent! - */

typedef struct {
               typddparams  DIR, DATA;
               boolean      unique;
               Rint         numbofdim;
               Rint         rootlvl;
               Rint         SIZE_ptrtosub, SIZEinfo;
               Rint         makeEvenNumbRints;	/* alignment filler */
               RpnintPath   PageCount;		/* PATH_RANGE is even (42) */
               Rlint        recordcount;        /* 256 bytes up to here */
               } typparameters, *refparameters;

typedef union {
              typparameters  _;			/* call the intrinsics "_" */
              typfixblock    fixblock;		/* adjust to SIZE_FIX_BL */
              } typparamblock, *refparamblock;

typedef struct {
               Rint  reinsertqty;
               Rint  m, DELm;
               } typddvers, *refddvers;

typedef struct {
               typddvers  DIR, DATA;
               Rint       reinstpercent;
               Rint       minfillpercent, DELminfillpercent;
               } typversion, *refversion;

typedef union {
              typversion   _;			/* call the intrinsics "_" */
              typfixblock  fixblock;		/* adjust to SIZE_FIX_BL */
              } typversblock, *refversblock;

typedef struct {
               Rpnint  childnr;
               Rpnint  nofnumbers;		/* anyway Rpnint aligned! */
               Rpnint  number[MAX_PG_NRS + 1];	/* 0 + [1 .. MAX_PG_NRS] */
               } typpagedir, *refpagedir;

typedef union {
              typpagedir   _;			/* call the intrinsics "_" */
              typfixblock  fixblock;		/* adjust to SIZE_FIX_BL */
              } typPDblock, *refPDblock;

typedef struct {
               void     *ptr;
               Rpint    size;
               Rpnint   pagelimit;
               boolean  locked;
               } typRAMdesc, *refRAMdesc;

typedef struct {
               Rlint read, write;
               } typRWcnt, *refRWcnt;

typedef struct {
               File        f;
               Rint        psize;
               typRAMdesc  RAM;
               typRWcnt    cnt;
               } typstorage, *refstorage;

typedef struct {
               typPDblock  pagedir;
               typstorage  str;
               } typpageadmin, *refpageadmin;

typedef struct {
               Rlint demand;
               Rlint get, put;
               Rlint overflow, underflow;
               Rlint reinst;
               Rlint split;
               Rlint S_Area0;
               Rlint comp;
               } typddcnt, *refddcnt;

typedef struct { 
               typddcnt  DIR, DATA;
               boolean   on;
               Rlint     CS_Call;
               Rlint     CS_NoFit;
               Rlint     CS_UniFit;
               Rlint     CS_SomeFit;
               Rlint     CS_OvlpEnlOpt;
               Rlint     CS_P;
               Rlint     CS_MaxP;
               Rlint     CS_PminusQ;
               Rlint     CS_OvlpEnlComput;
               Rlint     CS_P1OvlpEnl0;
               Rlint     CS_AfterwOvlpEnl0;
               Rlint     CS_Area0;
               Rint      DQ_PQlen; /* independent of the Counts-Switch */
               Rint      DQ_PQmax; /* independent of the Counts-Switch */
               Rlint     DQ_PQelems;
               } typcount, *refcount;

typedef struct {
               Rint          entLen;
               Rint          nodeLen;
               typpageadmin  PA;
               typstorage    str;
               } typdd, *refdd;

typedef struct {
               Rint           numbofdim;	/* */
               Rint           SIZEpoint;	/* */
               Rint           SIZErect;		/* */
               /***           SIZEptts		= const SIZEptrtosub! ***/
               Rint           SIZEinfo;		/* */
               Rint           pttsLen;		/* size(ptrtosub) */
               Rint           infoLen;		/* size(info) */
               Rint           Pent0;		/* adr(entry[0]) (page) */
               /***           Prect0		= Pent0! ***/
               Rint           entPptts;		/* adr(ptrtosub) (entry) */
               Rint           entPinfo;		/* adr(info) (entry) */
               Rint           DHELen;		/* size(Dist Heap Element) */
               Rint           DHEPent0;		/* adr(entry[0]) (DHE) */
               Rint           UNInodeLen;	/* size(universal page) */
               Rint           ValArrLen;	/* */
               Rint           IndArrLen;	/* */
               Rint           BytArrLen;	/* */
               Rint           RectArrLen;	/* */
               Rint           PointArrLen;	/* */
               Rint           ValAllArrLen;	/* */
               Rint           IndAllArrLen;	/* */
               typdd          DIR, DATA;
               refnode        helpnode;
               Path           L, LDel, L1;
               PagEntPath     LRrg;
               t_LRU          LRU;
               typparamblock  parameters;
               typversblock   version;
               typcount       count;
               StorageKind    storkind;
               boolean        buflinked;
               boolean        secbound;
               boolean        RSTDone;
               boolean        verboseRAMdiskExt;
               Rint           DIReqvWords_Mp1;
               Rint           DATAeqvWords_Mp1;
               Rint           error;
               RSTName        mainname;
               /*------------ Re-usable pre-allocated entities ... */
               /* ----------- in RSTInstDel, RSTChooseSub: */
               typinterval    *rootMBB;
               IndexArray     GIP_use0;
               typinterval    *I_instRect;
               IntervalArray  CSCM_EnlRects;
               ValueArray     CSCM_EdgeEnls;
               ValueArray     CSCM_OvlpEnls;
               uWordsBytes    *CSCM_X;
               refnode        S_NSibl;
               refnode        S_NSplit;
               typinterval    *S_allrect;
               IndexArray     SADDD_I;
               IndexAllArray  SADDD_IA;
               ValueAllArray  SADDD_Evals;
               ValueAllArray  SADDD_EOvals;
               ValueArray     DDPS_edgearray;
               IntervalArray  DDPS_rectarray;
               typinterval    *DDPS_leftrect;
               typinterval    *DDPS_rightrect;
               refnode        M_NMerge;
               IndexArray     M_use0;
               typinterval    *M_reducedrect;
               refnode        SM_NSibl;
               typinterval    *SM_allrect;
               /* ----------- in RSTree: */
               void           *IR_entry;
               /* ----------- in RSTQuery: */
               typinterval    *RQA_rect;
               refinfo        RQA_info;
               } rstree;

typedef rstree  *RSTREE; /* tree identifier */

/* begin -------------- DistHeap -------------------- */
/* ----- types ----- */

typedef struct {
               Rint      level;
               Rfloat    dist;
               } typDHEinfo, *refDHEinfo;
               
typedef struct {
               typDHEinfo s;
          /*** typentry  entry;			flexible ***/
               } /* DistHeapElem */ *refDHelem, *DistHeapArray;

typedef Rint DistHeapIndElem, *DistHeapIndArr;

typedef struct {
               Rint            elemLen;
               Rint            max;
               Rfloat          extFac;
               boolean         verbose;
               DistHeapArray   arr;
               DistHeapIndArr  I;
               Rint            qty;
               } DistHeap;

/* end   -------------- DistHeap -------------------- */


/* constants derived from changeable types
   (by use of curiously named necessarily global variables) */

typnodeinfo __oN0No__;
#define  MAX_ENTRIES ((Ruint) (1 << (8 * sizeof(__oN0No__.nofentries) - 1)) - 1)


/* declarations */

void SetOffsets(RSTREE R, Rint numbOfDim, Rint infoSize);


#endif /* __RSTBase_h */
