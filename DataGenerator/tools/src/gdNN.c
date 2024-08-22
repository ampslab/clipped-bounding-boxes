/* gdNN.c: multiply the number of entries of a rectangle distribution */
#//
#// Copyright (c) 1994 - 2015 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//


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
#include "RSTDistQueryFuncs.h"
#include "RSTFileAccess.h"
/*implies "RSTTypes.h", "RSTStdTypes.h" */

/* let's have a boolean */
// type boolean from RR*-tree (ok as long as the following definition holds)
// boolean = int; FALSE = 0; TRUE = 1

/* -- constants -- */
#define MAX_NAME_LEN_IN 100
#define MAX_SUFF_LEN 10
#define MAX_NAME_LEN_OUT 110
#define tmpName "tmp_gdNN"

/*-- types -- */
// type typinterval from RR*-tree (ok as long as RR*-tree.Rfloat = double)
typedef typinterval   *typrect;
typedef typinterval  *typedges;
// type File from RR*-tree (ok as long as RR*-tree.File = int)

/* -- global variables -- */
int GVnumbDim;
Rpint GVinNumbRects;
Rpint GVsizeRect;

/* -- declarations -- */
Rpint GetLOF(char *path);
void PrintHelp(char *arg);
void ExitPrintUsage(char *arg);
void PrintRectFormat(void);
void QSortIofRpints(int begin,
                    int end,
                    Rpint value[],
                    int I[]);
static void ExchangeInts(int *x, int *y);
static Rpint IofRpintsMedian(Rpint v[],
                             int I[],
                             int *b,
                             int m,
                             int *e);
void ComputeWorld(File f, typinterval *world);
boolean CreateRtGetWorld(File f,
                         Rpint lof,
                         typinterval *world,
                         t_RT *RT, t_LRU *LRU,
                         boolean *isRTonDisk);
void ComputeWidths(typinterval *world,
                   int distGrowFac,
                   double cellScale,
                   double *quadCellWidth,
                   double *minQuadNNworldWidth);
void PrintRect(typinterval *r);
void PrintDimDoubles(double *l);
void Work(File origFile,
          File newFile,
          t_RT RT,
          double quadCellWidth,
          double minQuadNNworldWidth,
          int distGrowFac,
          DistCalcFunc distFunc,
          RectDistType distType);
boolean AlwaysTrue(t_RT R,
                   Rint NumbOfDim,
                   const typinterval *RSTrect,
                   const typinterval *qRect,
                   Rint qRectQty,
                   void *qPtr);
void CalcEucl(Rfloat coordDist, Rfloat *cumuDist);
void CalcLinf(Rfloat coordDist, Rfloat *cumuDist);
void NotExistsExit(int numbnbrs);
void CpRct(typinterval *srcRect, typinterval *targetRect);
void GetEdges(typinterval *rectArr, Rpint number, typinterval *rect);
void MakeCenteredMBS(double *center, typinterval *rect, double minWidth);
void MakeMBS(typinterval *rect, double minWidth);
void MakeAligned(Rint *numb, Rint alignm);

/***********************************************************************/
/* no free(), because called only once */

int main(int argc, char *argv[]) {

  Rpint lof, remain;
  char inFileName[MAX_NAME_LEN_IN];
  char outFileName[MAX_NAME_LEN_OUT];
  File inFile, outFile;
  typinterval *world;
  int distGrowFac, ac;
  t_RT RT; t_LRU LRU;
  boolean isRTonDisk /*, success */;
  double quadCellWidth, minQuadNNworldWidth;
  double cellScale;
  DistCalcFunc distFunc;
  RectDistType distType;
  char nameSuffix[MAX_SUFF_LEN];
  char s[160];
  
  if (argc == 2 && (strcmp(argv[1],"-help") == 0 || strcmp(argv[1],"-h") == 0)) {
    PrintHelp(argv[0]);
    exit(0);
  }
  else if (argc < 5 || argc > 7) {
    ExitPrintUsage(argv[0]);
  }
  
  /* get command line parameters, set direct dependents: */
  GVnumbDim= atoi(argv[1]);
  if (strlcpy(inFileName,argv[2],MAX_NAME_LEN_IN) >= MAX_NAME_LEN_IN) {
    fprintf(stderr,"%s: %s\n",argv[0],"ERROR: name to long");
    exit(2);
  }
  if (strlcpy(outFileName,argv[2],MAX_NAME_LEN_IN) >= MAX_NAME_LEN_IN) {
    fprintf(stderr,"%s: %s\n",argv[0],"ERROR: name to long");
    exit(2);
  }
  distGrowFac= atoi(argv[3]);
  cellScale= atof(argv[4]);
  
  /* set defaults for optional parameters: */
  distFunc= CalcLinf;
  distType= maxDist;
  if (argc > 5) {
    for (ac= 5; ac < argc; ac++) {
      if (strcmp(argv[ac],"-e") == 0) {
        distFunc= CalcEucl;
      }
      else if (strcmp(argv[ac],"-i") == 0) {
        distFunc= CalcLinf;
      }
      else if (strcmp(argv[ac],"-m") == 0) {
        distType= minDist;
      }
      else if (strcmp(argv[ac],"-M") == 0) {
        distType= maxDist;
      }
      else {
        ExitPrintUsage(argv[0]);
      }
    }
  }
  
  fprintf(stderr,"              Input file: %s\n",inFileName);
  fprintf(stderr,"    Number of dimensions: %d\n",GVnumbDim);
  fprintf(stderr,"Distribution grow factor: %d\n",distGrowFac);
  fprintf(stderr,"       Cell scale factor: %f\n",cellScale);
  lof= GetLOF(inFileName);
  if (lof == 0) {
    perror(inFileName);
    exit(2);
  }
  GVsizeRect= sizeof(typinterval) * GVnumbDim;
  GVinNumbRects= lof / GVsizeRect;
  remain= lof % GVsizeRect;
  fprintf(stderr,strans("Number of rectangles: %P\n",s),GVinNumbRects);
  if (remain != 0) {
    fprintf(stderr,"ERROR: ");
    fprintf(stderr,strans("Length (%s) = %P%s%P%s\n",s),inFileName,lof," % ",GVsizeRect," != 0");
    fprintf(stderr,strans("       Remainder: %P\n",s),remain);
    fprintf(stderr,"EXITING\n");
    exit(2);
  }
  if (distGrowFac < 1) {
    fprintf(stderr,"%s: %s\n",argv[0],"ERROR: distGrowFac < 1");
    exit(2);
  }
  if (distGrowFac > GVinNumbRects) {
    fprintf(stderr,"%s: %s\n",argv[0],"ERROR: distGrowFac > number of rectangles");
    exit(2);
  }
  if (cellScale < 0.0) {
    fprintf(stderr,"%s: %s\n",argv[0],"ERROR: cellScale < 0");
    exit(2);
  }
  
  if (! OpenRdOnlyF(inFileName,&inFile)) {
    exit(2);
  }
  
  world= malloc(sizeof(typinterval) * GVnumbDim);
  if (! CreateRtGetWorld(inFile,lof,world,&RT,&LRU,&isRTonDisk)) {
    fprintf(stderr,"ERROR exit: RR*-tree construction failed.\n");
    return 2;
  }
  ComputeWidths(world,distGrowFac,cellScale,
                &quadCellWidth,&minQuadNNworldWidth);
                
  /* start nameSuffix construction: */
  strcpy(nameSuffix,".NN");
  
  fprintf(stderr,"---------------------------------------------------------\n");
  fprintf(stderr,"\"Proximity\" algorithm\n");
  if (distFunc == CalcLinf) {
    fprintf(stderr,"Distance function: L infinity\n");
    strcat(nameSuffix,"i");
  } 
  else if (distFunc == CalcEucl) {
    fprintf(stderr,"Distance function: Euclidian\n");
    strcat(nameSuffix,"e");
  }
  else {
    fprintf(stderr,"FATAL INTERNAL ERROR: option disorder\n");
    exit(3);
  }
  if (distType == maxDist) {
    fprintf(stderr,"Distance type:     maximum distance\n");
    strcat(nameSuffix,"M");
  }
  else if (distType == minDist) {
    fprintf(stderr,"Distance type:     minimum distance\n");
    strcat(nameSuffix,"m");
  }
  else {
    fprintf(stderr,"FATAL INTERNAL ERROR: option disorder\n");
    exit(3);
  }
  fprintf(stderr,"---------------------------------------------------------\n");
  if (strlcat(outFileName,nameSuffix,MAX_NAME_LEN_OUT) >= MAX_NAME_LEN_OUT) {
    fprintf(stderr,"%s: %s\n",argv[0],"ERROR: name to long");
    exit(2);
  }
  fprintf(stderr,"Output file: %s\n",outFileName);
  if (! CreateTruncF(outFileName,&outFile)) {
    exit(2);
  }
  fprintf(stderr,"WORKING\n");
  Work(inFile,outFile,RT,
       quadCellWidth,minQuadNNworldWidth,distGrowFac,distFunc,distType);
  fprintf(stderr,"Parameters of %s:\n",outFileName);
  fprintf(stderr,strans("Number of rectangles: %P\n",s),GVinNumbRects * distGrowFac);
  if (close(inFile) == -1) {
    perror("Closing input file");
    exit(2);
  }
  if (close(outFile) == -1) {
    perror("Closing output file");
    exit(2);
  }
  if (isRTonDisk) {
    //CloseRST(&RT); /* not needed: RT temp., result file written */
    DisposeLRU(&LRU);
    TryRemoveRST(tmpName);
  }
  else {
    //SaveLRU(LRU,&success); /* not needed: RT temp., result file written */
    RemoveMainMemRST(&RT);
  }
  return 0;
}

/***********************************************************************/
/* no free(), because called only once */

void Work(File origFile,
          File newFile,
          t_RT RT,
          double quadCellWidth,
          double minQuadNNworldWidth,
          int distGrowFac,
          DistCalcFunc distFunc,
          RectDistType distType) {

  t_DQ distQ;
  Rpint i, j, nbytes;
  int d;
  typinterval *origRect= malloc(GVsizeRect);
  typinterval *NNworld= malloc(GVsizeRect);
  Rfloat *origRectCent= malloc(GVnumbDim * sizeof(Rfloat));
  char perrorStr[80];
  int infoSize= sizeof(Rpint);
  refinfo info= malloc(infoSize);
  Rpint *RpintPtr;
  Rfloat rawDist;
  boolean exists;
  typinterval *rectArr= malloc(distGrowFac * GVnumbDim * sizeof(typinterval));
  Rpint *rectNumbArr= malloc(distGrowFac * sizeof(Rpint));
  int *I= malloc(distGrowFac * sizeof(int));
  double cellOriginD, NNworldOriginD, scaleD, NNworldWidth;
  typinterval *inRect; /* only used as pointer */
  typinterval *newRect= malloc(GVsizeRect);
  char s[160];
  
  InitDistQueryIdent(&distQ);
  for (i= 0; i < GVinNumbRects; i++) {
    nbytes= read(origFile,origRect,GVsizeRect);
    if (nbytes != GVsizeRect) {
      sprintf(perrorStr,strans("Work(read #%P)",s),i);
      perror(perrorStr);
      fprintf(stderr,strans("bytes requested: %P, bytes read %P\n",s),GVsizeRect,nbytes);
      exit(2);
    }
    for (d= 0; d < GVnumbDim; d++) {
      origRectCent[d]= 0.5 * (origRect[d].l + origRect[d].h);
    }
    if (! NewDistQuery(RT,origRectCent,
                       distFunc,inc,distType,0.0,
                       NULL,0,NULL,AlwaysTrue,AlwaysTrue,
                       8192,FALSE,
                       &distQ)) {
      fprintf(stderr,"RR*-tree.NewDistQuery: ");
      fprintf(stderr,"FAILED, EXITING.\n");
      exit(2);
    }
    /* collect distGrowFac rectangles: */
    for (j= 0; j < distGrowFac; j++) {
      I[j]= j;
      if (GetDistQueryRec(distQ,RT,&rectArr[j * GVnumbDim],info,&rawDist,&exists)) {
        if (exists) {
          /* rectangle stored in rectArr now */
          /* get insertion count: */
          RpintPtr= (Rpint *)&(*info).c;
          rectNumbArr[j]= *RpintPtr;
        }
        else {
          NotExistsExit(j + 1);
        }
      }
    }
    /* rectangles through DistQuery sorted by proximity */
    DisposeDistQuery(&distQ);
    /* determine the outer edges of the nearest neighbors world: */
    GetEdges(rectArr,distGrowFac,NNworld);
    /* let NNworld a CELL centered square with a minimum width: */
    //MakeCenteredMBS(origRectCent,NNworld,minQuadNNworldWidth);
    /* let NNworld a SELF centered square with a minimum width: */
    MakeMBS(NNworld,minQuadNNworldWidth);
    /* sort index over rectangles by insertion order (info): */
    QSortIofRpints(0,distGrowFac - 1,rectNumbArr,I);
    /* map inRect from NNworld to cell, cell center = center(orig. rect.): */
    for (j= 0; j < distGrowFac; j++) {
      inRect= &rectArr[I[j] * GVnumbDim];
      for (d= 0; d < GVnumbDim; d++) {
        NNworldOriginD= NNworld[d].l;
        NNworldWidth= NNworld[d].h - NNworld[d].l;
        scaleD= quadCellWidth / NNworldWidth;
        cellOriginD= origRectCent[d] - 0.5 * quadCellWidth;
        newRect[d].l= cellOriginD + scaleD * (inRect[d].l - NNworldOriginD);
        newRect[d].h= cellOriginD + scaleD * (inRect[d].h - NNworldOriginD);
      }
      nbytes= write(newFile,newRect,GVsizeRect);
      if (nbytes != GVsizeRect) {
        sprintf(perrorStr,strans("Work(write #%P)",s),i * j + j);
        perror(perrorStr);
        fprintf(stderr,strans("bytes to be written: %P, bytes written %P\n",s),GVsizeRect,nbytes);
        exit(2);
      }
    }
  }
}

/***********************************************************************/
/* no free(), because called only once */

void ComputeWidths(typinterval *world,
                   int distGrowFac,
                   double cellScale,
                   double *quadCellWidth,
                   double *minQuadNNworldWidth) {

  double *worldWidths= malloc(sizeof(double) * GVnumbDim);
  double *worldCenter= malloc(sizeof(double) * GVnumbDim);
  typinterval *quadWorld= malloc(sizeof(typinterval) * GVnumbDim);
  double quadWorldWidth, worldVolume;
  int d;
  
  d= 0; worldVolume= 1.0;
  worldWidths[d]= world[d].h - world[d].l;
  quadWorldWidth= worldWidths[d];
  worldVolume*= worldWidths[d];
  worldCenter[d]= 0.5 * (world[d].l + world[d].h);
  for (d= 1; d < GVnumbDim; d++) {
    worldWidths[d]= world[d].h - world[d].l;
    if (worldWidths[d] > quadWorldWidth) {
      quadWorldWidth= worldWidths[d];
    }
    worldVolume*= worldWidths[d];
    worldCenter[d]= 0.5 * (world[d].l + world[d].h);
  }
  for (d= 0; d < GVnumbDim; d++) {
    quadWorld[d].l= worldCenter[d] - 0.5 * quadWorldWidth;
    quadWorld[d].h= worldCenter[d] + 0.5 * quadWorldWidth;
  }
  /* Width of a quadratic cell corresponding to the quadratic (made) world: */
  *quadCellWidth= quadWorldWidth / pow(GVinNumbRects,1.0 / GVnumbDim);
  /* Scale quadCellWidth by cellScale, i.e. reduce the area, groups of objects
     will cover, to reduce overlap, possibly accepting holes: 
     (cellScale > 1 does not make much sense, though allowed) */
  *quadCellWidth*= cellScale;
  /* Let the NNworld, containing distGrowFac neighbors, have at least a width,
     corresponding to distGrowFac cells: */
  *minQuadNNworldWidth= quadWorldWidth / pow((double)GVinNumbRects / distGrowFac,1.0 / GVnumbDim);
  
  fprintf(stderr,"----- original World Bounds: -----\n");
  PrintRect(world);
  fprintf(stderr,"----- original World Widths: -----\n");
  PrintDimDoubles(worldWidths);
  fprintf(stderr,"----- original World Volume: %e\n",worldVolume);
  fprintf(stderr,"\n");
  fprintf(stderr,"----- Minimum Bounding Square of a cell: %e\n",*quadCellWidth);
  fprintf(stderr,"----- Volume of the MBS of a cell: %e\n",pow(*quadCellWidth,GVnumbDim));
  fprintf(stderr,"\n");
  fprintf(stderr,"----- MBS of the original world: -----\n");
  PrintRect(quadWorld);
}

/***********************************************************************/
/* no free(), because called only once */
/* typatomkey = Rfloat must hold! */

boolean CreateRtGetWorld(File f,
                         Rpint lof,
                         typinterval *world,
                         t_RT *RT, t_LRU *LRU,
                         boolean *isRTonDisk) {

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
  Rpint dirRAMSize, dataRAMSize;
  Rint nomPageSize, pageGrnlMult, pageSize;
  Rpnint LRUcap;
  Rint sRF= sizeof(Rfloat);
  Rint infoSize= sizeof(Rpint);
  Rint entrySize= GVnumbDim * 2 * sRF + infoSize;
  refinfo info= malloc(infoSize);
  refinfo infoStored= malloc(infoSize);
  Rint UNUSED;
  Rpint *RpintPtr;
  Rpint nbytes, pos, p, i;
  boolean inserted, success;
  typinterval *inRect= malloc(GVsizeRect);
  char s[160];
  
  MakeAligned(&entrySize,sizeof(typatomkey));
  
  InitRSTreeIdent(RT);
  if (lof < MAX_MAIN_MEM_USE / N_E_M_DATA_FAC) {
    *isRTonDisk= FALSE;
    /* page size calculation: */
    nomPageSize= sRF + GVnumbDim * sRF + N_ENT_MAIN * entrySize;
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
    if (CreateMainMemRST(RT,pageSize,pageSize,
                         GVnumbDim,infoSize,FALSE,
                         dirRAMSize,dataRAMSize,TRUE)) {
      fprintf(stderr,"Done.\n");
    }
    else {
      fprintf(stderr,"FAILED.\n");
      return FALSE;
    }
  }
  else {
    *isRTonDisk= TRUE;
    /* page size calculation: */
    nomPageSize= sRF + GVnumbDim * sRF + N_ENT_SEC * entrySize;
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
    if (CreateRST(tmpName,pageSize,pageSize,GVnumbDim,infoSize,FALSE)) {
      fprintf(stderr,"Done.\n");
    }
    else{
      fprintf(stderr,"FAILED.\n");
      return FALSE;
    }
    
    /* LRU buffer capacity calculation in pages: */
    LRUcap= MAX_MAIN_MEM_USE / pageSize;
    
    fprintf(stderr,"LRU.NewLRU: ");
    success= TRUE;
    NewLRU(LRU,LRUcap,pageSize,&success);
    if (success) {
      fprintf(stderr,"Done.\n");
    }
    else{
      fprintf(stderr,"FAILED.\n");
      return FALSE;
    }
    fprintf(stderr,"RR*-tree.OpenBufferedRST: %s ",tmpName);
    if (OpenBufferedRST(RT,tmpName,*LRU)) {
      fprintf(stderr,"Done.\n");
    }
    else{
      fprintf(stderr,"FAILED.\n");
      return FALSE;
    }
  }
  
  fprintf(stderr,"RR*-tree: insertion running:\n");
  for (i= 0; i < GVinNumbRects; i++) {
    nbytes= read(f,inRect,GVsizeRect);
    if (nbytes != GVsizeRect) {
      perror("CreateRtWorld(read input file)");
      fprintf(stderr,strans("bytes requested: %P, bytes read %P\n",s),GVsizeRect,nbytes);
      return FALSE;
    }
    /* number rectangle to be recognized if necessary */
    RpintPtr= (Rpint *)(*info).c;
    *RpintPtr= i;
    /* then insert the rectangle */
    success= InsertRecord(*RT,inRect,info,&inserted,infoStored);
    if (! success) {
      fprintf(stderr,"FAILURE(RR*-tree.InsertRecord)\n");
      return FALSE;
    }
  }
  fprintf(stderr,"Done.\n");
  
  GetRootMBB(*RT,&UNUSED,world);
  
  /* reset file pointer to 0 (reset): */
  pos= 0;
  p= lseek(f,pos,SEEK_SET); /* manual: returns pos or -1 */
  if (p != pos) {
    perror("CreateRtGetWorld,lseek: reset failed");
    exit(2);
  }
  
  return TRUE;
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

boolean AlwaysTrue(t_RT R,
                   Rint NumbOfDim,
                   const typinterval *RSTrect,
                   const typinterval *qRect,
                   Rint qRectQty,
                   void *qPtr) {

  return TRUE;
}

/***********************************************************************/

void CalcEucl (Rfloat coordDist, Rfloat *cumuDist)

{
  *cumuDist+= coordDist * coordDist;
}

/***********************************************************************/

void CalcLinf (Rfloat coordDist, Rfloat *cumuDist)

{
  if (fabs(coordDist) > *cumuDist) {
    *cumuDist= fabs(coordDist);
  }
}

/***********************************************************************/

void NotExistsExit(int numbnbrs)

{
  fprintf(stderr,"RR*-tree.GetDistQueryRec:\n");
  fprintf(stderr,"Query exhausted! distribution contains less than %d rectangles.\n",numbnbrs);
  fprintf(stderr,"FAILURE, EXITING\n");
  exit(2);
}

/***********************************************************************/

void CpRct(typinterval *srcRect, typinterval *targetRect) {

  int d;
  
  for (d= 0; d < GVnumbDim; d++) {
    targetRect[d].l= srcRect[d].l;
    targetRect[d].h= srcRect[d].h;
  }
}

/***********************************************************************/

void GetEdges(typinterval *rectArr, Rpint number, typinterval *rect) {

  int d;
  Rpint i;
  
  for (d= 0; d < GVnumbDim; d++) {
    rect[d].l= rectArr[d].l;
    rect[d].h= rectArr[d].h;
  }
  rectArr+= GVnumbDim; /* next rectangle */
  for (i= 1; i < number; i++) {
    for (d= 0; d < GVnumbDim; d++) {
      if (rectArr[d].l < rect[d].l) {
        rect[d].l= rectArr[d].l;
      }
      if (rectArr[d].h > rect[d].h) {
        rect[d].h= rectArr[d].h;
      }
    }
    rectArr+= GVnumbDim; /* next rectangle */
  }
}

/***********************************************************************/
/* Modify rect, to be the minimum bounding square around center,
   the minimum width being minWidth. */

void MakeCenteredMBS(double *center, typinterval *rect, double minWidth) {

  int d;
  double centEdgeDist, maxCentEdgeDist;
  
  maxCentEdgeDist= 0.5 * minWidth;
  for (d= 0; d < GVnumbDim; d++) {
    centEdgeDist= fabs(rect[d].l - center[d]);
    if (centEdgeDist > maxCentEdgeDist) {
      maxCentEdgeDist= centEdgeDist;
    } 
    centEdgeDist= fabs(rect[d].h - center[d]);
    if (centEdgeDist > maxCentEdgeDist) {
      maxCentEdgeDist= centEdgeDist;
    } 
  }
  for (d= 0; d < GVnumbDim; d++) {
    rect[d].l= center[d] - maxCentEdgeDist;
    rect[d].h= center[d] + maxCentEdgeDist;
  }
}

/***********************************************************************/
/* Modify rect, to be a minimum bounding square, the minimum width
   being minWidth. */

void MakeMBS(typinterval *rect, double minWidth) {

  double *center= malloc(GVnumbDim * sizeof(double));
  double width, maxWidth;
  int d;
  
  for (d= 0; d < GVnumbDim; d++) {
    center[d]= 0.5 * (rect[d].l + rect[d].h);
  }
  maxWidth= minWidth;
  for (d= 0; d < GVnumbDim; d++) {
    width= rect[d].h - rect[d].l;
    if (width > maxWidth) {
      maxWidth= width;
    }
  }
  for (d= 0; d < GVnumbDim; d++) {
    rect[d].l= center[d] - 0.5 * maxWidth;
    rect[d].h= center[d] + 0.5 * maxWidth;
  }
  free(center);
}

/***********************************************************************/

void PrintRect(typinterval *r) {

  int d;
  
  for (d= 0; d < GVnumbDim; d++) {
    fprintf(stderr,"  %f %f",r[d].l,r[d].h);
  }
  fprintf(stderr,"\n");
}

/***********************************************************************/

void PrintDimDoubles(double *l) {

  int d;
  
  for (d= 0; d < GVnumbDim; d++) {
    fprintf(stderr,"  %f",l[d]);
  }
  fprintf(stderr,"\n");
}

/***********************************************************************/

void PrintHelp(char *arg) {

  fprintf(stdout,
"SYNOPSYS\n");
  fprintf(stdout,"%s%s%s",
"     Usage: ",arg," -help | -h\n");
  fprintf(stdout,"%s%s%s",
"            ",arg," NumbOfDim inputfile distGrowFac cellScale [-e | -i] [-M | -m]\n");
  fprintf(stdout,
"  distGrowFac: natural number > 0\n");
  fprintf(stdout,
"  cellScale:   real number >= 0\n");
  fprintf(stdout,"\n");
  fprintf(stdout,"%s%s",
arg," works on files, containing rectangles of the format:\n");
PrintRectFormat();
  fprintf(stdout,"%s",
"The original inputfile is kept unmodified and a new file, suffixed with\n");
  fprintf(stdout,"%s",
"\".NN??\" is created.\n");
  fprintf(stdout,"%s",
"\n");
  fprintf(stdout,"%s%s",
arg," multiplies the number of rectangles (NR) of inputfile by\n");
  fprintf(stdout,"%s",
"distGrowFac as follows:\n");
  fprintf(stdout,"%s",
"Each original rectangle is replaced by the distGrowFac nearest neighbors\n");
  fprintf(stdout,"%s",
"of the center of the original rectangle. Geometrically: The minimal\n");
  fprintf(stdout,"%s",
"bounding multidimensional square of the NN environment is projected into\n");
  fprintf(stdout,"%s",
"a cell sized square around the center of the original rectangle.\n");
  fprintf(stdout,"%s",
"For the width (CW) of a cell applies:\n");
  fprintf(stdout,"%s",
"CW = cellScale * worldWidth / pow(NR, 1.0/NumbOfDim),\n");
  fprintf(stdout,"%s",
"where cellScale is a parameter (see Usage), and worldWidth is the width of\n");
  fprintf(stdout,"%s",
"the quadratic (made) world.\n");
  fprintf(stdout,"%s",
"Obviously, if cellScale = 1, the width of a cell corresponds to a cell\n");
  fprintf(stdout,"%s",
"volume (CV), where (NR * CV) results in the total volume of the quadratic\n");
  fprintf(stdout,"%s",
"(made) world.\n");
  fprintf(stdout,"%s",
"Settings of cellScale less than 1 lead to a reduced size of the projected\n");
  fprintf(stdout,"%s",
"areas in the output. This can be utilized to alleviate or even prevent the\n");
  fprintf(stdout,"%s",
"increase of overlap, but on the other hand leads to empty space in the\n");
  fprintf(stdout,"%s",
"projection. Settings of cellScale > 1 will be accepted, though not making \n");
  fprintf(stdout,"%s",
"much sense. \n");
  fprintf(stdout,"%s",
"Important properties:\n");
  fprintf(stdout,"%s",
"  The local input order of the projected cutout is preserved.\n");
  fprintf(stdout,"%s",
"\n");
  fprintf(stdout,"%s",
"Options:\n");
  fprintf(stdout,"%s",
"  -e  Find the neighbors in the Euclidian environment (a circle)\n");
  fprintf(stdout,"%s",
"  -i  Find the neighbors in the L infinity environment (a rectangle)\n");
  fprintf(stdout,"%s",
"  -M  Proximity is defined by the distance to the farthest corner\n");
  fprintf(stdout,"%s",
"  -m  Proximity is defined by the distance to the nearest edge\n");
  fprintf(stdout,"%s",
"  NOTE that the default is \"-i -M\"\n");
  fprintf(stdout,"%s",
"The suffix of the newly created file is extended by the chosen (defaulted)\n");
  fprintf(stdout,"%s",
"options, such that the default suffix is \".NNiM\".\n");
  fprintf(stdout,"%s",
"\n");
  fprintf(stdout,"%s",
"Possible disadvantages concerning real rectangles:\n");
  fprintf(stdout,"%s",
"- overlap normally grows\n");
  fprintf(stdout,"%s",
"- this may be reduced for the cost of introducing empty space\n");
  fprintf(stdout,"%s",
"- the algorithm produces clusters of distGrowFac objects.\n");
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

void ExitPrintUsage(char *arg) {
  fprintf(stderr,"%s%s%s",
"Usage: ",arg," -help | -h\n");
  fprintf(stderr,"%s%s%s",
"       ",arg," NumbOfDim inputfile distGrowFac cellScale [-e | -i] [-M | -m]\n");
  fprintf(stderr,
"  distGrowFac: natural number > 0\n");
  fprintf(stderr,
"  cellScale:   real number >= 0\n");
  exit(1);
}

/***********************************************************************/

void PrintRectFormat(void) {

  fprintf(stdout,"struct {double l, h;} rectangle[NumbOfDim].\n");
}

/***********************************************************************/

static void ExchangeInts(int *x, int *y)

{
  int z;
  
  z= *x; *x= *y; *y= z;
}

/***********************************************************************/

void QSortIofRpints(int begin,
                    int end,
                    Rpint value[],
                    int I[])
/* Sorts I
   by value[I[i]]. */

{
  Rpint midelem;
  int i, j;
  
  i= begin; j= end;
  /* midelem= value[I[(i+j) >> 1]]; */
  /* profitable for #elements > 7: */
  midelem= IofRpintsMedian(value,I,&i,(i+j) >> 1,&j);
  if (i < j) {
    do {
      while (value[I[i]] < midelem) {
        i++;
      }
      while (value[I[j]] > midelem) {
        j--;
      }
      if (i < j) {
        ExchangeInts(&I[i],&I[j]);
        i++; j--;
      }
      else if (i == j) {
        i++; j--;
      }
    } while (i <= j);
    if (begin < j) {
      if (j - begin > 1) {
        QSortIofRpints(begin,j,value,I);
      }
      else {
        if (value[I[begin]] > value[I[j]]) {
          ExchangeInts(&I[begin],&I[j]);
        }
      }
    }
    if (i < end) {
      if (end - i > 1) {
        QSortIofRpints(i,end,value,I);
      }
      else {
        if (value[I[i]] > value[I[end]]) {
          ExchangeInts(&I[i],&I[end]);
        }
      }
    }
  }
}

/***********************************************************************/

static Rpint IofRpintsMedian(Rpint v[],
                             int I[],
                             int *b,
                             int m,
                             int *e)

{
  if (v[I[*b]] <= v[I[m]]) {
    if (v[I[m]] <= v[I[*e]]) {           //bme (sorted)
    }
    else {
      if (v[I[*b]] <= v[I[*e]]) {        //bem
        ExchangeInts(&I[m],&I[*e]);
      }
      else {                             //ebm  if (b == m) { x(b,e) } else ...
        ExchangeInts(&I[m],&I[*e]);      //     optimizes #exchanges
        ExchangeInts(&I[*b],&I[m]);
      }
    }
  }
  else {
    if (v[I[*b]] <= v[I[*e]]) {          //mbe
      ExchangeInts(&I[*b],&I[m]);
    }
    else {
      if (v[I[m]] < v[I[*e]]) {          //meb (NOTE the "<"!!)
        ExchangeInts(&I[*b],&I[m]);
        ExchangeInts(&I[m],&I[*e]);
      }
      else {                             //emb
        ExchangeInts(&I[*b],&I[*e]);
      }
    }
  }
  (*b)++; (*e)--;
  return v[I[m]];
}

/***********************************************************************/

void MakeAligned(Rint *numb, Rint alignm) {

  Rint ill= *numb & (alignm - 1);
  if (ill != 0) {
    (*numb)+= alignm - ill;
  }
}

/************************************************************************/
