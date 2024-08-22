/* MDsuc: see option -help */
#//
#// Copyright (c) 1994 - 2015 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
/* MDsuc: see option -help *** build surrounding containers */
/* MDsuc.c: grep Param. */

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

#define RANDSEED 8807 /* <--- set Param. */
#define tmpName "tmp_MDsuc"
#define RAND_PLUS_MINUS_FAC 0.5 /* <--- set Param. */
#define EPS_FAC 0.0 /* <--- set Param. */


/* types */


/* declarations */

void DistfileToRST(char *distname,
                   t_RT *rst,
                   t_LRU *LRU,
                   boolean *isRTonDisk);
void CreateQueryRects(char *inname, int AvgNumbNbrs, t_RT rst, char *outname);
Rpint GetLOF(char *path); /* GetLengthOfFile */
void PrintRectFormat(void);
void PrintNumbRecords(Rpint numb, int remain);
void PrintDescription(t_RT rst);
boolean AlwaysTrue(t_RT R,
                   Rint numbDim,
                   const typinterval *RSTrect,
                   const typinterval *qRect,
                   Rint qRectQty,
                   void *qPtr);
void CalcLinf (Rfloat coordDist, Rfloat *cumuDist);
void InitResultRect(typcoord *qp, typinterval *neighbor, typinterval *r, int numbnbrs);
void AdaptResultRect(typinterval *neighbor, typinterval *r);
Rfloat EpsOfLssGtr(Rfloat Lss, Rfloat Gtr);
void ErrorExit(char *str);
void NotExistsExit(int numbnbrs);
void SetCenter(typinterval *r, typcoord *p);
void MakeAligned(Rint *numb, Rint alignm);

/* global variables: */

Rint rectSize= NumbOfDim * sizeof(typinterval);
Rint pointSize= NumbOfDim * sizeof(typcoord);
char s[160];


int main(int argc, char *argv[])

{
  t_RT rst;
  t_LRU LRU;
  boolean isRTonDisk;
  
  if (argc == 2 && strcmp(argv[1],"-help") == 0) {
    printf(
"SYNOPSYS\n");
    printf("%s%s%s",
"     Usage: ",argv[0]," -help\n");
    printf("%s%s%s",
"            ",argv[0]," inputfile #neighbors distribfile outputfile\n");
    printf("\n");
    printf("%s%s",
argv[0]," works on files containing rectangles in the following format:\n");
    PrintRectFormat();
    printf(
"It reads all the k rectangles from inputfile (typically containing query\n");
    printf(
"points), performs distance queries with their centers in distribfile,\n");
    printf(
"finally stores k query-rectangles in outputfile.\n");
    printf(
"More precisely:\n");
    printf(
"For each rectangle-center of inputfile n nearest neighbors are searched\n");
    printf(
"in distribfile, the search metrics being defined L(infinity).\n");
    printf(
"From each set of n neighbors a query rectangle is built which, applied to\n");
    printf(
"an intersection query, retrieves the n neighbors. The arising query\n");
    printf(
"rectangles are stored in outputfile.\n");
    printf(
"NOTE that:\n");
    printf(
"- If #neighbors = 1, n = 1.\n");
    printf(
"- If #neighbors > 1, n randomly ranges in:\n");
    printf(
"    int(#neighbors +/- (%.2f * #neighbors)).\n",RAND_PLUS_MINUS_FAC);
    printf(
"- The search point is NOT necessarily part of the query rectangle.\n");
    printf(
"- An odd #neighbors leads to an average n of #neighbors - 0.5.\n");
    printf(
"- A small #neighbors, where the range of n contains 0 leads to less than k\n");
    printf(
"  query-rectangles in outputfile.\n");
    printf(
"At the end outputfile contains query-rectangles defined by the\n");
    printf(
"n-neighbors-in-distribfile-surrounding of the centers of the rectangles of\n");
    printf(
"inputfile.\n");
    printf(
"Obviously each last rectangle specifying the surrounding is only tangent\n");
    printf(
"to it. Thus, depending on the input format, internal conversions may\n");
    printf(
"inhibit that rectangle intersection queries with the generated\n");
    printf(
"query-rectangles actually retrieve n rectangles.\n");
    printf(
"Therefore - in this version of %s - the resulting rectangles are\n",argv[0]);
    printf(
"extended by an epsilon of %.2e * Max(|l|,|h|,dist),\n",EPS_FAC);
    printf(
"where l and h are the edge coordinates of the rectangle, and dist is the\n");
    printf(
"distance between these on the considered axis.\n");
    exit(1);
  }
  else if (argc != 5 ) {
    printf("%s%s%s",
"Usage: ",argv[0]," -help\n");
    printf("%s%s%s",
"       ",argv[0]," inputfile #neighbors distribfile outputfile\n");
    exit(1);
  }
  else {
    // srand48(RANDSEED); /* inadvertantly not set originally! */
    InitRSTreeIdent(&rst);
    DistfileToRST(argv[3],&rst,&LRU,&isRTonDisk);
    printf("-----\n");
    printf("Create %s from %d neighbors of %s in %s\n",argv[4],atoi(argv[2]),argv[1],argv[3]);
    CreateQueryRects(argv[1],atoi(argv[2]),rst,argv[4]);
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
/* --- with inputfile perform distance queries in distribfile --- */
/* --- copy the resulting rectangles to outputfile --- */

void CreateQueryRects(char *inname, int AvgNumbNbrs, t_RT rst, char *outname)

{
  Rpint lof, numbrecs, bytesrest, randLoLim, randHiLim, numbnbrs,
        cntlmt, itemsread, i, j;
  boolean exists;
  double RealRand;
  t_DQ distQ;
  /* distance query record: */
  typinterval *dqRect= malloc(rectSize);
  typinfo dqInfo;
  Rfloat dqRawDist;
  /* ---------------------- */
  typinterval *inrect= malloc(rectSize);
  typinterval *outrect= malloc(rectSize);
  typcoord *center= malloc(pointSize);
  FILE *infile, *outfile;
  
  infile= fopen(inname,"rb");
  lof= GetLOF(inname);
  if (lof == 0) {
    perror(inname);
    exit(2);
  }
  numbrecs= lof / rectSize;
  bytesrest= lof % rectSize;
  PrintRectFormat();
  PrintNumbRecords(numbrecs,bytesrest);
  
  if (AvgNumbNbrs == 1) {
    randLoLim= 1;
    randHiLim= 1;
  }
  else {
    randLoLim= AvgNumbNbrs - AvgNumbNbrs * RAND_PLUS_MINUS_FAC;
    randHiLim= AvgNumbNbrs + AvgNumbNbrs * RAND_PLUS_MINUS_FAC;
  }
  printf("#neighbors (on average): %d\n",AvgNumbNbrs);
  printf(strans("  #neighbors (actually): %P .. %P\n",s),randLoLim,randHiLim);
  
  outfile= fopen(outname,"wb");
  
  InitDistQueryIdent(&distQ);
  printf("Distance Queries: WORKING ...\n");
  cntlmt= 1;
  for (i= 1; i <= numbrecs; i++) {
    itemsread= fread(inrect,rectSize,1,infile);
    if (itemsread < 1) {
      ErrorExit("infile read FAILED");
    }
    RealRand= drand48();
    numbnbrs= RealRand * (randHiLim - randLoLim + 1) + randLoLim;
    if (numbnbrs > 0) {
      SetCenter(inrect,center);
      if (! NewDistQuery(rst,center,CalcLinf,inc,minDist,0.0,NULL,0,NULL,AlwaysTrue,AlwaysTrue,2048,FALSE,&distQ)) {
        ErrorExit("NewDistQuery FAILED");
      }
      if (GetDistQueryRec(distQ,rst,dqRect,&dqInfo,&dqRawDist,&exists)) {
        if (exists) {
          InitResultRect(center,dqRect,outrect,numbnbrs);
        }
        else {
          NotExistsExit(1);
        }
      }
      else {
        ErrorExit("GetDistQueryRec FAILED");
      }
      for (j= 2; j <= numbnbrs; j++) {
        if (GetDistQueryRec(distQ,rst,dqRect,&dqInfo,&dqRawDist,&exists)) {
          if (exists) {
            AdaptResultRect(dqRect,outrect);
          }
          else {
            NotExistsExit(j);
          }
        }
        else {
          ErrorExit("GetDistQueryRec FAILED");
        }
      }
      DisposeDistQuery(&distQ);
      fwrite(outrect,rectSize,1,outfile);
    }
    
    if (i == cntlmt) {
      printf(strans("%P\n",s),i);
      cntlmt*= 2;
    }
  }
  printf(strans("%P Done.\n",s),numbrecs);
  fclose(outfile);
  fclose(infile);
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

void CalcLinf (Rfloat coordDist, Rfloat *cumuDist)

{
  if (fabs(coordDist) > *cumuDist) {
    *cumuDist= fabs(coordDist);
  }
}

/***********************************************************************/
/* ">=" instead of ">" and addition of an epsilon */

void InitResultRect(typcoord *qp, typinterval *neighbor, typinterval *r, int numbnbrs)

{
  int d;
  
  for (d= 0; d < NumbOfDim; d++) {
    if (neighbor[d].l >= qp[d]) {
      r[d].l= neighbor[d].l - EpsOfLssGtr(qp[d],neighbor[d].l);
      r[d].h= neighbor[d].l + EpsOfLssGtr(qp[d],neighbor[d].l);
    }
    else if (neighbor[d].h <= qp[d]) {
      r[d].l= neighbor[d].h - EpsOfLssGtr(neighbor[d].h,qp[d]);
      r[d].h= neighbor[d].h + EpsOfLssGtr(neighbor[d].h,qp[d]);
    }
    else {
      r[d].l= qp[d];
      r[d].h= qp[d];
    }
  }
}

/***********************************************************************/
/* ">=" instead of ">" and addition of an epsilon */

void AdaptResultRect(typinterval *neighbor, typinterval *r)

{
  int d;
  
  for (d= 0; d < NumbOfDim; d++) {
    if (neighbor[d].l >= r[d].h) {
      r[d].h= neighbor[d].l + EpsOfLssGtr(r[d].l,neighbor[d].l);
    }
    else if (neighbor[d].h <= r[d].l) {
      r[d].l= neighbor[d].h - EpsOfLssGtr(neighbor[d].h,r[d].h);
    }
  }
}

/***********************************************************************/

Rfloat EpsOfLssGtr(Rfloat Lss, Rfloat Gtr)

{
  Rfloat dst, absGtr, absLss;
  
  dst= Gtr - Lss;
  absLss= fabs(Lss);
  absGtr= fabs(Gtr);
  if (absGtr > absLss) {
    if (dst > absGtr) {
      return dst * EPS_FAC;
    }
    else {
      return absGtr * EPS_FAC;
    }
  }
  else {
    if (dst > absLss) {
      return dst * EPS_FAC;
    }
    else {
      return absLss * EPS_FAC;
    }
  }
}

/***********************************************************************/

void ErrorExit(char *str)

{
  printf("%s\n",str);
  exit(2);
}

/***********************************************************************/

void NotExistsExit(int numbnbrs)

{
  printf("GetDistQueryRec:\n");
  printf("Query exhausted! distribfile contains less than %d rectangles.\n",numbnbrs);
  printf("FAILURE\n");
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
