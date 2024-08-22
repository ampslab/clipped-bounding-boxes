/* -----  RSTErrors.h  ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTErrors_h
#define __RSTErrors_h


/**:   Error symbols and a function to print error messages
       ====================================================             **/


/**    Implementation:  Norbert Beckmann
              Version:  3.3
                 Date:  07/14                                           **/

/** Properties:
    See functions pRSTerr() and pRSTwarn() below.
    See error message explication in RSTErrors.c.
    Level: bottom.
    Higher levels: RSTUtil.h                                            **/


/* ----- types ----- */

typedef enum {
/*** universal: ***/
  noError,
  reAlloc,
  redundant,
  checkSum,
  paramLss,
  negativeEdge,
  pgSizeMisMatch,
  sizeMisMatch,
  idNULL,
  idNotNULL,
  
/*** special NULL, non NULL: ***/
  RNULL,
  RnotNULL,
  distQNULL,
  distQnotNULL,
  
/*** special under limit: ***/
  dirMlss,
  dataMlss,
  dirRAMsizeLss,
  dataRAMsizeLss,
  nBufPagesLss,
  nDirBufPagesLss,
  nDataBufPagesLss,
  numbDimLss,
  infoSizeLss,
  
/*** special over limit: ***/
  dirMgtr,
  dataMgtr,
  numbDimGtr,
  dirNumbPagesExh,
  dataNumbPagesExh,
  nameLenGtr,
  
/*** special range: ***/
  querySort_distType_range,
  
/*** special alignment: ***/
  dirPageSizeAlign,
  dataPageSizeAlign,
  
/*** special LRU: ***/
  placement,
  displacement,
  multipleLocked,
  unexpectedWriteFlag,
  
/*** special inherited ***/
  inquireDesc,
  getVarDesc,
  LRUconsistenceCheck,
  
/*** special mismatch: ***/
  RdistQmismatch,
  dirPageBufMismatch,
  dataPageBufMismatch,
  treeDimMismatch,
  
/*** special other: ***/
  mainMemTree,
  secMemTree,
  notLRUbuffered,
  diffLRUbuffers,
  
/*** last of enum marker: ***/
  errorEnumLast /* no error assigned */
} typerrors;


/* ----- functions ----- */

void pRSTerr(const char *preMessage, typerrors error);
void pRSTwarn(const char *preMessage, typerrors error);

/* pRSTerr and pRSTwarn are similar to the C library function perror
   (see man perror).
   They produce a message on stderr, describing the error/warning passed by
   the parameter error.
   The complete message is formatted as follows:
   
   pRSTerr():   <preMessage>: ERROR: <error message>
   pRSTwarn():  <preMessage>: WARNING: <error message>

   where <preMessage> is a string passed by the parameter preMessage and
   <error message> is a string corresponding to the parameter error.
   
   preMessage:
   string built into the error message string.
   
   error:
   number (symbol) of the error to be described. */


void PrintErrMessNumbs(void);

/* For each error, prints the error number, followed by the corresponding
   message to *stderr*. */


#endif /* __RSTErrors_h */
