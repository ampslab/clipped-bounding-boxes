/* rectpar.c: print properties of nD rectangle files */
#//
#// Copyright (c) 1994 - 2015 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//

/* -- NOTE: In newer publications the notion "volume" has established for the
   --       area of multidimensional rectangles.
   --       Help function and output adapt to this evolution, though it's
   --       questionable whether this is a good decision.
   --       The help function, where the notion "squares/cubes" is used now
   --       instead of "squares", illustrates the arising problems.
   --       While in this example "cubes" would be intrinsically correct in
   --       view of the notion "volume", it could irritate the reader, because
   --       using the common upward-generalization "2D --> nD", an nD-area is
   --       clear, but there is indeed no 2D-volume.
   --       Bearing this in mind sub-strings in variable- and function-names
   --       (not only) in this source will keep the notion "area"! */
/* ++ 2nd NOTE: Since this double binded treatment might be irritating to the
   programmer, it might be lead back to a bare "area" usage sometime. */
   
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
//-----
#include "RSTOtherFuncs.h"
#include "RSTFileAccess.h"
/*implies "RSTTypes.h", "RSTStdTypes.h" */


/*** constants: ***/

#define MAX_NAME_LEN 40
// if MAX_NAME_LEN shall be changed, also the table formatting has to be
// changed (for simplicity does not use MAX_NAME_LEN)
#define tmpName "tmp_rectpar"
#define MEGA1_1 ((1 << 20) -1) /* 1M - 1 */

/*** types: ***/

/*** declarations: ***/

Rpint GetLOF(char *path); /* GetLengthOfFile */
boolean PrintStatistics(void);
boolean CalcExtParams(void);
void CalcAreaOvlpParams_VerboseArea0(void);
void FillovlpArr(void);
void PrintRectFormat(void);
void RP_QSortRfloats(Rint begin,
                  Rint end,
                  Rfloat value[]);
static void ExchangeRfloats(Rfloat *x, Rfloat *y);
static Rfloat RfloatsMedian(Rfloat v[],
                            Rint *b,
                            Rint m,
                            Rint *e);
boolean RP_Overlaps(const typinterval *r1, const typinterval *r2);
double RP_GetOverlap(const typinterval *r1, const typinterval *r2);
double GetArea(typinterval *r, Rint *numbDim);
void MakeAligned(Rint *numb, Rint alignm);
void ManageJoin(t_RT RT1, t_RT RT2,
                Rint numbDim,
                const typinterval *rect1, const typinterval *rect2,
                refinfo info1ptr, refinfo info2ptr,
                Rint info1Size, Rint info2Size,
                void *ptr,
                boolean *finish);
void PrintUsage(char *arg);
void PrintHelp(char *arg);
void PrTblRow3Digits(char *str1, char *str2, char *str3, double value);
void PrTblFormd(FILE *stream, int nDgts, double value);
double RoundUpLim(int nDgts, int expo);

boolean DirEncloses(t_RT rt,
                    Rint numbDim,
                    const typinterval *RSTrect,
                    const typinterval *qRects,
                    Rint qRectQty,
                    void *qPtr);
boolean DataEqual(t_RT rt,
                  Rint numbDim,
                  const typinterval *RSTrect,
                  const typinterval *qRects,
                  Rint qRectQty,
                  void *qPtr);

/*** global variables ***/

int inFile;
char fileName[MAX_NAME_LEN + 1];
double *avgExtArr;
double *nsdExtArr;
double *minDmaxExtArr;
double *areaArr;
double *ovlpArr;
double *ovlpDareaArr;
int *dim0Arr;
double leastAreaNot0;
int NumbOfDim, sizeRect;
Rpint lof, NumbRects, area0, nQrects, nPoints;
boolean isDetailed;
Rpint faultyRect= -1;
int faultyDim= -1;
Rpint pairCount= 0;


int main(int argc, char *argv[])

{
  int remain;
  char s[160];
  
  if (argc == 2 && (strcmp(argv[1],"-help") == 0 || strcmp(argv[1],"-h") == 0)) {
    PrintHelp(argv[0]);
    exit(0);
  }
  else if (argc == 3) {
    NumbOfDim= atof(argv[1]);
    isDetailed= FALSE;
    if (NumbOfDim <= 0) {
      PrintUsage(argv[0]);
      exit(1);
    }
    if (strlcpy(fileName,argv[2],MAX_NAME_LEN) > MAX_NAME_LEN) {
      fprintf(stderr,"Error: file name too long\n");
      exit(1);
    }
  }
  else if (argc == 4) {
    if (strcmp(argv[1],"-d") != 0) {
      PrintUsage(argv[0]);
      exit(1);
    }
    NumbOfDim= atof(argv[2]);
    isDetailed= TRUE;
    if (NumbOfDim <= 0) {
      PrintUsage(argv[0]);
      exit(1);
    }
    if (strlcpy(fileName,argv[3],MAX_NAME_LEN) > MAX_NAME_LEN) {
      fprintf(stderr,"Error: file name too long\n");
      exit(1);
    }
  }
  else {
    PrintUsage(argv[0]);
    exit(1);
  }
  
  sizeRect= sizeof(typinterval) * NumbOfDim;
  
  fprintf(stdout,"File: %s (%dD)\n",fileName,NumbOfDim);
  if (! OpenRdOnlyF(fileName,&inFile)) {
    exit(2);
  }
  lof= GetLOF(fileName);
  if (lof == 0) {
    fprintf(stderr,"Error: empty file\n");
    exit(2);
  }
  NumbRects= lof / sizeRect;
  fprintf(stdout,strans("Total number of rectangles: %P\n",s),NumbRects);
  remain= lof % sizeRect;
  if (remain != 0) {
    fprintf(stderr,"WARNING: INCOMPATIBLE FILE? Incomplete last rectangle of size %d ignored!\n",remain);
  }
    
  dim0Arr= malloc(sizeof(int) * (NumbOfDim + 1));
    
  if (! PrintStatistics()) {
    fprintf(stderr,strans("FATAL ERROR in rectangle %P, dimension %d!\n",s),faultyRect,faultyDim);
  }
  fprintf(stdout,"\n------------------------------------------------------------------------------\n");
  free(dim0Arr);
  return 0;
}

/***********************************************************************/

Rpint GetLOF(char *path) /* GetLengthOfFile */

{
  struct stat status;
  int ferr;
  
  ferr= stat(path,&status);
  if (ferr == -1) {
    return 0;
  }
  else {
    return status.st_size;
  }
}

/***********************************************************************/

boolean PrintStatistics(void)

{
  double sumAvgExt, sumNSDext, sumMDMext, sumArea, sumODA, 
         avgAvgExt, avgNSDext, avgMDMext, avgArea, avgODA;
  int d;
  Rpint i;
  char s[160];
  
  avgExtArr= malloc(sizeof(double) * NumbRects);
  nsdExtArr= malloc(sizeof(double) * NumbRects);
  minDmaxExtArr= malloc(sizeof(double) * NumbRects);
  
  fprintf(stderr,"--- WORKING on %s ---:\n",fileName);
  fprintf(stderr,"Collecting extension properties:\n");
  if (! CalcExtParams()) {
    return FALSE;
  }
  fprintf(stderr,"Done.\n");
  
  fprintf(stderr,"Sorting result arrays:\n");
  RP_QSortRfloats(0,NumbRects-1,avgExtArr);
  RP_QSortRfloats(0,NumbRects-1,nsdExtArr);
  RP_QSortRfloats(0,NumbRects-1,minDmaxExtArr);
  fprintf(stderr,"Done.\n");
  
  fprintf(stderr,"Computing sums:\n");
  sumAvgExt= 0; sumNSDext= 0.0; sumMDMext= 0.0;
  for (i= 0; i < NumbRects; i++) {
    sumAvgExt+= avgExtArr[i];
    sumNSDext+= nsdExtArr[i];
    sumMDMext+= minDmaxExtArr[i];
  }
  free(avgExtArr); avgExtArr= NULL;
  free(nsdExtArr); nsdExtArr= NULL;
  free(minDmaxExtArr); minDmaxExtArr= NULL;
  fprintf(stderr,"Done.\n");
  
  areaArr= malloc(sizeof(double) * NumbRects);
  ovlpArr= malloc(sizeof(double) * NumbRects);
  ovlpDareaArr= malloc(sizeof(double) * NumbRects);
  
  /* Messages for these steps in the called function: */
  CalcAreaOvlpParams_VerboseArea0();
  
  fprintf(stderr,"Sorting result arrays:\n");
  RP_QSortRfloats(0,NumbRects-1,areaArr);
  /* ovlpArr is NOT used for vast summation ==> no sort */
  RP_QSortRfloats(0,NumbRects-1,ovlpDareaArr);
  fprintf(stderr,"Done.\n");
  
  fprintf(stderr,"Computing sums:\n");
  sumArea= 0; sumODA= 0.0;
  for (i= 0; i < NumbRects; i++) {
    sumArea+= areaArr[i];
    sumODA+= ovlpDareaArr[i];
  }
  free(areaArr); areaArr= NULL;
  free(ovlpArr); ovlpArr= NULL;
  free(ovlpDareaArr); ovlpDareaArr= NULL;
  fprintf(stderr,"Done.\n");
  
  if (area0 > 0) {
    /* begin separate section */
    fprintf(stdout,"\n");
  }
  fprintf(stdout,strans("%s: rectangles with volume = 0: %P\n",s),fileName,area0);
  if (area0 > 0) {
    for (d= 0; d <= NumbOfDim; d++) {
      fprintf(stdout,"%s: Volume = 0 due to %2d of %2d extensions = 0: %9d",fileName,d,NumbOfDim,dim0Arr[d]);
      if (d == 0) {
        fprintf(stdout," (fp underflow)\n");
      }
      else {
        fprintf(stdout,"\n");
      }
    }
    /* end separate section */
    fprintf(stdout,"\n");
  }
  fprintf(stdout,"%s: Volume of smallest non degenerated rectangle: %.2e\n",fileName,leastAreaNot0);
  /* final calculations and output of avg(nsd), avg(minDmax), avg(ovlpDarea): */
  avgAvgExt= sumAvgExt / NumbRects;
  avgNSDext= sumNSDext / NumbRects;
  avgMDMext= sumMDMext/ NumbRects;
  avgArea= sumArea / NumbRects;
  avgODA= sumODA / NumbRects;
  fprintf(stdout,strans("%s: Number of quadratic rectangles: %P\n",s),fileName,nQrects);
  fprintf(stdout,strans("%s: Number of points: %P\n",s),fileName,nPoints);
  fprintf(stdout,"%36s  %11s  %13s  %6s\n","File","Property","Specificly","Result");
  fprintf(stdout,"====================================  ===========  =============  ==========\n");
  fprintf(stdout,"%35s:  %11s  %13s ",fileName,"extensions:","AVG:");
  PrTblFormd(stdout,3,avgAvgExt); fprintf(stdout,"\n");
  fprintf(stdout,"%35s:  %11s  %13s ",fileName,"extensions:","average %NSD:");
  PrTblFormd(stdout,3,avgNSDext); fprintf(stdout,"\n");
  fprintf(stdout,"%35s:  %11s  %13s ",fileName,"extensions:","average %m/M:");
  PrTblFormd(stdout,3,avgMDMext); fprintf(stdout,"\n");
  fprintf(stdout,"%35s:  %11s  %13s ",fileName,"volumes:","AVG:");
  PrTblFormd(stdout,3,avgArea); fprintf(stdout,"\n");
  fprintf(stdout,"%35s:  %11s  %13s ",fileName,"overlap:","average %O/V:");
  PrTblFormd(stdout,3,avgODA); fprintf(stdout,"\n");
  if (close(inFile) == -1) {
    perror("Closing input file");
    /* exit(2); */
  }
  if (NumbRects != i) {
    /* Error exceptionally to stdout, because table is already written */
    fprintf(stdout,strans("ERROR: #rectangles scanned: %P\n",s),i);
    exit(3);
  }
  return TRUE;
}

/***********************************************************************/
/* pre-calculations for avg(nsd(ext)), avg(minDmax(ext)) */

boolean CalcExtParams(void) {

  typinterval *rect= malloc(sizeRect);
  double ext, sumExt, sumsqrExt, minExt, maxExt, avgExt, varExt;
  int d;
  Rpint i, nbytes, pos, posSet;
  boolean faultyLen;
  
  nQrects= 0; nPoints= 0;
  pos= 0;
  posSet= lseek(inFile,pos,SEEK_SET);
  if (posSet != pos) {
    perror("rectpar: reading");
    exit(2);
  }
  for (i= 0; i < NumbRects; i++) {
    nbytes= read(inFile,rect,sizeRect);
    if (nbytes != sizeRect) {
      perror("rectpar: reading");
      exit(2);
    }
    sumExt= 0.0; sumsqrExt= 0.0; faultyLen= FALSE;
    for (d= 0; d < NumbOfDim; d++) {
      ext= rect[d].h - rect[d].l;
      if (ext < 0.0) {
        faultyLen= TRUE;
        faultyDim= d;
        break;
      }
      if (d == 0) {
        minExt= ext;
        maxExt= ext;
      }
      else {
        if (ext < minExt) {
          minExt= ext;
        }
        else if (ext > maxExt) {
          maxExt= ext;
        }
      }
      sumExt+= ext;
      sumsqrExt+= ext * ext;
    }
    if (faultyLen) {
      faultyRect= i;
      return FALSE;
    }
    avgExt= sumExt / NumbOfDim;
    /* avgExt < 0: faulty data */
    varExt= sumsqrExt / NumbOfDim - avgExt * avgExt;
    /* var: math.: always >= 0 */
    avgExtArr[i]= avgExt;
    if (maxExt > 0.0) {
      /* rectangle is not point */
      minDmaxExtArr[i]= 100.0 * minExt / maxExt;
      if (varExt > 0.0) {
        /* rectangle is not quadratic */
        nsdExtArr[i]= 100.0 * sqrt(varExt) / avgExt;
      }
      else {
        /* rectangle is quadratic BUT NOT POINT */
        nsdExtArr[i]= 0.0;
        nQrects++; /* not counted for points */
      }
    }
    else {
      /* rectangle is point */
      minDmaxExtArr[i]= 100.0;
      nPoints++; /* not counted for quadratic */
    }
  }
  free(rect);
  return TRUE;
}

/***********************************************************************/
/* fill dim0Arr; compute leastAreaNot0; pre-calculations for avg(area);
   FillovlpArr calls RR*-tree.SpatialJoin; detailed information about
   rectangles with area = 0 */

void CalcAreaOvlpParams_VerboseArea0(void) {

  typinterval *rect= malloc(sizeRect);
  char str[23]; /* max: #4294967295,4294967295 */
  double area;
  int d, numbDim;
  Rpint i, nbytes, pos, posSet;
  char s[160];
  boolean LFpending;
  
  if (isDetailed) {
    fprintf(stderr,strans("Performing \"detailed\" task for %P rectangles.\n",s),NumbRects);
  }
  /* Fill ovlpArr with index structure (RR*-Tree) */
  /* Messages for this step in the called function: */
  FillovlpArr();
  for (d= 0; d <= NumbOfDim; d++) {
    dim0Arr[d]= 0;
  }
  area0= 0; LFpending= FALSE;
  leastAreaNot0= 0.0;
  pos= 0;
  posSet= lseek(inFile,pos,SEEK_SET);
  if (posSet != pos) {
    perror("rectpar: reading");
    exit(2);
  }
  
  fprintf(stderr,"Collecting volume and overlap properties:\n");
  for (i= 0; i < NumbRects; i++) {
    nbytes= read(inFile,rect,sizeRect);
    if (nbytes != sizeRect) {
      perror("rectpar: reading");
      exit(2);
    }
    area= GetArea(rect,&numbDim);
    areaArr[i]= area;
    if (area == 0.0) {
      if (isDetailed && area0 == 0) {
        fprintf(stdout,"Volume = 0 for the following rectangles:\n");
      }
      area0++;
      if (isDetailed) {
        sprintf(str,strans("#%P,%2d",s),i,numbDim);
        fprintf(stdout,"%14s",str); LFpending= TRUE;
        if (area0 % 5 == 0) {
          fprintf(stdout,"\n"); LFpending= FALSE;
        }
        if (ovlpArr[i] != 0.0) {
          fprintf(stdout,strans("\n%s: INCONSISTENCE: volume = 0 but ovlp != 0 ?? at %P\n",s),fileName,i);
        }
      }
      dim0Arr[numbDim]++;
      ovlpDareaArr[i]= 0.0;
    }
    else {
      ovlpDareaArr[i]= 100.0 * ovlpArr[i] / area;
      if (leastAreaNot0 == 0.0) {
        leastAreaNot0= area;
      }
      else {
        if (area < leastAreaNot0) {
          leastAreaNot0= area;
        }
      }
    }
  }
  free(rect);
  if (LFpending) {
    fprintf(stdout,"\n");
  }
  fprintf(stderr,"Done.\n");
}

/***********************************************************************/

void FillovlpArr(void) {
  
# define GIGA1 (1LL << 30) /* 1G */
# define MAX_MAIN_MEM_USE (12 * GIGA1) /* roughly(!) followed */
  Rpint max_main_mem_use= MAX_MAIN_MEM_USE - 3 * NumbRects * sizeof(double);
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
  t_RT RT, RT2;
  t_LRU LRU;
  boolean isRTonDisk;
  typinterval *rect= malloc(sizeRect);
  Rpint dirRAMSize, dataRAMSize;
  Rint nomPageSize, pageGrnlMult, pageSize;
  Rpnint LRUcap;
  Rint sRF= sizeof(Rfloat);
  Rint infoSize= sizeof(Rpint);
  Rint entrySize= NumbOfDim * 2 * sRF + infoSize;
  refinfo info= malloc(infoSize);
  refinfo infoStored= malloc(infoSize);
  Rpint *RpintPtr;
  Rpint nbytes, i, pos, posSet;
  boolean inserted, success;
  char s[160];
  int unused= FALSE;
  
  MakeAligned(&entrySize,sizeof(typatomkey));
  
  InitRSTreeIdent(&RT);
  if (lof < max_main_mem_use / N_E_M_DATA_FAC) {
    isRTonDisk= FALSE;
    /* page size calculation: */
    nomPageSize= sRF + NumbOfDim * sRF + N_ENT_MAIN * entrySize;
    pageSize= nomPageSize;
    /* align upwards: */
    MakeAligned(&pageSize,PG_GRNL_MAIN);
    
    dirRAMSize= lof * N_E_M_DIR_FAC;
    if (dirRAMSize < 3 * pageSize) {
      dirRAMSize= 3 * pageSize;
    }
    dataRAMSize= lof * N_E_M_DATA_FAC;
    if (dataRAMSize < 3 * pageSize) {
      dataRAMSize= 3 * pageSize;
    }
    
    fprintf(stderr,"RR*-tree.CreateMainMemRST: ");
    if (CreateMainMemRST(&RT,pageSize,pageSize,
                         NumbOfDim,infoSize,FALSE,
                         dirRAMSize,dataRAMSize,TRUE)) {
      fprintf(stderr,"Done.\n");
    }
    else {
      fprintf(stderr,"FAILED.\n");
      exit(2);
    }
  }
  else {
    isRTonDisk= TRUE;
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
    fprintf(stderr,"RR*-tree.CreateRST: %s ",tmpName);
    if (CreateRST(tmpName,pageSize,pageSize,NumbOfDim,infoSize,FALSE)) {
      fprintf(stderr,"Done.\n");
    }
    else{
      fprintf(stderr,"FAILED.\n");
      exit(2);
    }
    
    /* LRU buffer capacity calculation in pages: */
    LRUcap= max_main_mem_use / pageSize;
    
    fprintf(stderr,"LRU.NewLRU: ");
    success= TRUE;
    NewLRU(&LRU,LRUcap,pageSize,&success);
    if (success) {
      fprintf(stderr,"Done.\n");
    }
    else{
      fprintf(stderr,"FAILED.\n");
      exit(2);
    }
    fprintf(stderr,"RR*-tree.OpenBufferedRST: %s ",tmpName);
    if (OpenBufferedRST(&RT,tmpName,LRU)) {
      fprintf(stderr,"Done.\n");
    }
    else{
      fprintf(stderr,"FAILED.\n");
      exit(2);
    }
  }
  
  pos= 0;
  posSet= lseek(inFile,pos,SEEK_SET);
  if (posSet != pos) {
    perror("rectpar: reading");
    exit(2);
  }
  
  fprintf(stderr,"RR*-tree: insertion running:\n");
  for (i= 0; i < NumbRects; i++) {
    nbytes= read(inFile,rect,sizeRect);
    if (nbytes != sizeRect) {
      perror("rectpar: reading");
      exit(2);
    }
    /* initialize ovlpArr: */
    ovlpArr[i]= 0.0;
    /* number rectangle to be recognized in the spatial Join: */
    RpintPtr= (Rpint *)(*info).c;
    *RpintPtr= i;
    /* then insert the rectangle */
    success= InsertRecord(RT,rect,info,&inserted,infoStored);
    if (! success) {
      fprintf(stderr,"FAILURE(RR*-tree.InsertRecord)\n");
      exit(2);
    }
  }
  fprintf(stderr,"Done.\n");
  
  fprintf(stderr,"RR*-tree: spatial join running:\n");
  RT2= RT;
  success= SpJoin(RT,RT2,&unused,ManageJoin);
  if (! success) {
    fprintf(stderr,"FAILURE(RR*-tree.SpJoin)\n");
    exit(2);
  }
  /* The following print is required: Intermediate numbers are printed by
     ManageJoin on the same line ("\r"). Prints final number with a "\n". */
  fprintf(stderr,strans("#pairs found: %10P\n",s),pairCount);
  if ((pairCount & 1) != 0) {
    /* odd number of pairs */
    fprintf(stderr,"INCONSISTENCY(RR*-tree.SpJoin): odd number of pairs");
  }
  fprintf(stderr,"Done.\n");
  fprintf(stdout,strans("Number of intersections (points included): %P\n",s),pairCount / 2);
    
  if (isRTonDisk) {
    fprintf(stderr,"RR*-tree.CloseRST: ");
    success= CloseRST(&RT);
    if (! success) {
      fprintf(stderr,"FAILURE(RR*-tree.CloseRST)\n");
      /* exit(2); */
    }
    fprintf(stderr,"Done.\n");
    DisposeLRU(&LRU);
    TryRemoveRST(tmpName);
  }
  else {
    RemoveMainMemRST(&RT);
  }
  
  free(info);
  free(infoStored);
  free(rect);
}

/***********************************************************************/

void ManageJoin(t_RT RT1, t_RT RT2,
                Rint numbDim,
                const typinterval *rect1, const typinterval *rect2,
                refinfo info1, refinfo info2,
                Rint info1Size, Rint info2Size,
                void *ptr,
                boolean *finish) {

  double ovlp;
  Rpint *RpintPtr1, *RpintPtr2;
  Rpint ind1, ind2;
  char s[160];
  /* global: Rpint pairCount= 0; */
  
  RpintPtr1= (Rpint *)&(*info1).c;
  RpintPtr2= (Rpint *)&(*info2).c;
  ind1= *RpintPtr1;
  ind2= *RpintPtr2;
  if (ind1 != ind2) {
    if (RP_Overlaps(rect1,rect2)) {
      ovlp= RP_GetOverlap(rect1,rect2);
      ovlpArr[ind1]+= ovlp;
      /* each rectangle is rect1 once! */
    }
    pairCount++;
    if ((pairCount & MEGA1_1) == MEGA1_1) {
      /* (pairCount % 1048575 == 0) */
      fprintf(stderr,strans("#pairs found: %10P\r",s),pairCount);
      fflush(stderr);
    }
  }
}

/***********************************************************************/

void PrintRectFormat(void)

{
  if (sizeof(typatomkey) == sizeof(double)) {
    fprintf(stdout,"struct {double l, h;} rectangle[NumbOfDim].\n");
  }
  else if (sizeof(typatomkey) == sizeof(float)) {
    fprintf(stdout,"struct {float l, h;} rectangle[NumbOfDim].\n");
  }
  else {
    fprintf(stdout,"struct {UNKNOWN l, h;} rectangle[NumbOfDim].\n");
  }
}

/***********************************************************************/

void PrintUsage(char *arg) {

  fprintf(stderr,"%s%s%s","Usage: ",arg," -help | -h\n");
  fprintf(stderr,"%s%s%s","       ",arg," [-d] NumbOfDim file\n");
}

/***********************************************************************/

void PrintHelp(char *arg) {

  fprintf(stdout,
"SYNOPSYS\n");
  fprintf(stdout,"%s%s%s",
"    ",arg," -help | -h\n");
  fprintf(stdout,"%s%s%s",
"    ",arg," NumbOfDim [-d] file\n");
  fprintf(stdout,"\n");
  fprintf(stdout,"%s%s",
arg," works on files, containing axis parallel multi dimensional\n");
  fprintf(stdout,
"rectangles of the following format:\n");
    PrintRectFormat();
  fprintf(stdout,
"It determines properties of the rectangles in the file and prints them to\n");
  fprintf(stdout,
"stdout. Error messages and some verbose state information are printed to\n");
  fprintf(stdout,
"stderr.\n");
  fprintf(stdout,
"Options:\n");
  fprintf(stdout,
"  -d  \"detailed\"\n");
  fprintf(stdout,"\n");
  fprintf(stdout,
"If -d is set, a list of all degenerated rectangles (volume = 0) is given\n");
  fprintf(stdout,
"out additionally. For 3D these would be points, lines and planes.\n");
  fprintf(stdout,
"The list consists of tuples (#number, number), where the first term\n");
  fprintf(stdout,
"denotes the position of the rectangle in the file (starting at 0) and the\n");
  fprintf(stdout,
"second term denotes the number of axes, where the extension is 0.\n");
  fprintf(stdout,"\n");
  fprintf(stdout,"%s%s%s",
"Overall ",arg," consecutively prints the following properties:\n");
  fprintf(stdout,
"  - the name of the examined file, followed by its dimensionality\n");
  fprintf(stdout,
"  - the total number of rectangles scanned\n");
  fprintf(stdout,
"  - the number of rectangle intersections (points included)\n");
  fprintf(stdout,
"( - a list of degenerated rectangles if -d was set)\n");
  fprintf(stdout,
"  - the number of degenerated rectangles (volume = 0)\n");
  fprintf(stdout,
"  - a compilation about how many rectangles are degenerated due to how\n");
  fprintf(stdout,
"    many extensions being 0 (left out if the previous point yielded 0)\n");
  fprintf(stdout,
"  - the volume of the smallest non degenerated rectangle\n");
  fprintf(stdout,
"  - the number of quadratic rectangles (not being points)\n");
  fprintf(stdout,
"  - the number of points.\n");
  fprintf(stdout,"\n");
  fprintf(stdout,
"The above information is followed by a table of average calculations:\n");
  fprintf(stdout,
"  - extensions:           AVG: the average extension of a rectangle in a\n");
  fprintf(stdout,
"                               single dimension, i.e. the average over\n");
  fprintf(stdout,
"                               all extensions of all rectangles\n");
  fprintf(stdout,
"  - extensions:  average %%NSD: the average of the normalized standard\n");
  fprintf(stdout,
"                               deviation over all extensions of a\n");
  fprintf(stdout,
"                               rectangle in %%, which is 0 if all\n");
  fprintf(stdout,
"                               rectangles are squares/cubes\n");
  fprintf(stdout,
"  - extensions:  average %%m/M: the average ratio\n");
  fprintf(stdout,
"                               (minimum extension) / (Maximum extension)\n");
  fprintf(stdout,
"                               of a rectangle in %%, which is 100 if all\n");
  fprintf(stdout,
"                               rectangles are squares/cubes\n");
  fprintf(stdout,
"  -    volumes:           AVG: the average volume of a rectangle\n");
  fprintf(stdout,
"  -    overlap:  average %%O/V: the average ratio\n");
  fprintf(stdout,
"                               (spatial overlap) / (volume) of a rectangle\n");
  fprintf(stdout,
"                               in %%, [0 .. Infinity], 0 if none of the\n");
  fprintf(stdout,
"                               rectangles overlaps with another.\n");
fprintf(stdout,"%s\n",
"Exit status:"
);
  fprintf(stdout,"%s\n",
"  Returns 0 on success;"
  );
  fprintf(stdout,"%s\n",
"  Returns 1 if a parameter mismatch was detected;"
  );
  fprintf(stdout,"%s\n",
"  Returns 2 if a fatal error occured."
  );
}

/***********************************************************************/

void RP_QSortRfloats(Rint begin,
                     Rint end,
                     Rfloat value[])
/* Sorts value. */

{
  Rfloat midelem;
  Rint i, j;
  
  i= begin; j= end;
  /* midelem= value[(i+j) >> 1]; */
  /* profitable for #elements > 7: */
  midelem= RfloatsMedian(value,&i,(i+j) >> 1,&j);
  if (i < j) {
    do {
      while (value[i] < midelem) {
        i++;
      }
      while (value[j] > midelem) {
        j--;
      }
      if (i < j) {
        ExchangeRfloats(&value[i],&value[j]);
        i++; j--;
      }
      else if (i == j) {
        i++; j--;
      }
    } while (i <= j);
    if (begin < j) {
      if (j - begin > 1) {
        RP_QSortRfloats(begin,j,value);
      }
      else {
        if (value[begin] > value[j]) {
          ExchangeRfloats(&value[begin],&value[j]);
        }
      }
    }
    if (i < end) {
      if (end - i > 1) {
        RP_QSortRfloats(i,end,value);
      }
      else {
        if (value[i] > value[end]) {
          ExchangeRfloats(&value[i],&value[end]);
        }
      }
    }
  }
}

/***********************************************************************/

static void ExchangeRfloats(Rfloat *x, Rfloat *y)

{
  Rfloat z;
  
  z= *x; *x= *y; *y= z;
}

/***********************************************************************/

static Rfloat RfloatsMedian(Rfloat v[],
                            Rint *b,
                            Rint m,
                            Rint *e)

{
  if (v[*b] <= v[m]) {
    if (v[m] <= v[*e]) {                 //bme (sorted)
    }
    else {
      if (v[*b] <= v[*e]) {              //bem
        ExchangeRfloats(&v[m],&v[*e]);
      }
      else {                             //ebm  if (b == m) { x(b,e) } else ...
        ExchangeRfloats(&v[m],&v[*e]);   //     optimizes #exchanges
        ExchangeRfloats(&v[*b],&v[m]);
      }
    }
  }
  else {
    if (v[*b] <= v[*e]) {                //mbe
      ExchangeRfloats(&v[*b],&v[m]);
    }
    else {
      if (v[m] < v[*e]) {                //meb (NOTE the "<"!!)
        ExchangeRfloats(&v[*b],&v[m]);
        ExchangeRfloats(&v[m],&v[*e]);
      }
      else {                             //emb
        ExchangeRfloats(&v[*b],&v[*e]);
      }
    }
  }
  (*b)++; (*e)--;
  return v[m];
}

/***********************************************************************/

boolean RP_Overlaps(const typinterval *rect1, const typinterval *rect2) {
  Rint d;
  
  d= 0;
  do {
    if (rect1[d].l > rect2[d].h || rect1[d].h < rect2[d].l) {
      return FALSE;
    }
    d++;
  } while (d < NumbOfDim);
  return TRUE;
}

/***********************************************************************/

double RP_GetOverlap(const typinterval *r1, const typinterval *r2) {
  Rfloat low, high, spc;
  Rint d;
  
  spc= 1.0;
  for (d= 0; d < NumbOfDim; d++) {
    if (r1[d].l < r2[d].l) {
      low= r2[d].l;
    }
    else {
      low= r1[d].l;
    }
    if (r1[d].h < r2[d].h) {
      high= r1[d].h;
    }
    else {
      high= r2[d].h;
    }
    spc= spc * (high-low);
  }
  return spc;
}

/***********************************************************************/

double GetArea(typinterval *r, Rint *numbDim) {
  Rfloat spc, len;
  Rint d;
  
  spc= 1.0; *numbDim= 0;
  for (d= 0; d < NumbOfDim; d++) {
    len= r[d].h - r[d].l;
    if (len == 0.0) {
      (*numbDim)++;
    }
    spc= spc * len;
  }
  return spc;
}

/***********************************************************************/

void MakeAligned(Rint *numb, Rint alignm) {

  Rint ill= *numb & (alignm - 1);
  if (ill != 0) {
    (*numb)+= alignm - ill;
  }
}

/************************************************************************/

boolean DirEncloses(t_RT rt,
                    Rint numbDim,
                    const typinterval *RSTrect,
                    const typinterval *qRects,
                    Rint qRectQty,
                    void *qPtr)

{
  int d;
  
  //GVdirComparisons++;
  d= 0;
  do {
    if ((*RSTrect).l > (*qRects).l || (*RSTrect).h < (*qRects).h) {
      return FALSE;
    }
    RSTrect++; qRects++;
    d++;
  } while (d < numbDim);
  return TRUE;
}

/***********************************************************************/

boolean DataEqual(t_RT rt,
                  Rint numbDim,
                  const typinterval *RSTrect,
                  const typinterval *qRects,
                  Rint qRectQty,
                  void *qPtr)

{
  int d;
  
  //GVdataComparisons++;
  d= 0;
  do {
    if ((*RSTrect).l != (*qRects).l || (*RSTrect).h != (*qRects).h) {
      return FALSE;
    }
    RSTrect++; qRects++;
    d++;
  } while (d < numbDim);
  return TRUE;
}

/***********************************************************************/

void PrTblRow3Digits(char *str1, char *str2, char *str3, double value) {

  if (value < 1.0) {
    fprintf(stdout,"%35s:  %11s  %13s %11.2e\n",str1,str2,str3,value);
  }
  else if (value < 10.0) {
    fprintf(stdout,"%35s:  %11s  %13s %7.2f\n",str1,str2,str3,value);
  }
  else if (value < 100.0) {
    fprintf(stdout,"%35s:  %11s  %13s %6.1f\n",str1,str2,str3,value);
  }
  else {
    fprintf(stdout,"%35s:  %11s  %13s %4.0f\n",str1,str2,str3,value);
  }
}

/***********************************************************************/
/* The function prints doubles ready formatted for tables and with a constant
   precision of nDgts to stream.
   Formatting:
   - For security reasons, a single leading blank is inserted.
   - For security reasons, if the function is called with nDgts <= 1
     (e-formatting impossible), nDgts is set to 2.
   The "." is always at the same position (even if invisible);
        if ROUNDED PRINTED value <  0.01,                  %e - formatting;
   else if ROUNDED PRINTED value <  pow(10,nDgts-1),       %f - formatting;
   else if ROUNDED PRINTED value <  pow(10,nDgts),    integer - formatting;
   else  ( ROUNDED PRINTED value >= pow(10,nDgts) ),       %e - formatting;
   Example (nDgts = 3):
   
   |   9.99e-03|
   |   0.0100  |
   |   0.0999  |
   |   0.100   |
   |   0.999   |
   |   1.00    |
   |   9.99    |
   |  10.0     |
   |  99.9     |
   | 100       |
   | 999       |
   |   1.00e+03|
*/
/** 
    CAUTION: The function assumes e-formatting "T.Fe-EE" NOT "T.Fe-EEE" !!
    For EEE formatting (Unix if required, Windows default), e-formatted
    numbers are shifted to the left (the dot moves). Apart from that, the
    formatting is stable.
**/
/* Programmers notes:
   Shorter number strings are padded with leading blanks, therefore the
   computation of the length of the number strings always contains the maximum
   length of the trunc max(trunc). */

void PrTblFormd(FILE *stream, int nDgts, double value) {

  int i, e_formLen, len, prec, pad;
  char format[80];
  
  if (nDgts <= 1) {
    nDgts= 2;
  }
  e_formLen= nDgts + 5 + nDgts-1; // max(trunc) + ".e-EE" + max(frac)
  
  if (value < RoundUpLim(nDgts,-2)) {
    len= e_formLen;
    prec= nDgts - 1;
    sprintf(format," %%%d.%de",len,prec);
    fprintf(stream,format,value);
    // fprintf(stream," value < %f (A)",RoundUpLim(nDgts,0));
    return;
  }
  for (i= -1; i <= nDgts; i++) {
    if (value < RoundUpLim(nDgts,i)) {
      if (i == nDgts) {
        len= nDgts; // max(trunc)
      }
      else {
        len= nDgts + 1 + nDgts-i; // max(trunc) + "." + frac
      }
      prec= nDgts - i;
      sprintf(format," %%%d.%df",len,prec);
      for (pad= len; pad < e_formLen; pad++) {
        strcat(format," ");
      }
      fprintf(stream,format,value);
      // fprintf(stream," value < %f (B)",RoundUpLim(nDgts,i));
      return;
    }
  }
  /* fall through (function did not return): */
  len= e_formLen;
  prec= nDgts - 1;
  sprintf(format," %%%d.%de",len,prec);
  fprintf(stream,format,value);
  // fprintf(stream," value >= %f (D)",RoundUpLim(nDgts,nDgts));
}

/***********************************************************************/

double RoundUpLim(int nDgts, int expo) {

int nFracDgts;
double power;

  power= pow(10,expo);
  nFracDgts= nDgts - expo;
  return power - pow(10,-nFracDgts) * 0.5;
}

/***********************************************************************/

