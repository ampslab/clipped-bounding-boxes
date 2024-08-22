/* ----- RSTErrors.c ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//

#include <stdlib.h>
#include <stdio.h>

#include "RSTErrors.h"
#include "RSTStdTypes.h"


/* declarations */

static void pRSTmess(typerrors error);


/***********************************************************************/

void pRSTerr(const char *preMessage, typerrors error) {
  fprintf(stderr,"%s: ERROR: ",preMessage);
  pRSTmess(error);
}

/***********************************************************************/

void pRSTwarn(const char *preMessage, typerrors error) {
  fprintf(stderr,"%s: WARNING: ",preMessage);
  pRSTmess(error);
}

/***********************************************************************/

static void pRSTmess(typerrors error) {
  switch (error) {
/*** universal: ***/
    case noError:
      fprintf(stderr,"%s\n","no error");
      break;
    case reAlloc:
      fprintf(stderr,"%s\n","(re-)allocation failed");
      break;
    case redundant:
      fprintf(stderr,"%s\n","multiple redundant occurrence");
      break;
    case checkSum:
      fprintf(stderr,"%s\n","check sum error");
      break;
    case paramLss:
      fprintf(stderr,"%s\n","given parameter below minimum");
      break;
    case negativeEdge:
      fprintf(stderr,"%s\n","FATAL: data intervals [l, h] with h < l detected");
      break;
    case pgSizeMisMatch:
      fprintf(stderr,"%s\n","page size mismatch");
      break;
    case sizeMisMatch:
      fprintf(stderr,"%s\n","size mismatch");
      break;
    case idNULL:
      fprintf(stderr,"%s\n","invalid (NULL) identifier");
      break;
    case idNotNULL:
      fprintf(stderr,"%s\n","non initial identifier");
      break;
/*** special NULL, non NULL: ***/
    case RNULL:
      fprintf(stderr,"%s\n","invalid (NULL) t_RT identifier");
      break;
    case RnotNULL:
      fprintf(stderr,"%s\n","non initial t_RT identifier");
      break;
    case distQNULL:
      fprintf(stderr,"%s\n","invalid (NULL) t_DQ identifier");
      break;
    case distQnotNULL:
      fprintf(stderr,"%s\n","non initial t_DQ identifier");
      break;
      
/*** special under limit: ***/
    case dirMlss:
      fprintf(stderr,"%s\n","#entries per dir-page below 3");
      break;
    case dataMlss:
      fprintf(stderr,"%s\n","#entries per data-page below 1");
      break;
    case dirRAMsizeLss:
      fprintf(stderr,"%s\n","dir-RAM size too small");
      break;
    case dataRAMsizeLss:
      fprintf(stderr,"%s\n","data-RAM size too small");
      break;
    case nBufPagesLss:
      fprintf(stderr,"%s\n","number of buffer pages below minimum");
      break;
    case nDirBufPagesLss:
      fprintf(stderr,"%s\n","number of directory buffer pages below minimum");
      break;
    case nDataBufPagesLss:
      fprintf(stderr,"%s\n","number of data buffer pages below minimum");
      break;
    case numbDimLss:
      fprintf(stderr,"%s\n","number of dimensions below 1");
      break;
    case infoSizeLss:
      fprintf(stderr,"%s %d\n","information part size below",(int)sizeof(Rpnint));
      break;
      
/*** special over limit: ***/
    case dirMgtr:
      fprintf(stderr,"%s\n","#entries per dir-page exceeds maximum");
      break;
    case dataMgtr:
      fprintf(stderr,"%s\n","#entries per data-page exceeds maximum");
      break;
    case numbDimGtr:
      fprintf(stderr,"%s\n","number of dimensions exceeds maximum");
      break;
    case dirNumbPagesExh:
      fprintf(stderr,"%s\n","number of directory page numbers exhausted");
      break;
    case dataNumbPagesExh:
      fprintf(stderr,"%s\n","number of data page numbers exhausted");
      break;
    case nameLenGtr:
      fprintf(stderr,"%s\n","name string too long");
      break;
      
/*** special range: ***/
    case querySort_distType_range:
      fprintf(stderr,"%s\n","querySort or distType out of range");
      break;
      
/*** special alignment: */
    case dirPageSizeAlign:
      fprintf(stderr,"%s\n","dir-page size: bad alignment");
      break;
    case dataPageSizeAlign:
      fprintf(stderr,"%s\n","data-page size: bad alignment");
      break;
      
/*** special LRU: ***/
    case placement:
      fprintf(stderr,"%s\n","placement failed");
      break;
    case displacement:
      fprintf(stderr,"%s\n","displacement failed, too many locked entries");
      break;
    case multipleLocked:
      fprintf(stderr,"%s\n","entry multiple locked");
      break;
    case unexpectedWriteFlag:
      fprintf(stderr,"%s\n","unexpectedly entry to be written found");
      break;
      
/*** special inherited ***/
    case inquireDesc:
      fprintf(stderr,"%s\n","internal call of InquireRSTDesc failed");
      break;
    case getVarDesc:
      fprintf(stderr,"%s\n","internal call of GetVarRSTDesc failed");
      break;
    case LRUconsistenceCheck:
      fprintf(stderr,"%s\n","internal call of LRU consistence check failed");
      break;
      
/*** special mismatch: ***/
    case RdistQmismatch:
      fprintf(stderr,"%s\n","bad t_RT identifier, not registered for this query");
      break;
    case dirPageBufMismatch:
      fprintf(stderr,"%s\n","dir-page size does not match buffer page size");
      break;
    case dataPageBufMismatch:
      fprintf(stderr,"%s\n","data-page size does not match buffer page size");
      break;
    case treeDimMismatch:
      fprintf(stderr,"%s\n","trees of different dimensionality");
      break;
      
/*** special other: ***/
    case mainMemTree:
      fprintf(stderr,"%s\n","tree resides in main memory");
      break;
    case secMemTree:
      fprintf(stderr,"%s\n","tree resides in secondary memory");
      break;
    case notLRUbuffered:
      fprintf(stderr,"%s\n","tree is not LRU buffered");
      break;    
    case diffLRUbuffers:
      fprintf(stderr,"%s\n","trees with different LRU buffers");
      break;   
       
/*** last of enum marker: ***/
    case errorEnumLast:
      fprintf(stderr,"UNREGISTERED ERROR NUMBER %d\n",error);
      break;
      
    default:
      fprintf(stderr,"UNREGISTERED ERROR NUMBER %d\n",error);
      /* no strans() here: enum defaults to int */
  }
}

/***********************************************************************/

void PrintErrMessNumbs(void) {

  int e;
  
  fprintf(stderr,"+++  RSTree error messages and their numbers:\n");
  fprintf(stderr,"\n");
  for (e= 0; e <= errorEnumLast; e++) {
    fprintf(stderr,"%3d: ",e); 
    pRSTmess(e);
  }
}

/***********************************************************************/

