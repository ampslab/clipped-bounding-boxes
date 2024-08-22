/* MDrpr: see option -help */
#//
#// Copyright (c) 1994 - 2015 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
/* MDrpr: see option -help *** sort by absolute/relative nearest/farthest */
/* MDrpr.c: grep Param. */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "RSTOtherFuncs.h"
#include "RSTDistQueryFuncs.h"

/* constants */

#define tmpName "tmp_MDrpr"

/* types */


/* declarations */

void DistfileToRST(char *distname,
                   t_RT *rst,
                   t_LRU *LRU,
                   boolean *isRTonDisk);
Rpint GetLOF(char *path); /* GetLengthOfFile */
void SetGlobrectCenter(t_RT rst, typinterval *r, typcoord *p);
void SetCenter(typinterval *r, typcoord *p);
void PrintRectFormat(void);
void PrintGlobrectStart(typinterval *r, typcoord *p);
void PrintNumbRecords(Rpint numb, int remain);
void PrintDescription(t_RT rst);
boolean InfPartCmp(t_RT R,
                   const typinfo *stored,
                   Rint size,
                   const typinfo *searched,
                   void *ref);
boolean AlwaysTrue(t_RT R,
                   Rint numbDim,
                   const typinterval *RSTrect,
                   const typinterval *qRect,
                   Rint qRectQty,
                   void *qPtr);
void CalcEucl (Rfloat coordDist, Rfloat *cumuDist);
void ErrorExit(char *str);
void MakeAligned(Rint *numb, Rint alignm);

/* global variables: */

Rint rectSize= NumbOfDim * sizeof(typinterval);
Rint pointSize= NumbOfDim * sizeof(typcoord);
char s[160];


int main(int argc, char *argv[])

{
  Rpnint dummy;
  Rpint i, j, cntlmt;
  Rlint numbrecs;
  boolean deleted, exists;
  typinterval *globrect= malloc(rectSize);
  typcoord *center= malloc(pointSize);;
  t_RT rst;
  t_LRU LRU;
  boolean isRTonDisk;
  t_DQ distQ;
  /* distance query record: */
  typinterval *dqRect= malloc(rectSize);
  typinfo dqInfo;
  Rfloat dqRawDist;
  /* ---------------------- */
  DistQuerySort sort;
  FILE *outfile;
  
  if (argc == 2 && strcmp(argv[1],"-help") == 0) {
    printf(
"SYNOPSYS\n");
    printf("%s%s%s",
"     Usage: ",argv[0]," -help\n");
    printf("%s%s%s",
"            ",argv[0]," ( -n | -f ) inputfile outputfile\n");
    printf("\n");
    printf("%s%s",
argv[0]," works on files containing rectangles in the following format:\n");
    PrintRectFormat();
    printf(
"It reads all the rectangles from inputfile and writes them in modified\n");
    printf(
"succession to outputfile.\n");
    printf(
"Reading outputfile sequentially, the following holds in every position:\n");
    printf(
"Depending on the first argument -n(earest) / -f(arthest), the distance\n");
    printf(
"between the current rectangle and its successor is the MINIMUM / MAXIMUM\n");
    printf(
"regarding the remaining (following) rectangles of the file.\n");
    printf(
"The algorithm starts at the center of the distribution and proceeds as\n");
    printf(
"follows:\n");
    printf(
"(in principle! (it particularly does not really modify inputfile))\n");
    printf(
"set the starting point to the center of the distribution\n");
    printf(
"while inputfile is not exhausted\n");
    printf(
"  search for a rectangle in inputfile\n");
    printf(
"  write that rectangle to outputfile\n");
    printf(
"  remove that rectangle from inputfile\n");
    printf(
"  set the starting point to the center of that rectangle\n");
    printf(
"end.\n");
    exit(1);
  }
  else if (argc != 4 || strcmp(argv[1],"-n") != 0 && strcmp(argv[1],"-f") != 0) {
    printf("%s%s%s",
"Usage: ",argv[0]," -help\n");
    printf("%s%s%s",
"       ",argv[0]," ( -n | -f ) inputfile outputfile\n");
    exit(1);
  }
  else {
    InitRSTreeIdent(&rst);
    DistfileToRST(argv[2],&rst,&LRU,&isRTonDisk);
    
    /* --- move RSTree data sorted by proximity into the output file --- */
    if (! GetPagesRecords(rst,&dummy,&dummy,&numbrecs)) {
      ErrorExit("GetPagesRecords FAILED");
    }
    SetGlobrectCenter(rst,globrect,center);
    PrintGlobrectStart(globrect,center);
    outfile= fopen(argv[3],"wb");
    InitDistQueryIdent(&distQ);
    printf("Distance Queries, Deletions: WORKING ...\n");
    if (strcmp(argv[1],"-n") == 0) {
      sort= inc;
    }
    else {
      sort= dec;
    }
    i= 0; cntlmt= 1;
    for (j= 1; j <= numbrecs; j++) {
      if (! NewDistQuery(rst,center,CalcEucl,sort,minDist,0.0,NULL,0,NULL,AlwaysTrue,AlwaysTrue,2048,FALSE,&distQ)) {
        ErrorExit("NewDistQuery FAILED");
      }
      if (! GetDistQueryRec(distQ,rst,dqRect,&dqInfo,&dqRawDist,&exists)) {
        ErrorExit("GetDistQueryRec FAILED");
      }
      if (exists) {
        fwrite(dqRect,rectSize,1,outfile);
        SetCenter(dqRect,center);
        i++;
        if (i == cntlmt) {
          printf(strans("%P\n",s),i);
          cntlmt*= 2;
        }
      }
      DisposeDistQuery(&distQ);
      if (! DeleteRecord(rst,dqRect,NULL,InfPartCmp,NULL,&deleted)) {
        ErrorExit("DeleteRecord FAILED");
        if (! deleted) {
          printf("DeleteRecord: deletion FAILED\n");
        }
      }
    }
    PrintNumbRecords(i,0);
    if (i == numbrecs) {
      printf("Done.\n");
    }
    else {
      printf("FAILURE\n");
    }
    fclose(outfile);
    if (isRTonDisk) {
      //CloseRST(&rst); /* not needed: RT temp., result file written */
      DisposeLRU(&LRU);
      TryRemoveRST(tmpName);
    }
    else {
      //SaveLRU(LRU,&success); /* not needed: RT temp., result file written */
      RemoveMainMemRST(&rst);
    }
  }
  return 0;
}

/***********************************************************************/
/* typatomkey = Rfloat must hold! */

void DistfileToRST(char *distname,
                   t_RT *rst,
                   t_LRU *LRU,
                   boolean *isRTonDisk)

{
# define GIGA1 (1LL << 30) /* 1G */
# define MAX_MAIN_MEM_USE (12 * GIGA1) /* roughly(!) followed */
/* page size stepping: */
# define PG_GRNL_MAIN 16
# define PG_GRNL_SEC 4096
/* number of entries (main memory),*/
/* leading to moderate directory size and enough split range: */
# define N_ENT_MAIN 20
# define N_E_M_DATA_FAC 2.2
# define N_E_M_DIR_FAC 0.2
/* approximate number of entries (secondary memory), */
/* leading to sufficiently large pages: */
# define N_ENT_SEC 75
  Rpint lof, numbrecs, bytesrest, dirRAMsize, dataRAMsize,
        cntlmt, itemsread, i;
  Rint nomPageSize, pageGrnlMult, pageSize;
  Rpnint LRUcap;
  Rint sRF= sizeof(Rfloat);
  Rint infoSize= sizeof(typinfo);
  Rint entrySize= NumbOfDim * 2 * sRF + infoSize;
  boolean inserted, success;
  typinterval *distrect= malloc(rectSize);
  typinfo info;
  FILE *distfile;

  MakeAligned(&entrySize,sizeof(typatomkey));
  
  printf("%s%s%s","Storing ",distname," in an R-Tree:\n");
  distfile= fopen(distname,"rb");
  lof= GetLOF(distname);
  if (lof == 0) {
    perror(distname);
    exit(2);
  }
  numbrecs= lof / rectSize;
  bytesrest= lof % rectSize;
  PrintRectFormat();
  PrintNumbRecords(numbrecs,bytesrest);
  
  if (lof < MAX_MAIN_MEM_USE / N_E_M_DATA_FAC) {
    *isRTonDisk= FALSE;
    /* page size calculation: */
    nomPageSize= sRF + NumbOfDim * sRF + N_ENT_MAIN * entrySize;
    pageSize= nomPageSize;
    /* align upwards: */
    MakeAligned(&pageSize,PG_GRNL_MAIN);
    
    dirRAMsize= lof * N_E_M_DIR_FAC;
    if (dirRAMsize < 3 * pageSize) {
      dirRAMsize= 3 * pageSize;
    }
    dataRAMsize= lof * N_E_M_DATA_FAC;
    if (dataRAMsize < 3 * pageSize) {
      dataRAMsize= 3 * pageSize;
    }
    
    printf("RR*-tree.CreateMainMemRST: ");
    if (CreateMainMemRST(rst,pageSize,pageSize,
                         NumbOfDim,infoSize,FALSE,
                         dirRAMsize,dataRAMsize,TRUE)) {
      printf("Done.\n");
    }
    else {
      ErrorExit("FAILURE");
    }
  }
  else {
    *isRTonDisk= TRUE;
    /* page size calculation: */
    nomPageSize= sRF + NumbOfDim * sRF + N_ENT_SEC * entrySize;
    /* align rounding, preserving one page of size PG_GRNL_SEC: */
    pageGrnlMult= (int)((double)nomPageSize / (double)PG_GRNL_SEC + 0.5);
    if (pageGrnlMult == 0) {
      pageGrnlMult= 1;
    }
    pageSize= pageGrnlMult * PG_GRNL_SEC;
    
    /* remove possibly remaining old temporary tree: */
    TryRemoveRST(tmpName);
    /* then create temporary new one: */
    printf("RR*-tree.CreateRST: %s ",tmpName);
    if (CreateRST(tmpName,pageSize,pageSize,NumbOfDim,infoSize,FALSE)) {
      printf("Done.\n");
    }
    else{
      ErrorExit("FAILURE");
    }
    /* LRU buffer capacity calculation in pages: */
    LRUcap= MAX_MAIN_MEM_USE / pageSize;
    
    printf("LRU.NewLRU: ");
    success= TRUE;
    NewLRU(LRU,LRUcap,pageSize,&success);
    if (success) {
      printf("Done.\n");
    }
    else{
      ErrorExit("FAILURE");
    }
    printf("RR*-tree.OpenBufferedRST: %s ",tmpName);
    if (OpenBufferedRST(rst,tmpName,*LRU)) {
      printf("Done.\n");
    }
    else{
      ErrorExit("FAILURE");
    }
  }
  
  printf("Insertion: WORKING ...\n");
  cntlmt= 1;
  for (i= 1; i <= numbrecs; i++) {
    itemsread= fread(distrect,rectSize,1,distfile);
    if (itemsread < 1) {
      ErrorExit("distfile read FAILED");
    }
    info.tag= i;
    if (! InsertRecord(*rst,distrect,&info,&inserted,&info)) {
      ErrorExit("InsertRecord FAILED");
    }
    if (i == cntlmt) {
      printf(strans("%P\n",s),i);
      cntlmt*= 2;
    }
  }
  printf(strans("%P Done.\n",s),numbrecs);
  PrintDescription(*rst);
  fclose(distfile);
}

/***********************************************************************/

Rpint GetLOF(char *path) /* GetLengthOfFile */

{
  struct stat status;
  int ferr;
  
  ferr= stat(path,&status);
  if (ferr == -1) {
    perror("GetLOF");
    return 0;
  }
  else {
    return status.st_size;
  }
}

/***********************************************************************/

void SetGlobrectCenter(t_RT rst, typinterval *r, typcoord *p)

{
  int d;
  Rint dummy;
  
  if (GetGlobalMBB(rst,&dummy,r)) {
    for (d= 0; d < NumbOfDim; d++) {
      p[d]= (r[d].l + r[d].h) * 0.5;
    }
  }
  else {
    ErrorExit("GetGlobalMBB FAILED");
  }
}

/***********************************************************************/

void PrintRectFormat()

{
  if (sizeof(typatomkey) == sizeof(double)) {
    printf("rectangle: struct{ DOUBLE l, h } ");
  }
  else if (sizeof(typatomkey) == sizeof(float)) {
    printf("rectangle: struct{ FLOAT l, h } ");
  }
  else {
    printf("rectangle: struct{ UNKNOWN(!?) l, h } ");
  }
  printf("%s%d%s\n","[",NumbOfDim,"].");
}

/***********************************************************************/

void PrintGlobrectStart(typinterval *r, typcoord *p)

{
  int d;
  
  printf("global rectangle(");
  for (d= 0; d < NumbOfDim; d++) {
    printf("%g %g",r[d].l,r[d].h);
    if (d < NumbOfDim - 1) {
      printf("  ");
    }
    else {
      printf(").\n");
    }
  }
  printf("starting point(");
  for (d= 0; d < NumbOfDim; d++) {
    printf("%g",p[d]);
    if (d < NumbOfDim - 1) {
      printf("  ");
    }
    else {
      printf(").\n");
    }
  }
}

/***********************************************************************/

void PrintNumbRecords(Rpint numb, int remain)

{
  printf(strans("%s%P\n",s),"Number of records: ",numb);
  if (remain != 0) {
    printf("%s%d%s\n",
           "ERROR: incomplete record of size ",remain," at the end!");
    exit(2);
  }
}

/***********************************************************************/

void PrintDescription(t_RT rst)

{
  Rpnint numbofdirpages, numbofdatapages;
  Rlint numbofrecords;

  if (! GetPagesRecords(rst,&numbofdirpages,&numbofdatapages,&numbofrecords)) {
    ErrorExit("GetPagesRecords FAILED");
  }
  printf(strans("%s%L\n",s),"#records in RSTree: ",numbofrecords);
}

/***********************************************************************/

boolean InfPartCmp(t_RT R,
                   const typinfo *stored,
                   Rint size,
                   const typinfo *searched,
                   void *ref) {

  /* we do not want to examine info parts */
  return TRUE;
}

/***********************************************************************/

boolean AlwaysTrue(t_RT R,
                   Rint numbDim,
                   const typinterval *RSTrect,
                   const typinterval *qRect,
                   Rint qRectQty,
                   void *qPtr)

{
  return TRUE;
}

/***********************************************************************/

void CalcEucl (Rfloat coordDist, Rfloat *cumuDist)

{
  *cumuDist+= coordDist * coordDist;
}

/***********************************************************************/

void ErrorExit(char *str)

{
  printf("%s\n",str);
  exit(2);
}

/***********************************************************************/

void SetCenter(typinterval *r, typcoord *p)

{
  int d;
  
  for (d= 0; d < NumbOfDim; d++) {
    p[d]= (r[d].l + r[d].h) * 0.5;
  }
}

/***********************************************************************/

void MakeAligned(Rint *numb, Rint alignm) {

  Rint ill= *numb & (alignm - 1);
  if (ill != 0) {
    (*numb)+= alignm - ill;
  }
}

/************************************************************************/
