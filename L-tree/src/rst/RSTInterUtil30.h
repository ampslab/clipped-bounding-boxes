/* -----  RSTInterUtil.h  ----- */
#//
#// Copyright (c) 1994 - 2015 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTInterUtil_h
#define __RSTInterUtil_h


/**:   Utility routines needed by the main Interface
       =============================================                    **/


/**    Implementation:  Norbert Beckmann
              Version:  3.1
                 Date:  01/10                                           **/

/**    Level: intermediate.
                                                                        **/

#include "RSTOtherFuncs.h"
#include "RSTDistQueryFuncs.h"
#include "RSTBase.h"


/* constants */

#define DIR_MIN_ENTRY_QTY  2
/* R-trees require a DIRECTORY node to contain at least 2 entries.
   The constant above defines the effective minimum for the case that
   MIN_FILL_PERCENT (defined below) leads to a smaller value. */

#define DATA_MIN_ENTRY_QTY 1
/* R-trees require a DATA node to contain at least 1 entry.
   The constant above defines the effective minimum for the case that
   MIN_FILL_PERCENT (defined below) leads to a smaller value. */

#define MIN_FILL_PERCENT 30
#define DEL_MIN_FILL_PERCENT 50
#define REINSERT_PERCENT 30


/* declarations */

void CreateRSFiles(RSTREE R);
void OpenRSFiles(RSTREE R);
void FastCloseRSFiles(RSTREE R);
void CloseRSFiles(RSTREE R);
void UpdateRSTDescFile(RSTREE R);
boolean ReadRSTDescFile(const char *name);
void InitRamDisc(RSTREE R);
void ReAllocRamDisc(RSTREE R, Rpint dirRAMsize, Rpint dataRAMsize);
void DeallocRamDisc(RSTREE R);
void SetRAMdiscLimits(RSTREE R);
void HandleRamDiscLock(RSTREE R);
void RemoveMMRSTIdentity(RSTREE *r);
boolean CreateMMRSTIdentity(RSTREE *r, RSTREE sourceR);
void SetBase(RSTREE R, Rint dirpagesize, Rint datapagesize, boolean unique);
void SetVersion(RSTREE R);
void SetCheck(RSTREE R, boolean creation);
void InitBuffersFlags(RSTREE R);
void AllocBuffers(RSTREE R);
void DeallocBuffers(RSTREE R);
void WriteParamsPDs(RSTREE R);
void ReadParamsPDs(RSTREE R);
void WriteParamsPDsPure(RSTREE R);
void ReadParamsPDsPure(RSTREE R);
void SetFixParamIOblocks(RSTREE R);
void SetVarDirDataIOblocks(RSTREE R);
void SyncPath(RSTREE R);
void PutPath(RSTREE R);
void InitCount(RSTREE R);
void BasicCheck(void);
boolean InternalOpen(RSTREE *r, const char *name, boolean allocBuffers);
void ResetStorageInfo(RSTREE R, StorageKind storage);
boolean LRUSatisfies(t_LRU LRU, Rpnint need);
boolean OpenBufRSTIdentity(RSTREE *r, RSTREE sourceR);
boolean CloseBufRSTIdentity(RSTREE *r);
boolean CloseRSTIdentity(RSTREE *r);
void JoinJoinCounts(RSTREE mainR, RSTREE secR);
void InitRootAndCounting(RSTREE R, boolean rootExists);
void CleanupCloseRST(RSTREE *r);
boolean CloseLRUBuffering(RSTREE R);
void InitFlags(RSTREE R);
void AdaptPathAlloc(RSTREE R, Rint OLDrootlvl);


#endif /* __RSTInterUtil_h */
