/* MDmpr: see option -help */
#//
#// Copyright (c) 1994 - 2015 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
/* MDmpr: see option -help *** sort by absolute/relative nearest/farthest 
                               relative to multiple starting points */
/* MDmpr.c: grep Param. */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "RSTOtherFuncs.h"
#include "RSTDistQueryFuncs.h"
#include "drand48.h"
/* for drand48 on platforms not providing it (Windows); linking: requires
   libPltfHelp or libUTIL. */

/* constants */

#define RANDSEED 8819 /* <--- set Param. */
#define tmpName "tmp_MDmpr"


/* types */


/* declarations */

void DistfileToRST(char *distname,
                   t_RT *rst,
                   t_LRU *LRU,
                   boolean *isRTonDisk);
Rpint GetLOF(char *path); /* GetLengthOfFile */
void SetGlobrectOriginDists(t_RT rst,
                            typinterval *r,
                            typcoord *orig,
                            typcoord *dists);
void SetRandPoint(typcoord *orig, typcoord *dists, typcoord *p);
void PrintRectFormat(void);
void PrintGlobrect(typinterval *r);
void PrintNumbRecords(Rpint numb, int remain);
void PrintDescription(t_RT rst);
boolean AlwaysTrue(t_RT R,
                   Rint numbDim,
                   const typinterval *RSTrect,
                   const typinterval *qRect,
                   Rint qRectQty,
                   void *qPtr);
boolean InfPartCmp(t_RT R,
                   const typinfo *stored,
                   Rint size,
                   const typinfo *searched,
                   void *ref);
void CalcEucl (Rfloat coordDist, Rfloat *cumuDist);
void ErrorExit(char *str);
void SetCenter(typinterval *r, typcoord *p);
void MakeAligned(Rint *numb, Rint alignm);

/* global variables: */

Rint rectSize= NumbOfDim * sizeof(typinterval);
Rint pointSize= NumbOfDim * sizeof(typcoord);
char s[160];


int main(int argc, char *argv[])

{
  Rpnint dummy;
  Rpint i, j, newStartInd, maxstarts, cntlmt;
  Rlint numbrecs;
  boolean deleted, exists, relative;
  typinterval *globrect= malloc(rectSize);
  typcoord *orig= malloc(pointSize);
  typcoord *dists= malloc(pointSize);
  typcoord *start;
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
"            ",argv[0]," ( -a | -r ) ( -n | -f ) inputfile outputfile\n");
    printf("\n");
    printf("%s%s",
argv[0]," works on files containing rectangles in the following format:\n");
    PrintRectFormat();
    printf(
"It reads all the rectangles from inputfile and writes them in modified\n");
    printf(
"succession to outputfile.\n");
    printf("%s%s%s",
"The succession ",argv[0]," creates is derived from what MDapr<n> and\n");
    printf(
"MDrpr<n> respectively create. See MDapr<n> -help and MDrpr<n> -help.\n");
    printf("%s%s%s",
"The difference is, that ",argv[0]," starts from\n");
    printf(
"sqrt(2 * (number of rectangles)) starting points, which are created\n");
    printf(
"sequencially.\n");
    printf(
"Explanation of the first two arguments:\n");
    printf(
"First argument:\n");
    printf(
"   -a: Use absolute distances as in MDapr<n>\n");
    printf(
"   -r: Use relative distances as in MDrpr<n>.\n");
    printf(
"Second argument:\n");
    printf(
"As in MDapr<n> / MDrpr<n>: The successor in every sitiuation is:\n");
    printf(
"   -n: the nearest rectangle\n");
    printf(
"   -f: the farthest rectangle.\n");
    printf(
"The algorithm proceeds as follows:\n");
    printf(
"(in principle! (it particularly does not really modify inputfile))\n");
    printf(
"k = 1\n");
    printf(
"repeat\n");
    printf(
"  randomly create the k-th starting point\n");
    printf(
"  foreach starting point\n");
    printf(
"    if inputfile is not yet exhausted\n");
    printf(
"      search for a rectangle in inputfile\n");
    printf(
"      write that rectangle to outputfile\n");
    printf(
"      remove that rectangle from inputfile\n");
    printf(
"      if the first argument is -r\n");
    printf(
"        set the current starting point to the center of that rectangle\n");
    printf(
"      end\n");
    printf(
"    end\n");
    printf(
"  end\n");
    printf(
"  increment k\n");
    printf(
"until inputfile is exhausted.\n");
    exit(1);
  }
  else if (argc != 5 || strcmp(argv[1],"-a") != 0 && strcmp(argv[1],"-r") != 0 && strcmp(argv[2],"-n") != 0 && strcmp(argv[2],"-f")) {
    printf("%s%s%s",
"Usage: ",argv[0]," -help\n");
    printf("%s%s%s",
"       ",argv[0]," ( -a | -r ) ( -n | -f ) inputfile outputfile\n");
    exit(1);
  }
  else {
    // srand48(RANDSEED); /* inadvertantly not set originally! */
    InitRSTreeIdent(&rst);
    DistfileToRST(argv[3],&rst,&LRU,&isRTonDisk);
    
    /* --- move RSTree data sorted by proximity into the output file --- */
    if (! GetPagesRecords(rst,&dummy,&dummy,&numbrecs)) {
      ErrorExit("GetPagesRecords FAILED");
    }
    SetGlobrectOriginDists(rst,globrect,orig,dists);
    PrintGlobrect(globrect);
    maxstarts= sqrt(2 * numbrecs) + 0.5;
    start= malloc(maxstarts * pointSize);
    outfile= fopen(argv[4],"wb");
    InitDistQueryIdent(&distQ);
    printf("Distance Queries, Deletions: WORKING ...\n");
    relative= strcmp(argv[1],"-r") == 0;
    if (strcmp(argv[2],"-n") == 0) {
      sort= inc;
    }
    else {
      sort= dec;
    }
    i= 0; newStartInd= 0; cntlmt= 1;
    while (i < numbrecs) {
      SetRandPoint(orig,dists,&start[NumbOfDim * newStartInd]);
      newStartInd++;
      j= 0;
      while (j < newStartInd && i < numbrecs) {
        if (! NewDistQuery(rst,&start[NumbOfDim * j],CalcEucl,sort,minDist,0.0,NULL,0,NULL,AlwaysTrue,AlwaysTrue,2048,FALSE,&distQ)) {
          ErrorExit("NewDistQuery FAILED");
        }
        if (! GetDistQueryRec(distQ,rst,dqRect,&dqInfo,&dqRawDist,&exists)) {
          ErrorExit("GetDistQueryRec FAILED");
        }
        if (exists) {
          fwrite(dqRect,rectSize,1,outfile);
          if (relative) {
            SetCenter(dqRect,&start[NumbOfDim * j]);
          }
          j++; i++;
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

void SetGlobrectOriginDists(t_RT rst,
                            typinterval *r,
                            typcoord *orig,
                            typcoord *dists)

{
  int d;
  Rint dummy;
  
  if (GetGlobalMBB(rst,&dummy,r)) {
    for (d= 0; d < NumbOfDim; d++) {
      orig[d]= r[d].l;
      dists[d]= r[d].h - r[d].l;
    }
  }
  else {
    ErrorExit("GetGlobalMBB FAILED");
  }
}

/***********************************************************************/

void SetRandPoint(typcoord *orig, typcoord *dists, typcoord *p)

{
  int d;
  double RealRand;

  for (d= 0; d < NumbOfDim; d++) {
    RealRand= drand48();
    p[d]= orig[d] + RealRand * dists[d];
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

void PrintGlobrect(typinterval *r)

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

boolean InfPartCmp(t_RT R,
                   const typinfo *stored,
                   Rint size,
                   const typinfo *searched,
                   void *ref) {

  /* we do not want to examine info parts */
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
