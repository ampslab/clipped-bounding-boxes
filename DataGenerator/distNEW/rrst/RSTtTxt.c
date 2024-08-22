/* -----  RSTtTxt.c  ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//

#include <errno.h>
#include <unistd.h>

#include "RSTFileAccess.h"
#include "RSTMemAlloc.h"

#include "RSTOtherFuncs.h"
#include "RSTDistQueryFuncs.h"
#include "RSTErrors.h"
/* the other RSTree interfaces are included implicitly */


/* constants */

#define RST_SUFF ".RSF"
#define FIXED_INFO_PART 42
#define ASCII_SUFF ".RSF.ASC"
#define FB_N_RECS 200


/* types */

typedef struct {
               File file1, file2;
               } typfildes, *reffildes;
typedef struct {
               File f;
               refinfo info;
               boolean qstart;
               boolean recordMatch;
               } typHandleRec, *refHandleRec;


/* declarations */

boolean NoTreeInUse(void);
boolean JoinCalled(boolean *externJoin);
   void DataAndTreeName(void);
   void GetAndPrintParamsForUsage(t_RT R);
   void SecTreeName(void);
   void QueryName(void);
   void DisplErr(char *name, int recNumb);
   void DisplBufFillInfo(char *name, int recNumb);
boolean opendata(void);
   void closedata(void);
boolean openquery(void);
   void closequery(void);
   void exeCreate(void);
   void exeRemove(void);
   void exeMainCreate(void);
   void exeRemoveMain(void);
boolean exeOpen(char *name, t_RT *r);
boolean exeOpenBuffered(char *name,
                        t_RT *r,
                        t_LRU *lru,
                        boolean isJoin,
                        int otherDirPsize,
                        int otherDataPsize);
boolean exeClose(void);
void exeSyncRST(void);
void exeGetRSTSync(void);
boolean exeLoad(char *name, t_RT *r);
boolean exeSave(void);
boolean GetTestParameters(FILE *file,
                          char *name);
   void exeInsert(void);
   void exeDelete(void);
   void exeExistsRegion(void);
   void exeExactMatchCount(void);
   void exeExactMatchQuery(void);
   void exeRegionCount(void);
   void exeRegionQuery(void);
   void exeXJoinCount(void);
   void exeXJoin(void);
   void exeSpatialJoinCount(void);
   void exeSpatialJoin(void);
   void exeNNearestTest(void);
   void exeDistanceQuery(void);
boolean exeCheckConsistency(boolean verbose);
   void exeASCIIdump(void);
   void exeDirLevelDump(void);
   void exeMediaReorg(void);
   void exeInquire(void);
   void exeExamDescFile(void);
   void exeComputePageSizes(void);
   void exePathsDump(void);
   void exeWriteSalvagedP(void);
   void exeDumpSalvagedP(void);
   void exeRecovFromSalvDump(void);
   void ShortInstruction(void);
   void CalcEucl (Rfloat coordDist, Rfloat *cumuDist);
   void ReadDistQParams(typcoord *qpPtr,
                        double *startDist,
                        double *endDist,
                        typinterval *clipRectPtr,
                        Rlint *number);
boolean infoEqual(t_RT rt,
                  const typinfo *stored,
                  Rint infoSize,
                  const typinfo *searched,
                  void *ptr);
boolean DirEqual(t_RT rt,
                 Rint numbOfDim,
                 const typinterval *RSTrect,
                 const typinterval *qRects,
                 Rint qRectQty,
                 void *qPtr);
boolean DataEqual(t_RT rt,
                  Rint numbOfDim,
                  const typinterval *RSTrect,
                  const typinterval *qRects,
                  Rint qRectQty,
                  void *qPtr);
boolean DirIntersects(t_RT rt,
                      Rint numbOfDim,
                      const typinterval *RSTrect,
                      const typinterval *qRects,
                      Rint qRectQty,
                      void *qPtr);
boolean DataIntersects(t_RT rt,
                       Rint numbOfDim,
                       const typinterval *RSTrect,
                       const typinterval *qRects,
                       Rint qRectQty,
                       void *qPtr);
boolean DirEncloses(t_RT rt,
                    Rint numbOfDim,
                    const typinterval *RSTrect,
                    const typinterval *qRects,
                    Rint qRectQty,
                    void *qPtr);
boolean DataEncloses(t_RT rt,
                     Rint numbOfDim,
                     const typinterval *RSTrect,
                     const typinterval *qRects,
                     Rint qRectQty,
                     void *qPtr);
boolean DirIsContained(t_RT rt,
                       Rint numbOfDim,
                       const typinterval *RSTrect,
                       const typinterval *qRects,
                       Rint qRectQty,
                       void *qPtr);
boolean DataIsContained(t_RT rt,
                        Rint numbOfDim,
                        const typinterval *RSTrect,
                        const typinterval *qRects,
                        Rint qRectQty,
                        void *qPtr);
boolean AlwaysTrue(t_RT rt,
                   Rint numbOfDim,
                   const typinterval *RSTrect,
                   const typinterval *qRects,
                   Rint qRectQty,
                   void *qPtr);
boolean J_DirIntersects(t_RT rt1,
                        t_RT rt2,
                        Rint numbOfDim,
                        const typinterval *RST1rect,
                        const typinterval *RST2rect);
boolean J_DataIntersects(t_RT rt1,
                         t_RT rt2,
                         Rint numbOfDim,
                         const typinterval *RST1rect,
                         const typinterval *RST2rect);
void GVCompCount0(void);
void ManageEMQuery(t_RT R,
                   Rint numbOfDim,
                   const typinterval *rectangle,
                   refinfo infoptr,
                   Rint infoSize,
                   void *buf,
                   boolean *modify,
                   boolean *finish);
void ManageRegQuery(t_RT R,
                    Rint numbOfDim,
                    const typinterval *rectangle,
                    refinfo infoptr,
                    Rint infoSize,
                    void *buf,
                    boolean *modify,
                    boolean *finish);
void ManageJoin(t_RT R1, t_RT R2,
                Rint numbOfDim,
                const typinterval *rectangle1, const typinterval *rectangle2,
                refinfo info1ptr, refinfo info2ptr,
                Rint info1Size, Rint info2Size,
                void *buf,
                boolean *finish);
void ConvRectForDump(t_RT R,
                     Rint numbOfDim,
                     const typinterval *rstrect,
                     void *buf,
                     Rint bufSize);
void PrintDistQueryRec(typinterval *rect,
                       refinfo info,
                       Rfloat rawDist,
                       char distName[]);
void PrintFounds(Rlint min, Rlint max, double sum, double sumsqr);
void PrintPer(double value, double div, char *tvalue, char *tdiv);
void PrintReads(t_RT R, Rlint found);
void PrintJoinedReads(t_RT Rmain, t_RT Rsec, Rlint found);
void PrintAdminReads(t_RT R);
void PrintWrites(t_RT R, Rlint found);
void PrintAdminWrites(t_RT R);
void PrintAccesses(t_RT R, Rlint found);
void PrintOverUnder(t_RT R);
void PrintChooseSubtree(t_RT R);
void PrintCompares(t_RT R, Rlint found);
void PrintGVCompares(Rlint found);
void PrintPQelems(t_RT R, Rlint nbrs);
void PrintTimes(t_RT R);
void PrintTimes2fold(t_RT R);
void PrintCoverage(t_RT R);
void PrintRootMBB(t_RT R);
void PrintCalls(int numbCalls);
void PrintIndHeight(t_RT R, int i);
void PrintIndHeightFounds(t_RT R, int i, Rlint found);
void PrintIndHeightMatches(t_RT R, int i, Rlint match);
void PrintRect(typinterval *r);
void PrintPoint(typcoord *p);
void PrintLRUStatistics(t_LRU lru);
int GetChar(void);
void StartTimers(void);
void StopTimers(void);
boolean RdTextToRect(FILE *stream,
                     typinterval *r);


/* global variables local to this file, standard: prefix: GV */
Rint GVnumbOfDim, GVrectSize, GVpointSize, GVinfoSize;
char GVdistName[MaxNameLength];
char GVquerName[MaxNameLength];
char GV_RfileName[MaxNameLength], GV_RfileName2[MaxNameLength];
char GVdummyCh;
FILE *GVdataFile, *GVquerFile;
int GVbegin, GVend, GVNumbCalls, GVfdBckMod;
/* restricts the number of entries of the tree to MAXINT,
   avoids %-operation on Rlint/Rpint (see below),
   see also variables i in the different functions.
*/
boolean GVdoneMsg, GVnotDmsg;
boolean GVisBuffered; /* TRUE: set dir-/date-specific R/W output to 0 */
t_LRU GVlru, GVlru2;
t_RT GVrst, GVrst2;
Rlint GVdirComparisons, GVdataComparisons;
Rlint GVnumFound;
Rlint GVpairCount;
/*** extra for reading in data as text: */
char *GVstrPtr; 

/* global variables local to this file, string buffer for strans, */
/* should be short: single character, no prefix: */
char s[256];
/* "s" MUST NOT BE DECLARED LOCALLY!! */

/***********************************************************************/

int main(int argc, char *argv[])

{
  char mainchoose, choose;
  boolean WorkMenu, extJoin;
  int otherDirPsize, otherDataPsize;
  
  if (argc > 1) {
    if (argc > 2 || strcmp(argv[1],"-v") != 0) {
      printf("Usage:  %s [-v]\n",argv[0]);
      exit(1);
    }
    else {
      PrintRSTStdTypesConsts();
      printf("\n");
      PrintRSTImplLimits();
      printf("\n");
      PrintErrMessNumbs();
      printf("\n");
    }
  }
  GVlru= NULL;
  GVlru2= NULL;
  InitRSTreeIdent(&GVrst);
  InitRSTreeIdent(&GVrst2);
  GVisBuffered= FALSE; /* initialize flag for R/W counts output */
  GVstrPtr= NULL; // important: NULL --> GVstrPtr is allocated by getline
  ShortInstruction();
  do {
    printf("\n");
    printf("## %14s,   %25s,   %25s,\n","Create = \"c\"","Create buffered = \"C\"","MainCreate = \"m\"");
    printf("## %14s,   %25s,   %25s,\n","Open = \"o\"","Open buffered = \"O\"","Load = \"l\"");
    printf("## %14s,   %25s    %25s,\n","Close = \".\"","","Save = \"s\"");
    printf("## %28s    %40s,\n","Remove = \"R\"","MainKill = \"K\"");
    printf("## %28s,\n","Synchronize tree = \"S\"");
    printf("## %28s,\n","Get Synchronization = \"G\"");
    printf("## %36s,\n","Inquire Description = \"I\"");
    printf("## %36s,\n","Examine stored Description = \"E\"");
    printf("## %36s,\n","Compute page sizes = \"`\"");
    printf("## %36s,\n","leave this menu = \"-\"");
    printf("## %36s.\n","quit = \"q\"");
    printf("Choice: ");
    do {
      mainchoose= GetChar();
    } while ((mainchoose != 'c') && (mainchoose != 'C') && (mainchoose != 'm') &&
             (mainchoose != 'o') && (mainchoose != 'O') && (mainchoose != 'l') &&
             (mainchoose != '.') && (mainchoose != 's') &&
             (mainchoose != 'R') && (mainchoose != 'K') &&
             (mainchoose != 'S') &&
             (mainchoose != 'G') &&
             (mainchoose != 'I') &&
             (mainchoose != 'E') &&
             (mainchoose != '`') &&
             (mainchoose != '-') &&
             (mainchoose != 'q'));
    GVdummyCh= GetChar();
    WorkMenu= TRUE;
    if (mainchoose != 'q') {
      if (mainchoose == '-') {
      }
      else if (mainchoose == 'c') {
        if (NoTreeInUse()) {
          DataAndTreeName();
          exeCreate();
          exeOpen(GV_RfileName,&GVrst);
        }
        else {
          WorkMenu= FALSE;
        }
      }
      else if (mainchoose == 'C') {
        if (NoTreeInUse()) {
          DataAndTreeName();
          exeCreate();
          exeOpenBuffered(GV_RfileName,&GVrst,&GVlru,FALSE,0,0);
        }
        else {
          WorkMenu= FALSE;
        }
      }
      else if (mainchoose == 'm') {
        if (NoTreeInUse()) {
          DataAndTreeName();
          exeMainCreate();
        }
        else {
          WorkMenu= FALSE;
        }
      }
      else if (mainchoose == 'o') {
        if (NoTreeInUse()) {
          DataAndTreeName();
          if (exeOpen(GV_RfileName,&GVrst)) {
            if (JoinCalled(&extJoin)) {
              if (extJoin) {
                exeOpen(GV_RfileName2,&GVrst2);
              }
            }
          }
        }
        else {
          WorkMenu= FALSE;
        }
      }
      else if (mainchoose == 'O') {
        if (NoTreeInUse()) {
          DataAndTreeName();
          if (exeOpenBuffered(GV_RfileName,&GVrst,&GVlru,FALSE,0,0)) {
            if (JoinCalled(&extJoin)) {
              if (extJoin) {
                GetPageSizes(GVrst,&otherDirPsize,&otherDataPsize);
                exeOpenBuffered(GV_RfileName2,&GVrst2,&GVlru2,TRUE,otherDirPsize,otherDataPsize);
              }
            }
          }
        }
        else {
          WorkMenu= FALSE;
        }
      }
      else if (mainchoose == 'l') {
        if (NoTreeInUse()) {
          DataAndTreeName();
          if (exeLoad(GV_RfileName,&GVrst)) {
            if (JoinCalled(&extJoin)) {
              if (extJoin) {
                exeLoad(GV_RfileName2,&GVrst2);
              }
            }
          }
        }
        else {
          WorkMenu= FALSE;
        }
      }
      else if (mainchoose == '.') {
        exeClose();
        WorkMenu= FALSE;
      }
      else if (mainchoose == 's') {
        exeSave();
        WorkMenu= FALSE;
      }
      else if (mainchoose == 'R') {
        exeRemove();
        WorkMenu= FALSE;
      }
      else if (mainchoose == 'K') {
        exeRemoveMain();
        WorkMenu= FALSE;
      }
      else if (mainchoose == 'I') {
        exeInquire();
      }
      else if (mainchoose == 'E') {
        exeExamDescFile();
      }
      else if (mainchoose == 'S') {
        exeSyncRST();
      }
      else if (mainchoose == 'G') {
        exeGetRSTSync();
      }
      else if (mainchoose == '`') {
        exeComputePageSizes();
        WorkMenu= FALSE;
      }
      if (WorkMenu) {
        GVbegin= 0; GVend= 0; /* initialization for the PrintXxx functions */
        printf("\n");
        printf("## %25s,   %25s,\n","Insert = \"i\"","Delete = \"d\"");
        printf("## %36s,\n","ExistsRegion = \"x\"");
        printf("## %25s,   %25s,\n","ExactMatchCount = \"e\"","ExactMatchQuery = \"E\"");
        printf("## %25s,   %25s,\n","RegionCount = \"r\"","RegionQuery = \"[\"");
        printf("## %25s,   %25s,\n","SpatialJoinCount = \"j\"","SpatialJoin = \"=\"");
        printf("## %25s,   %25s,\n","XJoinCount = \"J\"","XJoin = \"#\"");
        printf("## %25s,   %25s,\n","n-NearestTest = \"N\"","DistanceQuery = \"D\"");
        printf("## %36s,\n","CheckConsistency = \"C\"");
        printf("## %36s,\n","PathsDump = \"P\"");
        printf("## %36s,\n","ASCIIdump = \"A\"");
        printf("## %36s,\n","DirLevelDump = \"L\"");
        printf("## %36s,\n","MediaReorganization = \"M\"");
        printf("## %36s.\n","leave this menu = \"-\"");
        printf("Choice: ");
        do {
          choose= GetChar();
        } while ((choose != 'i') && (choose != 'd') &&
                 (choose != 'x') &&
                 (choose != 'e') && (choose != 'E') &&
                 (choose != 'r') && (choose != '[') &&
                 (choose != 'j') && (choose != '=') &&
                 (choose != 'J') && (choose != '#') &&
                 (choose != 'N') && (choose != 'D') &&
                 (choose != 'C') &&
                 (choose != 'P') &&
                 (choose != 'A') &&
                 (choose != 'L') &&
                 (choose != 'M') &&
                 (choose != '-'));
        GVdummyCh= GetChar();
        if (choose == '-') {
        }
        else if (choose == 'i') {
          if (opendata()) {
            exeInsert();
            closedata();
          }
        }
        else if (choose == 'd') {
          if (opendata()) {
            exeDelete();
            closedata();
          }
        }
        else if (choose == 'x') {
          if (opendata()) {
            exeExistsRegion();
            closedata();
          }
        }
        else if (choose == 'e') {
          QueryName();
          if (openquery()) {
            exeExactMatchCount();
            closequery();
          }
        }
        else if (choose == 'E') {
          QueryName();
          if (openquery()) {
            exeExactMatchQuery();
            closequery();
          }
        }
        else if (choose == 'r') {
          QueryName();
          if (openquery()) {
            exeRegionCount();
            closequery();
          }
        }
        else if (choose == '[') {
          QueryName();
          if (openquery()) {
            exeRegionQuery();
            closequery();
          }
        }
        else if (choose == 'j') {
          exeSpatialJoinCount();
        }
        else if (choose == '=') {
          exeSpatialJoin();
        }
        else if (choose == 'J') {
          exeXJoinCount();
        }
        else if (choose == '#') {
          exeXJoin();
        }
        else if (choose == 'D') {
          exeDistanceQuery();
        }
        else if (choose == 'N') {
          printf("\n");
          printf("NOTE that the query file must consist of RECTANGLES!\n");
          printf("Their LOW COORDINATES will serve as QUERY POINT!\n");
          QueryName();
          if (openquery()) {
            exeNNearestTest();
            closequery();
          }
        }
        else if (choose == 'C') {
          exeCheckConsistency(TRUE);
        }
        else if (choose == 'P') {
          exePathsDump();
        }
        else if (choose == 'A') {
          exeASCIIdump();
        }
        else if (choose == 'L') {
          exeDirLevelDump();
        }
        else if (choose == 'M') {
          exeMediaReorg();
        }
      }
    }
    else {
      if (! NoTreeInUse()) {
        mainchoose= 0;
      }
    }
  } while (mainchoose != 'q');
  return 0;
}

/***********************************************************************/

void ShortInstruction(void)

{
  printf("%s%s%s\n",
"\n##   --- TestRSTree ---  ","important notes",":"
  );
  printf("%s\n",
"## - This testprogram chiefly maintains a single \"main\" tree only. Exception:"
  );
  printf("%s\n",
"##   Joins, which work on the main tree and a secondary tree, and must be"
  );
  printf("%s\n",
"##   initialized by (re)opening/loading one of the trees as main tree first."
  );
  printf("%s\n",
"##   The testprogram keeps secondary trees open until some join finishes."
  );
  printf("%s\n",
"## - If an error occurs, the corresponding procedure does nothing but emitting"
  );
  printf("%s\n",
"##   a message, thus a great many of the actions may be retried (after enough"
  );
  printf("%s\n",
"##   non harmful failures)."
  );
  printf("%s\n",
"## - Damage protection for active trees: The following operations:"
  );
  printf("%s\n",
"##   \"Create\"(c, C), \"Open\"(o, O), \"MainCreate\"(m), \"Load\"(l) and \"quit\"(q)"
  );
  printf("%s\n",
"##   automatically close disk residing trees, whereas main memory trees must be"
  );
  printf("%s\n",
"##   killed (K) before these operations may be performed."
  );
  printf("%s\n",
"## NOTE that \"Create\"(c, C) removes a disk residing tree with the same name."
  );
}

/***********************************************************************/

boolean NoTreeInUse(void)

{
  char stork[10];
  boolean ok;
  
  TryGetStorageKind(GVrst,stork);
  if (strcmp(stork,"pri") == 0) {
    printf("A MAIN MEMORY RESIDING TREE EXISTS !\n");
    ok= FALSE;
  }
  else if (strcmp(stork,"") != 0) {
    ok= exeClose();
  }
  else {
    ok= TRUE;
  }
  return ok;
}

/***********************************************************************/

boolean JoinCalled(boolean *externJoin)

{
  char ch;
  boolean joinCalled= FALSE;
  
  printf("\n##      ===========================================\n## \n");
  printf("Prepare for Join? (y/n) ");
  do {
    ch= GetChar();
  } while ((ch != 'y') && (ch != 'n'));
  GVdummyCh= GetChar();
  if (ch == 'y') {
    joinCalled= TRUE;
    printf("R2=R? (y/n) ");
    do {
      ch= GetChar();
    } while ((ch != 'y') && (ch != 'n'));
    GVdummyCh= GetChar();
    if (ch == 'y') {
      *externJoin= FALSE;
      GVrst2= GVrst;
    }
    else {
      SecTreeName();
      *externJoin= TRUE;
    }
  }
  return joinCalled;
}

/***********************************************************************/

void DataAndTreeName(void)

{
  printf("%s%s%s","\nDataName | TreeName (omit \"",RST_SUFF,"\"): ");
  scanf("%s",GVdistName);
  GVdummyCh= GetChar();
  strlcpy(GV_RfileName,GVdistName,sizeof(GV_RfileName));
  strlcat(GV_RfileName,RST_SUFF,sizeof(GV_RfileName));
}

/***********************************************************************/

void GetAndPrintParamsForUsage(t_RT R) {

  char name[MaxNameLength];
  Rint dirPageSize, dataPageSize, numbOfDimensions, infoSize;
  boolean unique;
  
  GetCreatRSTDesc(R,
                  name,
                  &dirPageSize,
                  &dataPageSize,
                  &numbOfDimensions,
                  &infoSize,
                  &unique);
  /* adopt important lifecycle fix basic parameters: */
  GVnumbOfDim= numbOfDimensions;
  GVinfoSize= infoSize;
  /* build important lifecycle fix derived parameters: */
  GVrectSize= numbOfDimensions * sizeof(typinterval);
  GVpointSize= numbOfDimensions * sizeof(typcoord);
  /* print parameters: */
  printf(" directory page size: %d\n",dirPageSize);
  printf("      data page size: %d\n",dataPageSize);
  printf("number of dimensions: %d\n",numbOfDimensions);
  printf("information tag size: %d\n",infoSize);
  printf("              unique: %d\n",unique);
  PrintCoverage(R);
}

/***********************************************************************/

void SecTreeName(void)

{
  printf("%s%s%s","\nTreeName (omit \"",RST_SUFF,"\"): ");
  scanf("%s",GV_RfileName2);
  GVdummyCh= GetChar();
  strlcat(GV_RfileName2,RST_SUFF,sizeof(GV_RfileName2));
}

/***********************************************************************/

void QueryName(void)

{
  printf("%s","\nDataName: ");
  scanf("%s",GVquerName);
  GVdummyCh= GetChar();
}

/***********************************************************************/

void DisplErr(char *name, int recNumb)

{
  printf("-------------------------\n");
  printf("Error-Info: File: %s",name);
  if (recNumb != 0) {
    printf(", %dth RECORD BEYOND EOF.",recNumb+1); // counts start at 0
  }
  printf("\n");
  printf("-------------------------\n");
}
  
/***********************************************************************/
/* Optionally to be called for the case that RdBufBytes() returns 2 */

void DisplBufFillInfo(char *name, int recNumb) {

  printf("-------------------------\n");
  printf("Info: File %s",name);
  if (recNumb != 0) {
    printf(", %dth RECORD: Buffer fill incomplete! Probably due to EOF.",recNumb+1); // counts start at 0
  }
  printf("\n");
  printf("-------------------------\n");
}
  
/***********************************************************************/

boolean opendata(void)

{
  printf("Open Data File:\n");
  printf("%s\n",GVdistName);
  GVdataFile= fopen(GVdistName,"r");
  if (GVdataFile == NULL) {
    DisplErr(GVdistName,0);
    return FALSE;
  }
  else {
    return GetTestParameters(GVdataFile,GVdistName);
  }
}

/***********************************************************************/

void closedata(void)

{
  printf("Close Data File:\n");
  printf("%s\n",GVdistName);
  if (fclose(GVdataFile) != 0) {
    DisplErr(GVdistName,0);
  }
}

/***********************************************************************/

boolean openquery(void)

{
  printf("Open Query File:\n");
  printf("%s\n",GVquerName);
  GVquerFile= fopen(GVquerName,"r");
  if (GVquerFile == NULL) {
    DisplErr(GVquerName,0);
    return FALSE;
  }
  else {
    return GetTestParameters(GVquerFile,GVquerName);
  }
}

/***********************************************************************/

void closequery(void)

{
  printf("Close Query File:\n");
  printf("%s\n",GVquerName);
  if (fclose(GVquerFile) != 0) {
    DisplErr(GVquerName,0);
  }
}

/***********************************************************************/

void exeCreate(void)

{
  int dirpagelen, datapagelen, numbOfDimensions, infoSize;
  boolean unique;
  char ch;
  
  printf("Size of a directory page: ");
  scanf("%d",&dirpagelen);
  printf("     Size of a data page: ");
  scanf("%d",&datapagelen);
  printf("    Number of dimensions: ");
  scanf("%d",&numbOfDimensions);
  printf(" Size of information tag: ");
  scanf("%d",&infoSize);
  printf("Unique?, (y/n): ");
  do {
    ch= GetChar();
  } while ((ch != 'y') && (ch != 'n'));
  GVdummyCh= GetChar();
  unique= ch == 'y';
  
  /* adopt important lifecycle fix basic parameters: */
  GVnumbOfDim= numbOfDimensions;
  GVinfoSize= infoSize;
  /* build important lifecycle fix derived parameters: */
  GVrectSize= numbOfDimensions * sizeof(typinterval);
  GVpointSize= numbOfDimensions * sizeof(typcoord);
  
  printf("%s%s%s","TryRemoveRST(",GV_RfileName,"):\n");
  if (TryRemoveRST(GV_RfileName)) {
    printf("Done\n");
  }
  printf("%s%s,%d,%d,%d,%d,%d%s\n","CreateRST(",GV_RfileName,dirpagelen,datapagelen,GVnumbOfDim,GVinfoSize,unique,"):");
  if (CreateRST(GV_RfileName,dirpagelen,datapagelen,GVnumbOfDim,GVinfoSize,unique)) {
    printf("Done\n");
  }
  else {
    printf("FAILURE(CreateRST)\n");
  }
}

/***********************************************************************/

void exeRemove(void)

{
  printf("%s%s%s","\nRemoveRST(",GV_RfileName,"):\n");
  if (RemoveRST(GV_RfileName)) {
    printf("Done\n");
  }
  else {
    printf("FAILURE(RemoveRST)\n");
  }
}

/***********************************************************************/

void exeMainCreate(void)

{
  int dirpagelen, datapagelen, numbOfDimensions, infoSize;
  Rpint dirramsize, dataramsize;
  boolean unique;
  char ch;
  
  printf(" dirRAMsize: ");
  scanf(strans("%P",s),&dirramsize);
  printf("dataRAMsize: ");
  scanf(strans("%P",s),&dataramsize);
  printf("Size of a directory page: ");
  scanf("%d",&dirpagelen);
  printf("     Size of a data page: ");
  scanf("%d",&datapagelen);
  printf("    Number of dimensions: ");
  scanf("%d",&numbOfDimensions);
  printf(" Size of information tag: ");
  scanf("%d",&infoSize);
  printf("Unique?, (y/n): ");
  do {
    ch= GetChar();
  } while ((ch != 'y') && (ch != 'n'));
  GVdummyCh= GetChar();
  unique= ch == 'y';
  
  /* adopt important lifecycle fix basic parameters: */
  GVnumbOfDim= numbOfDimensions;
  GVinfoSize= infoSize;
  /* build important lifecycle fix derived parameters: */
  GVrectSize= numbOfDimensions * sizeof(typinterval);
  GVpointSize= numbOfDimensions * sizeof(typcoord);
  
  printf(strans("%s%d,%d,%d,%d,%d,%P,%P,%d%s\n",s),"CreateMainMemRST(",dirpagelen,datapagelen,GVnumbOfDim,GVinfoSize,unique,dirramsize,dataramsize,TRUE,"):");
  if (CreateMainMemRST(&GVrst,dirpagelen,datapagelen,GVnumbOfDim,GVinfoSize,unique,dirramsize,dataramsize,TRUE)) {
    printf("Done\n");
    printf("\n");
    GetAndPrintParamsForUsage(GVrst);
  }
  else {
    printf("FAILURE(CreateMainMemRST)\n");
  }
}

/***********************************************************************/

void exeRemoveMain(void)

{
  printf("%s","RemoveMainMemRST:\n");
  if (RemoveMainMemRST(&GVrst)) {
    printf("Done\n");
  }
  else {
    printf("FAILURE(RemoveMainMemRST)\n");
  }
}

/***********************************************************************/

boolean exeOpen(char *name, t_RT *r)

{
  printf("%s%s%s","OpenRST(",name,"):\n");
  if (OpenRST(r,name)) {
    printf("Done\n");
    printf("\n");
    GetAndPrintParamsForUsage(*r);
  }
  else {
    printf("FAILURE(OpenRST)\n");
    return FALSE;
  }
  return TRUE;
}

/***********************************************************************/
/* NOTE: Page sizes are freely choosable in this test program, with the sole
   exception that directory and data pages have to be equally sized in case
   of LRU buffering.
   This function creates an LRU buffer or may expand it in case of a join,
   whereas exeClose() disposes the LRU buffer. */
   
boolean exeOpenBuffered(char *name,
                     t_RT *r,
                     t_LRU *lru,
                     boolean isJoin,
                     int otherDirPsize,
                     int otherDataPsize)

{
  Rint dirPageSize, dataPageSize, bufferPageSize;
  Rpnint numbBufPages, currCap, currUse;
  boolean success;
  
  if (OpenRST(r,name)) {
    GetPageSizes(*r,&dirPageSize,&dataPageSize);
    CloseRST(r);
  }
  else {
    printf("FAILURE(OpenRST)\n");
    return FALSE;
  }
  if (dirPageSize == dataPageSize) {
    bufferPageSize= dataPageSize;
    printf("dirPageSize = dataPageSize = bufferPageSize = %d\n",bufferPageSize);
    printf("\n");
  }
  else {
    printf("Buffering IMPOSSIBLE!\n");
    printf("dirPageSize = %d != %d = dataPageSize\n",dirPageSize,dataPageSize);
    printf("Options:\n");
    printf("- Opening without buffer\n");
    printf("- Loading into main memory\n");
    return FALSE;
  }
  
  if (! isJoin || isJoin && dataPageSize != otherDataPsize) {
    if (! isJoin) {
      if (GVlru != NULL) {
        printf("IMPLEMENTATION LACK in RSTt.c.exeOpenBuffered: GVlru= %p\n",GVlru);
        return FALSE;
      }
    }
    else {
      printf(strans("%s has other page size: %N\n",s),name,dataPageSize);
      printf("Second LRU buffer required!\n");
    }
    printf("LRU buffer capacity (#pages): ");
    scanf(strans("%N",s),&numbBufPages);
    success= TRUE;
    /* This is important since the success flag is never set to TRUE, but */
    /* only (where appropriate) set to FALSE by the called LRU function!  */
    printf(strans("%s%N,%d%s",s),"NewLRU(",numbBufPages,bufferPageSize,"):\n");
    NewLRU(lru,numbBufPages,bufferPageSize,&success);
    if (! success) {
      printf("FAILURE(NewLRU)\n");
      return FALSE;
    }
    if (OpenBufferedRST(r,name,*lru)) {
      GVisBuffered= TRUE; /* flag, setting dir-/data-specific R/W output to 0 */
      printf("Done\n");
      printf("\n");
      GetAndPrintParamsForUsage(*r);
    }
    else {
      printf("FAILURE(OpenBufferedRST)\n");
      return FALSE;
    }
  }
  else {
    if (GVlru == NULL) {
      printf("IMPLEMENTATION LACK in RSTt.c.exeOpenBuffered: GVlru= %p\n",GVlru);
      return FALSE;
    }
    GetLRUCapUse(GVlru,&currCap,&currUse);
    printf(strans("Current LRU buffer capacity: %N pages\n",s),currCap);
    printf(strans("Current LRU buffer allocation: %N pages\n",s),currUse);
    printf("New LRU buffer capacity (0 = keep): ");
    scanf(strans("%N",s),&numbBufPages);
    if (numbBufPages == 0) {
      numbBufPages= currCap;
    }
    success= TRUE;
    do {
      ModifLRUCap(GVlru,numbBufPages,&success);
      if (! success) {
        printf("FAILURE(ModifLRUCap)\n");
        printf("New LRU buffer capacity: ");
        scanf(strans("%N",s),&numbBufPages);
      }
    } while (! success);
    printf("%s%s%s","OpenBufferedRST(",name,"):\n");
    if (OpenBufferedRST(r,name,GVlru)) {
      GVisBuffered= TRUE; /* flag, setting dir-/data-specific R/W output to 0 */
      printf("Done\n");
      printf("\n");
      GetAndPrintParamsForUsage(*r);
    }
    else {
      printf("FAILURE(OpenBufferedRST)\n");
      return FALSE;
    }
  }
  return TRUE;
}

/***********************************************************************/
/* NOTE: See exeOpenBuffered() */

boolean exeClose(void)

{
  char name[MaxNameLength];
  unsigned int cap, use;
  boolean lruDone= TRUE;
  t_FPB locksBag= FPB_Empty();
  
  if (! GetName(GVrst,name)) {
    sprintf(name,"%s","NIL");
  }
  if (GVlru != NULL) {
    /* check LRU things: */
    GetLRUCapUse(GVlru,&cap,&use);
    printf("LRU buffer: cap: %u, use: %u\n",cap,use);
    locksBag= LRUPagesLocked(GVlru,&lruDone);
    printf("LRU buffer entries locked:\n");
    FPB_Print(stdout,locksBag,5);
    FPB_Delete(&locksBag);
  }
  printf("%s%s%s","CloseRST(",name,"):\n");
  if (CloseRST(&GVrst)) {
    if (GVlru != NULL) {
      GetLRUCapUse(GVlru,&cap,&use);
      printf("LRU buffer: cap: %u, use: %u\n",cap,use);
      PrintLRUStatistics(GVlru);
      printf("%s","DisposeLRU():\n");
      DisposeLRU(&GVlru);
    }
    GVisBuffered= FALSE; /* reinitialize flag for R/W counts output */
    printf("Done\n");
  }
  else {
    printf("FAILURE(CloseRST)\n");
    return FALSE;
  }
  return TRUE;
}

/***********************************************************************/

void exeSyncRST(void) {

  printf("Synchronizing the tree\n");
  if (SyncRST(GVrst)) {
    printf("Done.\n");
  }
}

/***********************************************************************/

void exeGetRSTSync(void) {

  printf("Getting tree synchronization\n");
  if (GetRSTSync(GVrst)) {
    printf("Done.\n");
  }
}

/***********************************************************************/

boolean exeLoad(char *name, t_RT *r)

{
  Rpint dirramsize, dataramsize;
  
  printf(" dirRAMsize: ");
  scanf(strans("%P",s),&dirramsize);
  printf("dataRAMsize: ");
  scanf(strans("%P",s),&dataramsize);
  GVdummyCh= GetChar();
  printf("%s%s%s","LoadRST(",name,"):\n");
  if (LoadRST(r,name,dirramsize,dataramsize)) {
    printf("Done\n");
    printf("\n");
    GetAndPrintParamsForUsage(*r);
  }
  else {
    printf("FAILURE(LoadRST)\n");
    return FALSE;
  }
  return TRUE;
}

/***********************************************************************/

boolean exeSave(void)

{
  printf("%s%s%s","SaveRST(",GV_RfileName,"):\n");
  if (SaveRST(GVrst,GV_RfileName)) {
    printf("Done\n");
  }
  else {
    printf("FAILURE(SaveRST)\n");
    return FALSE;
  }
  return TRUE;
}

/***********************************************************************/

boolean GetTestParameters(FILE *file, char *name)

{
  char ch;
  size_t bufSize;
  int i;
  
  printf("\nExecution from/to entrynumber?\n");
  printf("(\"1 n\": first up to n-th, \"n 0\": n-th up to last,\n");
  printf(" \"1 0\" | \"0 0\": first up to last entry.)\n");
  printf("Input: ");
  do {
    scanf("%d",&GVbegin); scanf("%d",&GVend);
  } while (GVbegin == 0 && GVend != 0);
  if (GVbegin != 0) { GVbegin--; }
  if (GVend == 0) {
    /* determine total number of records (lines) */
    printf("Determining total number of records ...\n");
    while (1) {
      getline(&GVstrPtr,&bufSize,file);
      if (feof(file)) {
        break;
      }
      GVend++;
    }
    printf("Total number of records: %d\n",GVend);
    /* ********** rewind stream to start position: ********** */
    rewind(file);
  }
  /* skip lines BEFORE starting line (set start position) */
  printf("Setting start position ...\n");
  for (i= 0; i < GVbegin; i++) {
    getline(&GVstrPtr,&bufSize,file);
  }
  printf("Done\n");
  GVNumbCalls= GVend - GVbegin;

  printf("Message if done/match =\"d\",");
  printf(" not done/match =\"n\", message off =\"o\"\n");
  printf("Input: ");
  do {
    ch= GetChar();
  } while ((ch != 'd') && (ch != 'n') && (ch != 'o'));
  GVdummyCh= GetChar();
  GVnotDmsg= ch == 'n';
  GVdoneMsg= ch == 'd';
  printf("Feed Back at entrynumber MODULO m (m = 0: only at the end).\n");
  printf("m = ");
  scanf("%d",&GVfdBckMod);
  GVdummyCh= GetChar();
  if (GVfdBckMod == 0) {GVfdBckMod= MAXINT;}
  return TRUE;
}

/***********************************************************************/

void exeInsert(void)

{
  int i;
  double *doublePtr;
  boolean inserted, success;
  refinfo info= allocM(GVinfoSize);
  refinfo infoStored= allocM(GVinfoSize);
  typinterval *rectangle= allocM(GVrectSize);
  
  CountsOn0(GVrst);
  
  i= GVbegin;
  StartTimers();
  do {
    if (! RdTextToRect(GVdataFile,rectangle)) {
      DisplErr(GVdistName,i);
      freeM(&info); freeM(&infoStored); freeM(&rectangle);
      return;
    }
    i++;
    /*** Begin set (*info).tag to a verifyable value ***/
    //(*info).tag= FIXED_INFO_PART;
    (*info).tag= i;
    /* pass a double (pass i as such): */
    //doublePtr= (double *) &(*info).c[0];
    //*doublePtr= i;
    /* pass a string: */
    //sprintf(&(*info).c[0],"16 BYTES   LONG");
    //                     /*123456789012345*/
    /*** End   set (*info).tag to a verifyable value ***/
    success= InsertRecord(GVrst,rectangle,info,&inserted,infoStored);
    if (! success) {
      printf("FAILURE(InsertRecord)\n");
      PrintIndHeight(GVrst,i);
      freeM(&info); freeM(&infoStored); freeM(&rectangle);
      return;
    }
    if (GVnotDmsg) {
      if (! inserted) {
        printf("%s%d%s%d\n"," NOT INSERTED: ",i," stored: ",(*infoStored).tag);
      }
    }
    else if (GVdoneMsg) {
      if (inserted) {
        printf("%s%d\n"," INSERTED: ",i);
      }
    }
    if ((i % GVfdBckMod == 0) || (i == GVend)) {
      PrintIndHeight(GVrst,i);
      /*** Begin consistency check at each feedback: ***/
      /*** DAMAGES counting! ***/
      //if (! exeCheckConsistency(TRUE)) {
      //  break;
      //}
      /*** End   consistency check at each feedback: ***/
    }
    /*** Begin consistency check after each record: ***/
    /*** DAMAGES CPU- and I/O-performance!! ***/
    /*** DAMAGES counting! ***/
    //if (! exeCheckConsistency(FALSE)) {
    //  break;
    //}
    /*** End   consistency check after each record: ***/
  } while (i != GVend);
  StopTimers();
  
  printf("\n");
  PrintCalls(GVNumbCalls);
  PrintReads(GVrst,-1);
  PrintWrites(GVrst,-1);
  PrintAccesses(GVrst,-1);
  PrintOverUnder(GVrst);
  PrintChooseSubtree(GVrst);
  PrintAdminReads(GVrst);
  PrintAdminWrites(GVrst);
  PrintTimes(GVrst);
  freeM(&info); freeM(&infoStored); freeM(&rectangle);
}

/***********************************************************************/

boolean infoEqual(t_RT rt,
                  const typinfo *stored,
                  Rint infoSize,
                  const typinfo *searched,
                  void *ptr) {
  
  return (*stored).tag == (*searched).tag;
  /* check a double: */
  //return *(double *)stored == *(double *)searched;
}

/***********************************************************************/

void exeDelete(void)

{
  int i;
  double *doublePtr;
  boolean found, success, reorganized, truncated;
  refinfo info= allocM(GVinfoSize);
  typinterval *rectangle= allocM(GVrectSize);
  Rlint reorgcount, trunccount;
  
  CountsOn0(GVrst);
  reorgcount= 0;
  trunccount= 0;
  
  i= GVbegin;
  StartTimers();
  do {
    if (! RdTextToRect(GVdataFile,rectangle)) {
      DisplErr(GVdistName,i);
      freeM(&info); freeM(&rectangle);
      return;
    }
    i++;
    /*** Begin set (*info).tag to a verifyable value ***/
    //(*info).tag= FIXED_INFO_PART;
    (*info).tag= i;
    /* pass a double (pass i as such): */
    //doublePtr= (double *) &(*info).c[0];
    //*doublePtr= i;
    /* pass a string: */
    //sprintf(&(*info).c[0],"16 BYTES   LONG");
    //                     /*123456789012345*/
    /*** End   set (*info).tag to a verifyable value ***/
    /*** Begin create scattered gaps [CAUTION: exeDelete() BEHAVIOR !!] ***/
    //if (i % 3 == 0) {
    //  if (i % 2 == 0) {
    //    continue;
    //  }
    //}
    /*** End   create scattered gaps [CAUTION: exeDelete() BEHAVIOR !!] ***/
    success= DeleteRecord(GVrst,rectangle,info,infoEqual,NULL,&found);
    if (! success) {
      printf("FAILURE(DeleteRecord)\n");
      PrintIndHeight(GVrst,i);
      freeM(&info); freeM(&rectangle);
      return;
    }
    if (GVnotDmsg) {
      if (! found) {
        printf("%s%d\n"," NOT FOUND: ",i);
      }
    }
    else if (GVdoneMsg) {
      if (found) {
        printf("%s%d\n"," FOUND: ",i);
      }
    }
    if ((i % GVfdBckMod == 0) || (i == GVend)) {
      PrintIndHeight(GVrst,i);
      /*** Begin consistency check at each feedback: ***/
      /*** DAMAGES counting! ***/
      //if (! exeCheckConsistency(TRUE)) {
      //  break;
      //}
      /*** End   consistency check at each feedback: ***/
    }
    /*** Begin reorganize Medium after each SINGLE DELETION ***/
    //success= ReorganizeMedium(GVrst,FALSE,&reorganized,&truncated);
    //if (! success) {
    //  printf("FAILURE(ReorganizeMedium)\n");
    //  freeM(&info); freeM(&rectangle);
    //  DisposeFileBuf(&fb);
    //  return;
    //}
    //if (reorganized) {
    //  reorgcount++;
    //}
    //if (truncated) {
    //  trunccount++;
    //}
    /*** End   reorganize Medium after each SINGLE DELETION ***/
    /*** Begin consistency check after each record: ***/
    /*** DAMAGES CPU- and I/O-performance!! ***/
    /*** DAMAGES counting! ***/
    //if (! exeCheckConsistency(FALSE)) {
    //  break;
    //}
    /*** End   consistency check after each record: ***/
  } while (i != GVend);
  /*** Begin reorganize Medium after each DELETION SEQUENCE ***/
  //success= ReorganizeMedium(GVrst,TRUE,&reorganized,&truncated);
  //if (! success) {
  //  printf("FAILURE(ReorganizeMedium)\n");
  //  freeM(&info); freeM(&rectangle);
  //  DisposeFileBuf(&fb);
  //  return;
  //}
  //if (reorganized) {
  //  reorgcount++;
  //}
  //if (truncated) {
  //  trunccount++;
  //}
  /*** End   reorganize Medium after each DELETION SEQUENCE ***/
  StopTimers();
  
  printf(strans("%L Reorganization(s), %L Truncation(s)\n",s),reorgcount,trunccount);
  printf("\n");
  PrintCalls(GVNumbCalls);
  PrintReads(GVrst,-1);
  PrintWrites(GVrst,-1);
  PrintAccesses(GVrst,-1);
  PrintOverUnder(GVrst);
  PrintChooseSubtree(GVrst);
  PrintAdminReads(GVrst);
  PrintAdminWrites(GVrst);
  PrintTimes(GVrst);
  freeM(&info); freeM(&rectangle);
}

/***********************************************************************/

void exeExistsRegion(void)

{
  Rlint allfound;
  int i;
  boolean found, success;
  typinterval *rects= allocM(1 * GVrectSize);
  void *qPtr= NULL;
  
  allfound= 0;
  ClearPath(GVrst);
  CountsOn0(GVrst);
  GVCompCount0();
  
  i= GVbegin;
  StartTimers();
  do {
    if (! RdTextToRect(GVdataFile,rects)) {
      DisplErr(GVquerName,i);
      freeM(&rects);
      return;
    }
    i++;
    success= ExistsRegion(GVrst,rects,1,qPtr,DirEncloses,DataEqual,&found);
    if (! success) {
      printf("FAILURE(ExistsRegion)\n");
      PrintIndHeightFounds(GVrst,i,allfound);
      freeM(&rects);
      return;
    }
    if (found) {
      allfound++;
    }
    if (GVnotDmsg) {
      if (! found) {
        printf("%s%d\n"," NOT FOUND: ",i);
      }
    }
    else if (GVdoneMsg) {
      if (found) {
        printf("%s%d\n"," FOUND: ",i);
      }
    }
    if ((i % GVfdBckMod == 0) || (i == GVend)) {
      PrintIndHeightFounds(GVrst,i,allfound);
    }
  } while (i != GVend);
  StopTimers();
  
  printf("\n");
  PrintCalls(GVNumbCalls);
  PrintGVCompares(allfound);
  PrintReads(GVrst,allfound);
  PrintTimes(GVrst);
  freeM(&rects);
}

/***********************************************************************/

void exeExactMatchCount(void)

{
  Rlint numfound, minfound, maxfound, allfound, allsqrfound, emptyquery;
  int i;
  boolean first, success;
  typinterval *rects= allocM(1 * GVrectSize);
  void *qPtr= NULL;
  
  allfound= 0;
  allsqrfound= 0;
  emptyquery= 0;
  ClearPath(GVrst);
  CountsOn0(GVrst);
  GVCompCount0();
  
  i= GVbegin; first= TRUE;
  StartTimers();
  do {
    if (! RdTextToRect(GVquerFile,rects)) {
      DisplErr(GVquerName,i);
      freeM(&rects);
      return;
    }
    i++;
    success= RegionCount(GVrst,
                         rects,1,qPtr,DirEncloses,DataEqual,
                         &numfound);
    if (! success) {
      printf("FAILURE(ExactMatchCount)\n");
      PrintIndHeightMatches(GVrst,i,allfound);
      freeM(&rects);
      return;
    }
    if (numfound == 0) {
      emptyquery++;
    }
    else {
      allfound+= numfound;
      allsqrfound+= numfound * numfound;
    }
    if (first) {
      first= FALSE;
      minfound= numfound;
      maxfound= numfound;
    }
    else {
      if (numfound < minfound) {
        minfound= numfound;
      }
      else if (numfound > maxfound) {
        maxfound= numfound;
      }
    }
    if (GVnotDmsg) {
      if (numfound == 0) {
        printf("%s%d\n"," NO MATCH: ",i);
      }
    }
    else if (GVdoneMsg) {
      if (numfound != 0) {
        printf("%s%d\n"," MATCH: ",i);
      }
    }
    if ((i % GVfdBckMod == 0) || (i == GVend)) {
      PrintIndHeightMatches(GVrst,i,allfound);
    }
  } while (i != GVend);
  StopTimers();
  
  printf(strans("%s%L\n",s),"empty queries: ",emptyquery);
  printf(strans("%s%L\n",s),"rectangles found: ",allfound);
  printf("\n");
  PrintCalls(GVNumbCalls);
  PrintFounds(minfound,maxfound,allfound,allsqrfound);
  PrintGVCompares(allfound);
  PrintReads(GVrst,allfound);
  PrintTimes(GVrst);
  freeM(&rects);
}

/***********************************************************************/

void exeExactMatchQuery(void)

{
  Rlint allfound, emptyquery;
  int i;
  boolean success;
  typHandleRec handleRec;
  typinterval *rects= allocM(1 * GVrectSize);
  void *qPtr= NULL;
  
  handleRec.info= allocM(GVinfoSize);
  
  /*****/
  CreateTruncF("ExactMatchQ_Dupl",&handleRec.f);
  /* ---> CreateFileBuf, pass its reference, then use WrBufBytes! */
  /*****/
  
  allfound= 0;
  emptyquery= 0;
  ClearPath(GVrst);
  CountsOn0(GVrst);
  GVCompCount0();
  
  i= GVbegin;
  StartTimers();
  do {
    if (! RdTextToRect(GVquerFile,rects)) {
      DisplErr(GVquerName,i);
      freeM(&rects); freeM(&handleRec.info);
      return;
    }
    i++;
    GVnumFound= 0;
    (*handleRec.info).tag= i;     /* info part of the search record */
    handleRec.qstart= TRUE; handleRec.recordMatch= FALSE;
    success= RegionQuery(GVrst,
                         rects,1,qPtr,DirEncloses,DataEqual,
                         &handleRec,ManageEMQuery);
    if (! success) {
      printf("FAILURE(ExactMatchQuery)\n");
      PrintIndHeightMatches(GVrst,i,allfound);
      freeM(&rects); freeM(&handleRec.info);
      return;
    }
    if (! handleRec.qstart) {
      printf("\n"); /* close duplicates string of ManageQuery */
    }
    if (GVnumFound == 0) {
      emptyquery++;
    }
    else {
      allfound= allfound+GVnumFound;
    }
    if (GVnotDmsg) {
      if (GVnumFound == 0) {
        printf("%s%d\n"," NOT EVEN RECTANGLE MATCH: ",i);
      }
      else if (! handleRec.recordMatch) {
        printf("%s%d\n"," NO MATCH OF INFO PART: ",i);
      }
    }
    else if (GVdoneMsg) {
      if (handleRec.recordMatch) {
        printf("%s%d\n"," FULL MATCH: ",i);
      }
      else if (GVnumFound != 0) {
        printf("%s%d\n"," RECTANGLE MATCH: ",i);
      }
    }
    if ((i % GVfdBckMod == 0) || (i == GVend)) {
      PrintIndHeightMatches(GVrst,i,allfound);
    }
  } while (i != GVend);
  StopTimers();
  
  /*****/
  CloseF(handleRec.f);
  /*****/
  
  printf(strans("%s%L\n",s),"empty queries: ",emptyquery);
  printf(strans("%s%L\n",s),"rectangles found: ",allfound);
  printf("\n");
  PrintCalls(GVNumbCalls);
  PrintGVCompares(allfound);
  PrintReads(GVrst,allfound);
  PrintTimes(GVrst);
  freeM(&rects); freeM(&handleRec.info);
}

/***********************************************************************/

void ManageEMQuery(t_RT R,
                   Rint numbOfDim,
                   const typinterval *rectangle,
                   refinfo infoptr,
                   Rint infoSize,
                   void *ptr,
                   boolean *modify,
                   boolean *finish)

{
  char dummy;
  refHandleRec hand= ptr;
  
  /***** ----- count ----- *****/
  
  GVnumFound++;
  
  /***** ----- check existence of a matching complete record ----- *****/
  
  if ((*(*hand).info).tag == (*infoptr).tag) {
    (*hand).recordMatch= TRUE;
  }

  /***** ----- prompt the user ----- *****/
  
  //printf(">"); dummy= GetChar();
  
  /***** ----- print rectangle ----- *****/
  
  //PrintRect(rectangle);
  
  /***** ----- print info parts of duplicates ----- *****/
  
  //if ((*(*hand).info).tag < (*infoptr).tag) {
  //  if ((*hand).qstart) {
  //    (*hand).qstart= FALSE;
  //    printf("%10d",(*(*hand).info).tag);     /* info part of the search record */
  //  }
  //  printf(" = %10d",(*infoptr).tag);
  //}
  
  /***** ----- write info parts of duplicates to file----- *****/
  
  //if ((*(*hand).info).tag < (*infoptr).tag) {
  //  WrBytes((*hand).f,&(*infoptr).tag,sizeof((*infoptr).tag));
  //}
  
  /***** ----- modify every second record ----- *****/
  
  //if (GVnumFound % 2 == 0) {
  //  (*infoptr).tag= FIXED_INFO_PART;
  //  *modify= TRUE;
  //}
  
  /***** ----- finish after finding 42 entries ----- *****/
  
  //if (GVnumFound == 42) {
  //  *finish= TRUE;
  //}
  
}

/***********************************************************************/

void exeRegionCount(void)

{
  Rlint numfound, minfound, maxfound, allfound, allsqrfound, emptyquery;
  int i;
  boolean first, success;
  typinterval *rects= allocM(1 * GVrectSize);
  void *qPtr= NULL;
  
  allfound= 0;
  allsqrfound= 0;
  emptyquery= 0;
  ClearPath(GVrst);
  CountsOn0(GVrst);
  GVCompCount0();
  
  i= GVbegin; first= TRUE;
  StartTimers();
  do {
    if (! RdTextToRect(GVquerFile,rects)) {
      DisplErr(GVquerName,i);
      freeM(&rects);
      return;
    }
    i++;
    success= RegionCount(GVrst,
                         rects,1,qPtr,DirIntersects,DataIntersects,
                         &numfound);
    if (! success) {
      printf("FAILURE(RegionCount)\n");
      PrintIndHeightMatches(GVrst,i,allfound);
      freeM(&rects);
      return;
    }
    if (numfound == 0) {
      emptyquery++;
    }
    else {
      allfound+= numfound;
      allsqrfound+= numfound * numfound;
    }
    if (first) {
      first= FALSE;
      minfound= numfound;
      maxfound= numfound;
    }
    else {
      if (numfound < minfound) {
        minfound= numfound;
      }
      else if (numfound > maxfound) {
        maxfound= numfound;
      }
    }
    if (GVnotDmsg) {
      if (numfound == 0) {
        printf("%s%d\n"," NO MATCH: ",i);
      }
    }
    else if (GVdoneMsg) {
      if (numfound != 0) {
        printf("%s%d\n"," MATCH: ",i);
      }
    }
    if ((i % GVfdBckMod == 0) || (i == GVend)) {
      PrintIndHeightMatches(GVrst,i,allfound);
    }
  } while (i != GVend);
  StopTimers();
  
  printf(strans("%s%L\n",s),"empty queries: ",emptyquery);
  printf(strans("%s%L\n",s),"rectangles found: ",allfound);
  printf("\n");
  PrintCalls(GVNumbCalls);
  PrintFounds(minfound,maxfound,allfound,allsqrfound);
  PrintGVCompares(allfound);
  PrintReads(GVrst,allfound);
  PrintTimes(GVrst);
  freeM(&rects);
}

/***********************************************************************/
    /* 
    test AllQuery:
    in exeRegionQuery replace the call of RegionQuery by
    success= AllQuery(GVrst,&fildes,ManageRegQuery);
    */
/***********************************************************************/

void exeRegionQuery(void)

{
  Rlint allfound, emptyquery;
  int i;
  boolean success;
  typfildes fildes;
  typinterval *rects= allocM(1 * GVrectSize);
  void *qPtr= NULL;
  
  /*****/
  CreateTruncF("RegionQ",&fildes.file1);
  /* ---> CreateFileBuf, pass its reference, then use WrBufBytes! */
  /*****/
  
  allfound= 0;
  emptyquery= 0;
  ClearPath(GVrst);
  CountsOn0(GVrst);
  GVCompCount0();
  
  i= GVbegin;
  StartTimers();
  do {
    if (! RdTextToRect(GVquerFile,rects)) {
      DisplErr(GVquerName,i);
      freeM(&rects);
      return;
    }
    i++;
    GVnumFound= 0;
    success= AllQuery(GVrst,&fildes,ManageRegQuery);
    //success= RegionQuery(GVrst,
    //                     rects,1,qPtr,DirIntersects,DataIntersects,
    //                     &fildes,ManageRegQuery);
    if (! success) {
      printf("FAILURE(RegionQuery)\n");
      PrintIndHeightMatches(GVrst,i,allfound);
      freeM(&rects);
      return;
    }
    if (GVnumFound == 0) {
      emptyquery++;
    }
    else {
      allfound= allfound+GVnumFound;
    }
    if (GVnotDmsg) {
      if (GVnumFound == 0) {
        printf("%s%d\n"," NO MATCH: ",i);
      }
    }
    else if (GVdoneMsg) {
      if (GVnumFound != 0) {
        printf("%s%d\n"," MATCH: ",i);
      }
    }
    if ((i % GVfdBckMod == 0) || (i == GVend)) {
      PrintIndHeightMatches(GVrst,i,allfound);
    }
  } while (i != GVend);
  StopTimers();
  
  /*****/
  CloseF(fildes.file1);
  /*****/
  
  printf(strans("%s%L\n",s),"empty queries: ",emptyquery);
  printf(strans("%s%L\n",s),"rectangles found: ",allfound);
  printf("\n");
  PrintCalls(GVNumbCalls);
  PrintGVCompares(allfound);
  PrintReads(GVrst,allfound);
  PrintTimes(GVrst);
  freeM(&rects);
}

/***********************************************************************/

void ManageRegQuery(t_RT R,
                    Rint numbOfDim,
                    const typinterval *rectangle,
                    refinfo infoptr,
                    Rint infoSize,
                    void *ptr,
                    boolean *modify,
                    boolean *finish)

{
  char dummy;
  
  /***** ----- count ----- *****/
  
  GVnumFound++;
  
  /***** ----- prompt the user ----- *****/
  
  //printf(">"); dummy= GetChar();
  
  /***** ----- print rectangle ----- *****/
  
  //PrintRect(rectangle);
  
  /***** ----- print info part ----- *****/
  
  //printf("%15d\n",(*infoptr).tag);
  
  /***** ----- write rectangles to files ----- *****/
  
  //WrBytes((*(reffildes)ptr).file1,rectangle,GVrectSize);
  
  /***** ----- modify every second record ----- *****/
  
  //if (GVnumFound % 2 == 0) {
  //  (*infoptr).tag= FIXED_INFO_PART;
  //  *modify= TRUE;
  //}
  
  /***** ----- finish after finding 42 entries ----- *****/
  
  //if (GVnumFound == 42) {
  //  *finish= TRUE;
  //}
  
}

/***********************************************************************/

void exeXJoin(void)

{
  typinterval *RqRects= allocM(1 * GVrectSize);
  typinterval *R2qRects= allocM(1 * GVrectSize);
  void *qPtr= NULL;
  typfildes fildes;
  char stork[10];
  boolean success;
  
  /*
  RqRects[0][0].l= 0.0;
  RqRects[0][0].h= 0.75;
  RqRects[0][1].l= 0.0;
  RqRects[0][1].h= 0.75;
  
  R2qRects[0][0].l= 0.25;
  R2qRects[0][0].h= 1.0;
  R2qRects[0][1].l= 0.25;
  R2qRects[0][1].h= 1.0;
  */
  /*****/
  CreateTruncF("XJoin1",&fildes.file1);
  /* ---> CreateFileBuf, pass its reference, then use WrBufBytes! */
  CreateTruncF("XJoin2",&fildes.file2);
  /* ---> CreateFileBuf, pass its reference, then use WrBufBytes! */
  /*****/
  
  GVNumbCalls= 1; /* this one */
  ClearPath(GVrst);
  ClearPath(GVrst2);
  CountsOn0(GVrst);
  CountsOn0(GVrst2);
  GVCompCount0();

  GVpairCount= 0;
  StartTimers();
  success= XJoin(GVrst,RqRects,0,qPtr,AlwaysTrue,AlwaysTrue,
                 GVrst2,R2qRects,0,qPtr,AlwaysTrue,AlwaysTrue,
                 J_DirIntersects,J_DataIntersects,
                 &fildes,ManageJoin);
  if (! success) {
    printf("FAILURE(XJoin)\n");
    freeM(&RqRects); freeM(&R2qRects);
    return;
  }
  StopTimers();
  
  /*****/
  CloseF(fildes.file1);
  CloseF(fildes.file2);
  /*****/
  
  printf("\n");
  printf(strans("%s%L\n",s),"Number of pairs found: ",GVpairCount);
  printf("\n");
  PrintGVCompares(GVpairCount);
  if (GVrst2 != GVrst) {
    /*
    PrintReads(GVrst,GVpairCount);
    PrintReads(GVrst2,GVpairCount);
    */
    PrintJoinedReads(GVrst,GVrst2,GVpairCount);
  }
  else {
    PrintReads(GVrst,GVpairCount);
  }
  PrintTimes(GVrst);
  
  if (GVrst2 != GVrst) {
    success= GetStorageKind(GVrst2,stork);
    if (! success) {
      printf("FAILURE(GetMainMemFlag)\n");
      freeM(&RqRects); freeM(&R2qRects);
      return;
    }
    if (strcmp(stork,"pri") == 0) {
      printf("RemoveMainMemRST(R2):\n");
      if (RemoveMainMemRST(&GVrst2)) {
        printf("Done\n");
      }
      else {
        printf("FAILURE(RemoveMainMemRST)\n");
      }
    }
    else {
      printf("CloseRST(R2):\n");
      if (CloseRST(&GVrst2)) {
        if (GVlru2 != NULL) {
          DisposeLRU(&GVlru2);
        }
        printf("Done\n");
      }
      else {
        printf("FAILURE(CloseRST)\n");
      }
    }
  }
  else {
    InitRSTreeIdent(&GVrst2);
  }
  freeM(&RqRects); freeM(&R2qRects);
}

/***********************************************************************/

void exeSpatialJoin(void)

{
  Rlint dirComparisons, dataComparisons;
  typinterval *RqRects= allocM(1 * GVrectSize);
  typinterval *R2qRects= allocM(1 * GVrectSize);
  void *qPtr= NULL;
  typfildes fildes;
  char stork[10];
  boolean success;
  
  /*
  RqRects[0][0].l= 0.0;
  RqRects[0][0].h= 0.75;
  RqRects[0][1].l= 0.0;
  RqRects[0][1].h= 0.75;
  
  R2qRects[0][0].l= 0.25;
  R2qRects[0][0].h= 1.0;
  R2qRects[0][1].l= 0.25;
  R2qRects[0][1].h= 1.0;
  */
  /*****/
  CreateTruncF("SpJoin1",&fildes.file1);
  /* ---> CreateFileBuf, pass its reference, then use WrBufBytes! */
  CreateTruncF("SpJoin2",&fildes.file2);
  /* ---> CreateFileBuf, pass its reference, then use WrBufBytes! */
  /*****/
  
  GVNumbCalls= 1; /* this one */
  ClearPath(GVrst);
  ClearPath(GVrst2);
  CountsOn0(GVrst);
  CountsOn0(GVrst2);
  GVCompCount0();

  GVpairCount= 0;
  StartTimers();
  /*
  success= XSpJoin(GVrst,RqRects,1,qPtr,AlwaysTrue,AlwaysTrue,
                   GVrst2,R2qRects,1,qPtr,AlwaysTrue,AlwaysTrue,
                   J_DataIntersects,
                   &fildes,ManageJoin);
  if (! success) {
    printf("FAILURE(XSpJoin)\n");
    freeM(&RqRects); freeM(&R2qRects);
    return;
  }
  */
  /**/
  success= SpJoin(GVrst,GVrst2,&fildes,ManageJoin);
  if (! success) {
    printf("FAILURE(SpJoin)\n");
    freeM(&RqRects); freeM(&R2qRects);
    return;
  }
  /**/
  StopTimers();
  
  /*****/
  CloseF(fildes.file1);
  CloseF(fildes.file2);
  /*****/
  
  printf("\n");
  printf(strans("%s%L\n",s),"Number of pairs found: ",GVpairCount);
  printf("\n");
  if (GVrst2 != GVrst) {
    /* we may have set GVrst2= GVrst to simulate one and the same R*-tree
       identifier for the two trees to be joined, if not: */
    GetCountRectComp(GVrst2,&dirComparisons,&dataComparisons);
    if (dirComparisons != 0 || dataComparisons != 0) {
      printf("FAILURE IN COUNTS:\n");
      printf(strans("dircomp(R2)= %L, datacomp(R2)= %L should be 0 both\n",s),dirComparisons,dataComparisons);
    }
  }
  GetCountRectComp(GVrst,&dirComparisons,&dataComparisons);
  GVdirComparisons+= dirComparisons;
  GVdataComparisons+= dataComparisons;
  PrintGVCompares(GVpairCount);
  if (GVrst2 != GVrst) {
    /*
    PrintReads(GVrst,GVpairCount);
    PrintReads(GVrst2,GVpairCount);
    */
    PrintJoinedReads(GVrst,GVrst2,GVpairCount);
  }
  else {
    PrintReads(GVrst,GVpairCount);
  }
  PrintTimes(GVrst);
  
  if (GVrst2 != GVrst) {
    success= GetStorageKind(GVrst2,stork);
    if (! success) {
      printf("FAILURE(GetMainMemFlag)\n");
      freeM(&RqRects); freeM(&R2qRects);
      return;
    }
    if (strcmp(stork,"pri") == 0) {
      printf("RemoveMainMemRST(R2):\n");
      if (RemoveMainMemRST(&GVrst2)) {
        printf("Done\n");
      }
      else {
        printf("FAILURE(RemoveMainMemRST)\n");
      }
    }
    else {
      printf("CloseRST(R2):\n");
      if (CloseRST(&GVrst2)) {
        if (GVlru2 != NULL) {
          DisposeLRU(&GVlru2);
        }
        printf("Done\n");
      }
      else {
        printf("FAILURE(CloseRST)\n");
      }
    }
  }
  else {
    InitRSTreeIdent(&GVrst2);
  }
  freeM(&RqRects); freeM(&R2qRects);
}

/***********************************************************************/

void ManageJoin(t_RT R1, t_RT R2,
                Rint numbOfDim,
                const typinterval *rectangle1, const typinterval *rectangle2,
                refinfo info1ptr, refinfo info2ptr,
                Rint info1Size, Rint info2Size,
                void *ptr,
                boolean *finish)

{
# define MAX_PAIRS 42
  
  char dummy;
  
  /***** ----- count pairs ----- *****/
  
  GVpairCount++;
  
  /***** ----- prompt the user ----- *****/
  
  //printf(">"); dummy= GetChar();
  
  /***** ----- print rectangle of R1 ----- *****/
  
  //PrintRect(rectangle1);
  
  /***** ----- print info part of R1 ----- *****/
  
  //printf("%15d\n",(*info1ptr).tag);
  //printf("%s\n",&(*info1ptr).c[0]);
  
  /***** ----- print rectangle of R2 ----- *****/
  
  //PrintRect(rectangle2);
  
  /***** ----- print info part of R2 ----- *****/
  
  //printf("%15d\n",(*info2ptr).tag);
  //printf("%s\n",&(*info2ptr).c[0]);
  
  /***** ----- print a counting pair separation ----- *****/
  
  //printf(strans("        ^----- %019L -----^\n",s),GVpairCount);
  
  /***** ----- write rectangles to files ----- *****/
  
  //WrBytes((*(reffildes)ptr).file1,rectangle1,GVrectSize);
  //WrBytes((*(reffildes)ptr).file2,rectangle2,GVrectSize);
  
  /***** ----- finish after finding MAX_PAIRS pairs ----- *****/
  
  //if (GVpairCount == MAX_PAIRS) {
  //  *finish= TRUE;
  //}
  
  
# undef MAX_PAIRS
}

/***********************************************************************/

void exeXJoinCount(void)

{
  Rlint paircount;
  typinterval *RqRects= allocM(1 * GVrectSize);
  typinterval *R2qRects= allocM(1 * GVrectSize);
  void *qPtr= NULL;
  char stork[10];
  boolean success;
  
  /*
  RqRects[0][0].l= 0.0;
  RqRects[0][0].h= 0.75;
  RqRects[0][1].l= 0.0;
  RqRects[0][1].h= 0.75;
  
  R2qRects[0][0].l= 0.25;
  R2qRects[0][0].h= 1.0;
  R2qRects[0][1].l= 0.25;
  R2qRects[0][1].h= 1.0;
  */
  
  GVNumbCalls= 1; /* this one */
  ClearPath(GVrst);
  ClearPath(GVrst2);
  CountsOn0(GVrst);
  CountsOn0(GVrst2);
  GVCompCount0();
  
  StartTimers();
  success= XJoinCount(GVrst,RqRects,0,qPtr,AlwaysTrue,AlwaysTrue,
                      GVrst2,R2qRects,0,qPtr,AlwaysTrue,AlwaysTrue,
                      J_DirIntersects,J_DataIntersects,
                      &paircount);
  if (! success) {
    printf("FAILURE(XJoinCount)\n");
    freeM(&RqRects); freeM(&R2qRects);
    return;
  }
  StopTimers();
  
  printf("\n");
  printf(strans("%s%L\n",s),"Number of pairs found: ",paircount);
  printf("\n");
  PrintGVCompares(paircount);
  if (GVrst2 != GVrst) {
    /*
    PrintReads(GVrst,paircount);
    PrintReads(GVrst2,paircount);
    */
    PrintJoinedReads(GVrst,GVrst2,paircount);
  }
  else {
    PrintReads(GVrst,paircount);
  }
  PrintTimes(GVrst);
  
  if (GVrst2 != GVrst) {
    success= GetStorageKind(GVrst2,stork);
    if (! success) {
      printf("FAILURE(GetMainMemFlag)\n");
      freeM(&RqRects); freeM(&R2qRects);
      return;
    }
    if (strcmp(stork,"pri") == 0) {
      printf("RemoveMainMemRST(R2):\n");
      if (RemoveMainMemRST(&GVrst2)) {
        printf("Done\n");
      }
      else {
        printf("FAILURE(RemoveMainMemRST)\n");
      }
    }
    else {
      printf("CloseRST(R2):\n");
      if (CloseRST(&GVrst2)) {
        if (GVlru2 != NULL) {
          DisposeLRU(&GVlru2);
        }
        printf("Done\n");
      }
      else {
        printf("FAILURE(CloseRST)\n");
      }
    }
  }
  else {
    InitRSTreeIdent(&GVrst2);
  }
  freeM(&RqRects); freeM(&R2qRects);
}

/***********************************************************************/

void exeSpatialJoinCount(void)

{
  Rlint paircount, dirComparisons, dataComparisons;
  typinterval *RqRects= allocM(1 * GVrectSize);
  typinterval *R2qRects= allocM(1 * GVrectSize);
  void *qPtr= NULL;
  char stork[10];
  boolean success;
  
  /*
  RqRects[0][0].l= 0.0;
  RqRects[0][0].h= 0.75;
  RqRects[0][1].l= 0.0;
  RqRects[0][1].h= 0.75;
  
  R2qRects[0][0].l= 0.25;
  R2qRects[0][0].h= 1.0;
  R2qRects[0][1].l= 0.25;
  R2qRects[0][1].h= 1.0;
  */
  
  GVNumbCalls= 1; /* this one */
  ClearPath(GVrst);
  ClearPath(GVrst2);
  CountsOn0(GVrst);
  CountsOn0(GVrst2);
  GVCompCount0();
  
  StartTimers();
  /*
  success= XSpJoinCount(GVrst,RqRects,0,qPtr,AlwaysTrue,AlwaysTrue,
                        GVrst2,R2qRects,0,qPtr,AlwaysTrue,AlwaysTrue,
                        J_DataIntersects,
                        &paircount);
  if (! success) {
    printf("FAILURE(XSpJoinCount)\n");
    freeM(&RqRects); freeM(&R2qRects);
    return;
  }
  */
  /**/
  success= SpJoinCount(GVrst,GVrst2,&paircount);
  if (! success) {
    printf("FAILURE(SpJoinCount)\n");
    freeM(&RqRects); freeM(&R2qRects);
    return;
  }
  /**/
  StopTimers();
  
  printf("\n");
  printf(strans("%s%L\n",s),"Number of pairs found: ",paircount);
  printf("\n");
  if (GVrst2 != GVrst) {
    /* we may have set GVrst2= GVrst to simulate one and the same R*-tree
       identifier for the two trees to be joined, if not: */
    GetCountRectComp(GVrst2,&dirComparisons,&dataComparisons);
    if (dirComparisons != 0 || dataComparisons != 0) {
      printf("FAILURE IN COUNTS:\n");
      printf(strans("dircomp(R2)= %L, datacomp(R2)= %L should be 0 both\n",s),dirComparisons,dataComparisons);
    }
  }
  GetCountRectComp(GVrst,&dirComparisons,&dataComparisons);
  GVdirComparisons+= dirComparisons;
  GVdataComparisons+= dataComparisons;
  PrintGVCompares(paircount);
  if (GVrst2 != GVrst) {
    /*
    PrintReads(GVrst,paircount);
    PrintReads(GVrst2,paircount);
    */
    PrintJoinedReads(GVrst,GVrst2,paircount);
  }
  else {
    PrintReads(GVrst,paircount);
  }
  PrintTimes(GVrst);
  
  if (GVrst2 != GVrst) {
    success= GetStorageKind(GVrst2,stork);
    if (! success) {
      printf("FAILURE(GetMainMemFlag)\n");
      freeM(&RqRects); freeM(&R2qRects);
      return;
    }
    if (strcmp(stork,"pri") == 0) {
      printf("RemoveMainMemRST(R2):\n");
      if (RemoveMainMemRST(&GVrst2)) {
        printf("Done\n");
      }
      else {
        printf("FAILURE(RemoveMainMemRST)\n");
      }
    }
    else {
      printf("CloseRST(R2):\n");
      if (CloseRST(&GVrst2)) {
        if (GVlru2 != NULL) {
          DisposeLRU(&GVlru2);
        }
        printf("Done\n");
      }
      else {
        printf("FAILURE(CloseRST)\n");
      }
    }
  }
  else {
    InitRSTreeIdent(&GVrst2);
  }
  freeM(&RqRects); freeM(&R2qRects);
}

/***********************************************************************/
/* NOTE that RSTStdTypes.h defines "Infinity" */

void exeNNearestTest(void)

{
  Rlint numbrecs, reccount, allnbrs, PQelems;
  int i, d;
  Rint height, PQlen, PQmax, maxPQmax, minPQmax;
  boolean success, exists, queryDone, first;
  typinterval *qpRect= allocM(GVrectSize);
  typcoord *qp= allocM(GVpointSize);
  t_DQ distQ;
  typinterval *rectangle= allocM(GVrectSize);
  refinfo info= allocM(GVinfoSize);
  Rfloat rawDist;
  
  printf("Neighbors per call (0 = all): ");
  scanf(strans("%L",s),&numbrecs);
  GVdummyCh= GetChar();
  ClearPath(GVrst);
  CountsOn0(GVrst);
  
  i= GVbegin;
  allnbrs= 0;
  first= TRUE;
  InitDistQueryIdent(&distQ);
  StartTimers();
  do {
    if (! RdTextToRect(GVquerFile,qpRect)) {
      DisplErr(GVquerName,i);
      freeM(&qpRect); freeM(&qp); freeM(&rectangle); freeM(&info);
      return;
    }
    for (d= 0; d < GVnumbOfDim; d++) {
      qp[d]= qpRect[d].l;
    }
    i++;
    success= NewDistQuery(GVrst,qp,CalcEucl,inc,minDist,0,NULL,0,NULL,AlwaysTrue,AlwaysTrue,2048,TRUE,&distQ);
    if (! success) {
      printf("FAILURE(NewDistQuery)\n");
      freeM(&qpRect); freeM(&qp); freeM(&rectangle); freeM(&info);
      return;
    }
    
    reccount= 0;
    queryDone= FALSE;
    do {
      reccount++;
      success= GetDistQueryRec(distQ,GVrst,rectangle,info,&rawDist,&exists);
      if (! success) {
        printf("FAILURE(GetDistQueryRec)\n");
        freeM(&qpRect); freeM(&qp); freeM(&rectangle); freeM(&info);
        return;
      }
      /* Begin print record: */
      //printf(strans("%3d. Query   %3L. Record:\n",s),i,reccount);
      //for (d= 0; d < GVnumbOfDim; d++) {
      //  printf("%+15f     %+15f\n",rectangle[d].l,rectangle[d].h);
      //}
      //printf("info.tag: %06d\n",(*info).tag);
      //printf("distance: %f\n",sqrt(rawDist));
      /* End   print record: */
      if (! exists) {
        reccount--;
        queryDone= TRUE;
      }
      else {
        if (reccount == numbrecs) {
          queryDone= TRUE;
        }
      }
    } while (!queryDone);
    success= DisposeDistQuery(&distQ);
    if (! success) {
      printf("FAILURE(DisposeDistQuery)\n");
      freeM(&qpRect); freeM(&qp); freeM(&rectangle); freeM(&info);
      return;
    }
    GetCountPriorQ(GVrst,&PQlen,&PQmax,&PQelems);
    if (first) {
      minPQmax= PQmax;
      maxPQmax= PQmax;
      first= FALSE;
    }
    else {
      if (PQmax < minPQmax) {
        minPQmax= PQmax;
      }
      if (PQmax > maxPQmax) {
        maxPQmax= PQmax;
      }
    }
    allnbrs+= reccount;
    if ((i % GVfdBckMod == 0) || (i == GVend)) {
      printf("%6d%s",i,"::  ");
      GetHeight(GVrst,&height);
      printf("%s%3d","height:",height);
      printf(strans("%s%6L\n",s),"  neighbors:",allnbrs);
    }
  } while (i != GVend);
  StopTimers();

  printf("\n");
  printf("Minimum maximum priority queue length: %d\n",minPQmax);
  printf("Maximum maximum priority queue length: %d\n",maxPQmax);
  printf("\n");
  PrintCalls(GVNumbCalls);
  PrintReads(GVrst,allnbrs);
  PrintPQelems(GVrst,allnbrs);
  PrintTimes(GVrst);
  freeM(&qpRect); freeM(&qp); freeM(&rectangle); freeM(&info);
}

/***********************************************************************/
/* NOTE that RSTStdTypes.h defines "Infinity" */

void exeDistanceQuery(void)

{
  char kind, ch;
  char sortName[20], distName[20];
  Rlint numbrecs, reccount, PQelems;
  int d;
  Rint PQlen, PQmax;
  boolean success, exists, clipActive, queryDone;
  typcoord *qp= allocM(GVpointSize);
  DistQuerySort sort;
  RectDistType distType;
  double stDist, endDist;
  typinterval *fRects= allocM(1 * GVrectSize);
  t_DQ distQ;
  typinterval *rectangle= allocM(GVrectSize);
  refinfo info= allocM(GVinfoSize);
  Rfloat rawDist;
  typfildes fildes;
  
  do {
    ClearPath(GVrst);
    CountsOn0(GVrst);
    
    printf("\n");
    printf("   inc, minDist = \"1\"   dec, minDist = \"2\"\n");
    printf("  inc, cntrDist = \"3\"  dec, cntrDist = \"4\"\n");
    printf("   inc, maxDist = \"5\"   dec, maxDist = \"6\"\n");
    printf("Choice: ");
    do {
      kind= GetChar();
    } while ((kind < '1') || (kind > '6'));
    GVdummyCh= GetChar();
    
    if (kind == '1') {
      sort= inc;
      distType= minDist;
      sprintf(sortName,"nearest");
      sprintf(distName,"Distance");
    }
    else if (kind == '2') {
      sort= dec;
      distType= minDist;
      sprintf(sortName,"farthest");
      sprintf(distName,"Distance");
    }
    else if (kind == '3') {
      sort= inc;
      distType= cntrDist;
      sprintf(sortName,"nearest");
      sprintf(distName,"CntrDist");
    }
    else if (kind =='4') {
      sort= dec;
      distType= cntrDist;
      sprintf(sortName,"farthest");
      sprintf(distName,"CntrDist");
    }
    else if (kind == '5') {
      sort= inc;
      distType= maxDist;
      sprintf(sortName,"inside");
      sprintf(distName," MaxDist");
    }
    else if (kind == '6') {
      sort= dec;
      distType= maxDist;
      sprintf(sortName,"not inside");
      sprintf(distName," MaxDist");
    }
    else {
      printf("USER INPUT SCAN ERROR\n");
      freeM(&qp); freeM(&fRects); freeM(&rectangle); freeM(&info);
      return;
    }
    
    ReadDistQParams(qp,&stDist,&endDist,fRects,&numbrecs);
    printf("\n");
    
    InitDistQueryIdent(&distQ);
    clipActive= FALSE;
    for (d= 0; d < GVnumbOfDim; d++) {
      if (fRects[d].l != 0.0 || fRects[d].h != 0.0) {
        clipActive= TRUE; break;
      }
    }
    if (clipActive) {
      success= NewDistQuery(GVrst,qp,CalcEucl,sort,distType,stDist,fRects,1,NULL,DirIntersects,DataIntersects,2048,TRUE,&distQ);
    }
    else {
      success= NewDistQuery(GVrst,qp,CalcEucl,sort,distType,stDist,NULL,0,NULL,AlwaysTrue,AlwaysTrue,2048,TRUE,&distQ);
    }
    if (! success) {
      printf("FAILURE(NewDistQuery)\n");
      freeM(&qp); freeM(&fRects); freeM(&rectangle); freeM(&info);
      return;
    }
    
    /***** ----- create binary rectangle file ----- *****/
    /**/
    CreateTruncF("DistanceQuery",&fildes.file1);
    /* ---> CreateFileBuf, pass its reference, then use WrBufBytes! */
    /**/
    
    reccount= 0;
    queryDone= FALSE;
    do {
      reccount++;
      success= GetDistQueryRec(distQ,GVrst,rectangle,info,&rawDist,&exists);
      if (! success) {
        printf("FAILURE(GetDistQueryRec)\n");
        freeM(&qp); freeM(&fRects); freeM(&rectangle); freeM(&info);
        return;
      }
      if (! exists) {
        reccount--;
        printf(strans("%L. %s: WAS LAST RECORD, QUERY IS EXHAUSTED\n",s),reccount,sortName);
        queryDone= TRUE;
      }
      else {
        if (endDist != 0.0) {
          if (kind % 2 == 1) {
            if (rawDist > endDist) {
              printf("End -------------------------------------------------------------- End\n");
              queryDone= TRUE;
            }
          }
          else {
            if (rawDist < endDist) {
              printf("End -------------------------------------------------------------- End\n");
              queryDone= TRUE;
            }
          }
        }
        if (reccount == numbrecs) {
          queryDone= TRUE;
        }
        /***** ----- print record ----- *****/
        /**/
        printf(strans("%L. %s:\n",s),reccount,sortName);
        PrintDistQueryRec(rectangle,info,rawDist,distName);
        /**/
        /***** ----- write rectangles binary to file ----- *****/
        /*
        WrBytes(fildes.file1,rectangle,GVrectSize);
        */
        /***** ----- monitor priority queue length ----- *****/
        /*
        success= GetCountPriorQ(GVrst,&PQlen,&PQmax,&PQelems);
        if (! success) {
          printf("FAILURE(GetCountPriorQ)\n");
          freeM(&qp); freeM(&fRects); freeM(&rectangle); freeM(&info);
          return;
        }
        printf("Priority queue: Length: %6d Max: %6d\n",PQlen,PQmax);
        */
      }
    } while (!queryDone);
    success= DisposeDistQuery(&distQ);
    if (! success) {
      printf("FAILURE(DisposeDistQuery)\n");
      freeM(&qp); freeM(&fRects); freeM(&rectangle); freeM(&info);
      return;
    }
    
    /***** ----- close binary rectangle file ----- *****/
    /**/
    CloseF(fildes.file1);
    /**/
    
    printf("\n");
    printf(strans("%s%L\n",s),"Number of records found: ",reccount);
    printf("\n");
    PrintReads(GVrst,reccount);
    PrintPQelems(GVrst,reccount);
    
    printf("\nagain?, (y/n) ");
    do {
      ch= GetChar();
    } while ((ch != 'y') && (ch != 'n'));
    GVdummyCh= GetChar();
  } while (ch != 'n');
  freeM(&qp); freeM(&fRects); freeM(&rectangle); freeM(&info);
}

/***********************************************************************/

void CalcEucl (Rfloat coordDist, Rfloat *cumuDist)

{
  *cumuDist+= coordDist * coordDist;
}

/***********************************************************************/

void ReadDistQParams(typcoord *qp, double *stDist, double *endDist, typinterval *clipRect, Rlint *number)

{
  int d;
  double rad;
  
  printf("  %d query point coordinates: ",GVnumbOfDim);
  for (d= 0; d < GVnumbOfDim; d++) {
    scanf("%lf",&qp[d]);
  }
  printf("   startDist (0 = disabled): ");
  scanf("%lf",&rad);
  *stDist= rad * rad;
  printf("     endDist (0 = disabled): ");
  scanf("%lf",&rad);
  *endDist= rad * rad;
  printf("                   clipRect: %d intervals low, high\n",GVnumbOfDim);
  printf("%d values (all 0 = disabled): ",GVnumbOfDim*2);
  for (d= 0; d < GVnumbOfDim; d++) {
    scanf("%lf%lf",&clipRect[d].l,&clipRect[d].h);
  }
  printf("    #records (0 = disabled): ");
  scanf(strans("%L",s),number);
  GVdummyCh= GetChar();
}

/***********************************************************************/

void PrintDistQueryRec(typinterval *rect,
                       refinfo info,
                       Rfloat rawDist,
                       char distName[])

{
  int d;
  
  for (d= 0; d < GVnumbOfDim; d++) {
    printf("% 15f% 15f",rect[d].l,rect[d].h);
    if (d < GVnumbOfDim - 1) {
      printf("\n");
    }
    else {
      printf("%16s: % .7e",distName,sqrt(rawDist));
      printf("%12d",(*info).tag);
      printf("\n");
      /* --- print the squared distance too:
      printf("%30squared %s: % .7e","",distName,rawDist);
      printf("\n");
      */
    }
  }
}

/***********************************************************************/

boolean exeCheckConsistency(boolean verbose)

{
  Rint maxDirFanout, maxDataFanout;
  Rlint numbnodes;
  Rint rootLevel;
  int d, lv;
  Rint upperLevel, lowerLevel;
  Rpnint upperPage, lowerPage;
  boolean success, consistent, rootMBBok, otherMBBsOk;
  typinterval *upperRect= allocM(GVrectSize);
  typinterval *lowerRect= allocM(GVrectSize);
  typinterval *storedRootRect= allocM(GVrectSize);
  typinterval *rootsRect= allocM(GVrectSize);
  Rint path[50];
  Rpnint nodecount[50];
  
  success= GetMaxFanout(GVrst,&maxDirFanout,&maxDataFanout);
  if (! success) {
    printf("FAILURE(GetMaxFanout)\n");
    freeM(&upperRect); freeM(&lowerRect);
    freeM(&storedRootRect); freeM(&rootsRect);
    return FALSE;
  }
  GetRootLevel(GVrst,&rootLevel);
  if (! success) {
    printf("FAILURE(GetRootLevel)\n");
    freeM(&upperRect); freeM(&lowerRect);
    freeM(&storedRootRect); freeM(&rootsRect);
    return FALSE;
  }
  
  if (verbose) {
    printf("height: %d\n",rootLevel+1);
    printf("CHECKING: Consistency of all MBBs in the tree\n");
  }
  success= CheckConsistency(GVrst,
                            &consistent,
                            &rootMBBok,
                            &otherMBBsOk,
                            storedRootRect,
                            rootsRect,
                            path,
                            &upperLevel,
                            upperRect,
                            &upperPage,
                            &lowerLevel,
                            lowerRect,
                            &lowerPage,
                            nodecount);
  if (! success) {
    printf("FAILURE(CheckConsistency)\n");
    freeM(&upperRect); freeM(&lowerRect);
    freeM(&storedRootRect); freeM(&rootsRect);
    return FALSE;
  }
  
  if (consistent) {
    if (verbose) {
      printf("\n --- all MBBs are consistent ---\n\n");
      printf("Number of nodes per level:\n");
      numbnodes= 0;
      for (lv= rootLevel; lv >= 0; lv--) {
        printf(strans("%10N",s),nodecount[lv]);
        numbnodes+= nodecount[lv];
      }
      printf("\n");
      printf(strans("Overall number of nodes: %L\n",s),numbnodes);
      printf("Last path(#entry[0..%d]) checked:\n",maxDirFanout-1);
      for (lv= rootLevel; lv > 0; lv--) {     /* deepest pointer level = 1 */
        printf("%10d",path[lv]);
      }
      printf("\n");
    }
  }
  else {
    if (rootMBBok) {
      printf("\n --- the root MBB is consistent ---\n\n");
    }
    else {
      printf("\n ##### INCONSISTENT root MBB #####\n\n");
      printf("Registered rectangle:\n");
      for (d= 0; d < GVnumbOfDim; d++) {
        printf("%24.16e%24.16e",storedRootRect[d].l,storedRootRect[d].h);
        printf("\n");
      }
      printf("Actual enclosing rectangle of the root:\n");
      for (d= 0; d < GVnumbOfDim; d++) {
        printf("%24.16e%24.16e",rootsRect[d].l,rootsRect[d].h);
        printf("\n");
      }
    }
    if (otherMBBsOk) {
      printf("\n --- the other MBBs are consistent ---\n\n");
    }
    else {
      printf("\n ##### INCONSISTENT MBBs #####\n\n");
      printf("in levels: %d/%d\n",upperLevel,lowerLevel);
      printf("of the following path(#entry[0..%d]):\n",maxDirFanout-1);
      for (lv= rootLevel; lv >= upperLevel; lv--) { /* deepest pointer level = 1 */
        printf("%10d",path[lv]);
      }
      printf("\n");
      printf("Registered rectangle:\n");
      for (d= 0; d < GVnumbOfDim; d++) {
        printf("%24.16e%24.16e",upperRect[d].l,upperRect[d].h);
        if (d < GVnumbOfDim - 1) {
          printf("\n");
        }
        else {
          printf(strans("     page: %N\n",s),upperPage);
        }
      }
      printf("Actual enclosing rectangle of the child-node:\n");
      for (d= 0; d < GVnumbOfDim; d++) {
        printf("%24.16e%24.16e",lowerRect[d].l,lowerRect[d].h);
        if (d < GVnumbOfDim - 1) {
          printf("\n");
        }
        else {
          printf(strans("     page: %N\n",s),lowerPage);
        }
      }
      printf("Tracing state(number of nodes):\n");
      for (lv= rootLevel; lv >= 0; lv--) {
        printf(strans("%10N",s),nodecount[lv]);
      }
      printf("\n");
    }
  }
  freeM(&upperRect); freeM(&lowerRect);
  freeM(&storedRootRect); freeM(&rootsRect);
  return consistent;
}

/***********************************************************************/

void exePathsDump(void)

{
  int success;
  
  success= PathsDump(GVrst);
  if (! success) {printf("FAILURE(PathsDump)\n"); return;}
}

/***********************************************************************/

void exeASCIIdump(void)

{
  int success;
  char ch;
  char name[MaxNameLength];
  FILE *stream;
  
  strlcpy(name,GVdistName,sizeof(name));
  strlcat(name,ASCII_SUFF,sizeof(name));
  printf("Dump to file \"%s\" or to stdout (f/-)? ",name);
  do {
    ch= GetChar();
  } while ((ch != 'f') && (ch != '-'));
  GVdummyCh= GetChar();
  if (ch == 'f') {
    printf("Writing %s\n",name);
    stream= fopen(name,"w");
    success= ASCIIdump(GVrst,stream);
    if (success) {
      fclose(stream);
      printf("Done\n");
    }
  }
  else if (ch == '-') {
    success= ASCIIdump(GVrst,stdout);
  }
  if (! success) {printf("FAILURE(ASCIIdump)\n"); return;}
}

/***********************************************************************/

void exeDirLevelDump(void)

{
  int success;
  typinterval *rectBuf= allocM(GVrectSize);
  
  success= DirLevelDump(GVrst,rectBuf,GVrectSize,ConvRectForDump);
  freeM(&rectBuf);
  if (! success) {printf("FAILURE(DirLevelDump)\n"); return;}
}

/***********************************************************************/

void ConvRectForDump(t_RT R,
                     Rint numbOfDim,
                     const typinterval *rstrect,
                     void *buf,
                     Rint bufSize)

{
  typinterval *rect= buf;
  int d;
  
  for (d= 0; d < numbOfDim; d++) {
    rect[d]= rstrect[d];
  }
}

/***********************************************************************/

void exeMediaReorg(void)

{
  boolean reorganized, truncated;
  printf("Performing media reorganization ...\n");
  
  GVNumbCalls= 1; /* this one */
  CountsOn0(GVrst);
  
  StartTimers();
  if (ReorganizeMedium(GVrst,TRUE,&reorganized,&truncated)) {
    printf("Done\n");
  }
  else {
    printf("FAILURE(ReorganizeMedium)\n");
    return;
  }
  StopTimers();
  
  printf("\n");
  PrintReads(GVrst,-1);
  PrintWrites(GVrst,-1);
  PrintAccesses(GVrst,-1);
  PrintTimes(GVrst);
}

/***********************************************************************/

void exeInquire(void)

{
  char name[MaxNameLength];
  Rint RintSize, RpintSize, RpnintSize, RlintSize, RfloatSize,
       subtreePtrSize, infoSize, numbOfDimensions,
       dirPageSize, dataPageSize, netDirPageSize, netDataPageSize,
       dirEntrySize, dataEntrySize,
       maxDirFanout, maxDataFanout, minDirFanout, minDataFanout,
       minDirDELrest, minDataDELrest;
  Rpnint numbOfDirPages, numbOfDataPages;
  Rlint numbOfRecords;
  Rint rootLevel;
  boolean unique;
  Rpnint pagesPerLevel[50];
  double spaceutil[50];
  double sumspaceutil;
  boolean success;
  int lv;
  
  success= InquireRSTDesc(GVrst,
                          name,
                          &subtreePtrSize,
                          &infoSize,
                          &numbOfDimensions,
                          &dirPageSize,
                          &dataPageSize,
                          &netDirPageSize,
                          &netDataPageSize,
                          &dirEntrySize,
                          &dataEntrySize,
                          &maxDirFanout,
                          &maxDataFanout,
                          &minDirFanout,
                          &minDataFanout,
                          &minDirDELrest,
                          &minDataDELrest,
                          &numbOfDirPages,
                          &numbOfDataPages,
                          &numbOfRecords,
                          &rootLevel,
                          &unique,
                          pagesPerLevel);
  if (! success) {printf("FAILURE(InquireRSTDesc)\n"); return;}
  printf("%20s%s\n","name: ",name);
  printf("%20s%d\n","subtreePtrSize: ",subtreePtrSize);
  printf("%20s%d\n","infoSize: ",infoSize);
  printf("%20s%d\n","numbOfDimensions: ",numbOfDimensions);
  printf("%20s%d\n","dirPageSize: ",dirPageSize);
  printf("%20s%d\n","dataPageSize: ",dataPageSize);
  printf("%20s%d\n","netDirPageSize: ",netDirPageSize);
  printf("%20s%d\n","netDataPageSize: ",netDataPageSize);
  printf("%20s%d\n","dirEntrySize: ",dirEntrySize);
  printf("%20s%d\n","dataEntrySize: ",dataEntrySize);
  printf("%20s%d\n","maxDirFanout: ",maxDirFanout);
  printf("%20s%d\n","maxDataFanout: ",maxDataFanout);
  printf("%20s%d\n","minDirFanout: ",minDirFanout);
  printf("%20s%d\n","minDataFanout: ",minDataFanout);
  printf("%20s%d\n","minDirDELrest: ",minDirDELrest);
  printf("%20s%d\n","minDataDELrest: ",minDataDELrest);
  printf(strans("%20s%d\n",s),"numbOfDirPages: ",numbOfDirPages);
  printf(strans("%20s%d\n",s),"numbOfDataPages: ",numbOfDataPages);
  printf(strans("%20s%L\n",s),"numbOfPages: ",(Rlint)numbOfDirPages+numbOfDataPages);
  printf(strans("%20s%L\n",s),"numbOfRecords: ",numbOfRecords);
  printf("%20s%d\n","rootLevel: ",rootLevel);
  printf("%20s%d\n","unique: ",unique);
  printf("pages per level:\n");
  for(lv= rootLevel; lv >= 0; lv--) {
    printf(strans("%10N",s),pagesPerLevel[lv]);
  }
  printf("\n");
  for(lv= rootLevel; lv > 0; lv--) {
    spaceutil[lv]= (double)pagesPerLevel[lv-1] / (double)(pagesPerLevel[lv] * maxDirFanout);
  }
  spaceutil[0]= (double)(numbOfRecords) / (double)(pagesPerLevel[0] * maxDataFanout);
  printf("Space utilization:\n");
  for(lv= rootLevel; lv >= 0; lv--) {
    printf("%.2e ",spaceutil[lv]);
  }
  printf("\n");
  sumspaceutil= 0.0;
  for (lv= 1; lv < rootLevel; lv++) {
    sumspaceutil= sumspaceutil + spaceutil[lv];
  }
  printf("%s%.2e\n","    avg spc util dir (without root): ",
  sumspaceutil / (rootLevel - 1));
  sumspaceutil= sumspaceutil + spaceutil[0];
  printf("%s%.2e\n","avg spc util overall (without root): ",
  sumspaceutil / rootLevel);
}

/***********************************************************************/

void exeExamDescFile(void)

{
  char name[MaxNameLength];
  boolean success;
  printf("%s","\nDescription File (full name): ");
  scanf("%s",name);
  GVdummyCh= GetChar();
  success= ExamRSTDescFile(name);
  if (! success) {printf("FAILURE(ExamRSTDescFile)\n"); return;}
}

/***********************************************************************/

void exeComputePageSizes(void)

{
#define DIR_P_SIZE 65536
#define DATA_P_SIZE 65536
#define DIR_RAM_SIZE (3 * DIR_P_SIZE)	/* at least 3 */
#define DATA_RAM_SIZE (3 * DATA_P_SIZE)	/* at least 3 */
  t_RT R;
  boolean success;
  Rint dummy;
  Rint numbDim, infoLen, dirNodePref, dataNodePref;
  
  Rint subtreePtrSize, infoSize, numbOfDimensions,
       dirPageSize, dataPageSize,
       minDirPageSize, minDataPageSize,
       netDirPageSize, netDataPageSize,
       dirEntrySize, dataEntrySize;
  Rint maxDirFanout, maxDataFanout;
  Rint entryqty, byteQty, dirByteQty, dataByteQty;
  Rint calcDirNodeSize, calcDataNodeSize;
  char again;
  
  printf("-------------------- Begin computing page sizes:\n");
  InitRSTreeIdent(&R);
  do {
    printf("        Number of dimensions: ");
    scanf("%d",&numbDim);
    printf("        Size of an info part: ");
    scanf("%d",&infoLen);
    success= CreateMainMemRST(&R,
                              DIR_P_SIZE,
                              DATA_P_SIZE,
                              numbDim,
                              infoLen,
                              TRUE,
                              DIR_RAM_SIZE,
                              DATA_RAM_SIZE,
                              TRUE);
    if (! success) { break; }
    success= GetCapParams(R,
                          &subtreePtrSize,&infoSize,
                          &numbOfDimensions,
                          &dirPageSize,&dataPageSize,
                          &netDirPageSize,&netDataPageSize,
                          &dirEntrySize, &dataEntrySize);
    success= RemoveMainMemRST(&R);
    printf("     sizeof(subtree pointer):      %5d\n",subtreePtrSize);
    printf("           sizeof(info part):      %5d\n",infoSize);
    printf("   Aligned sizeof(dir entry):      %5d\n",dirEntrySize);
    printf("  Aligned sizeof(data entry):      %5d\n",dataEntrySize);
    dirNodePref= dirPageSize - netDirPageSize;
    dataNodePref= dataPageSize - netDataPageSize;
    printf("     Aligned dir page prefix:      %5d\n",dirNodePref);
    printf("    Aligned data page prefix:      %5d\n",dataNodePref);
    minDirPageSize= dirNodePref + 3 * dirEntrySize;
    minDataPageSize= dataNodePref + dataEntrySize;
    printf("---------------------------------- Given these parameters ...\n");
    printf("   Enter a number of entries: ");
    scanf("%d",&entryqty);
    calcDirNodeSize= dirNodePref + entryqty * dirEntrySize;
    calcDataNodeSize= dataNodePref + entryqty * dataEntrySize;
    printf("               Dir page size:      %5d\n",calcDirNodeSize);
    printf("              Data page size:      %5d\n",calcDataNodeSize);
    printf("  Enter a page size in bytes: ");
    scanf("%d",&byteQty);
    if (byteQty < minDirPageSize) {
      printf("##### Page size too small for dir pages!\n");
      dirByteQty= minDirPageSize;
      printf("##### Minimum (%d) applied!\n",dirByteQty);
    }
    else {
      dirByteQty= byteQty;
    }
    if (byteQty < minDataPageSize) {
      printf("##### Page size too small for data pages!\n");
      dataByteQty= minDataPageSize;
      printf("##### Minimum (%d) applied!\n",dataByteQty);
    }
    else {
      dataByteQty= byteQty;
    }
    success= CreateMainMemRST(&R,
                              dirByteQty,
                              dataByteQty,
                              numbDim,
                              infoLen,
                              TRUE,
                              3 * dirByteQty,
                              3 * dataByteQty,
                              TRUE);
    if (! success) { break; }
    success= GetCapParams(R,
                          &dummy,&dummy,&dummy,
                          &dirPageSize,&dataPageSize,
                          &netDirPageSize,&netDataPageSize,
                          &dummy,&dummy);
    success= GetMaxFanout(R,&maxDirFanout,&maxDataFanout);
    success= RemoveMainMemRST(&R);
    printf("               Dir page size:      %5d\n",dirPageSize);
    printf("              Data page size:      %5d\n",dataPageSize);
    printf("           Net dir page size:      %5d\n",netDirPageSize);
    printf("          Net data page size:      %5d\n",netDataPageSize);
    printf("  Number of dir page entries:      %5d\n",maxDirFanout);
    printf(" Number of data page entries:      %5d\n",maxDataFanout);
    printf("\nagain?, (y/n): ");
    do {
      again= GetChar();
    } while ((again != 'y') && (again != 'n'));
    GVdummyCh= GetChar();
  } while (again == 'y');
  printf("--------------------   End computing page sizes.\n");
}

/***********************************************************************/

boolean DirEqual(t_RT rt,
                 Rint numbOfDim,
                 const typinterval *RSTrect,
                 const typinterval *qRects,
                 Rint qRectQty,
                 void *qPtr)

{
  int d;
  
  GVdirComparisons++;
  d= 0;
  do {
    if ((*RSTrect).l != (*qRects).l || (*RSTrect).h != (*qRects).h) {
      return FALSE;
    }
    RSTrect++; qRects++;
    d++;
  } while (d < numbOfDim);
  return TRUE;
}

/***********************************************************************/

boolean DataEqual(t_RT rt,
                  Rint numbOfDim,
                  const typinterval *RSTrect,
                  const typinterval *qRects,
                  Rint qRectQty,
                  void *qPtr)

{
  int d;
  
  GVdataComparisons++;
  d= 0;
  do {
    if ((*RSTrect).l != (*qRects).l || (*RSTrect).h != (*qRects).h) {
      return FALSE;
    }
    RSTrect++; qRects++;
    d++;
  } while (d < numbOfDim);
  return TRUE;
}

/***********************************************************************/

boolean DirIntersects(t_RT rt,
                      Rint numbOfDim,
                      const typinterval *RSTrect,
                      const typinterval *qRects,
                      Rint qRectQty,
                      void *qPtr)

{
  int d;
  
  GVdirComparisons++;
  d= 0;
  do {
    if ((*RSTrect).l > (*qRects).h || (*RSTrect).h < (*qRects).l) {
      return FALSE;
    }
    RSTrect++; qRects++;
    d++;
  } while (d < numbOfDim);
  return TRUE;
}

/***********************************************************************/

boolean DataIntersects(t_RT rt,
                       Rint numbOfDim,
                       const typinterval *RSTrect,
                       const typinterval *qRects,
                       Rint qRectQty,
                       void *qPtr)

{
  int d;
  
  GVdataComparisons++;
  d= 0;
  do {
    if ((*RSTrect).l > (*qRects).h || (*RSTrect).h < (*qRects).l) {
      return FALSE;
    }
    RSTrect++; qRects++;
    d++;
  } while (d < numbOfDim);
  return TRUE;
}

/***********************************************************************/

boolean DirEncloses(t_RT rt,
                    Rint numbOfDim,
                    const typinterval *RSTrect,
                    const typinterval *qRects,
                    Rint qRectQty,
                    void *qPtr)

{
  int d;
  
  GVdirComparisons++;
  d= 0;
  do {
    if ((*RSTrect).l > (*qRects).l || (*RSTrect).h < (*qRects).h) {
      return FALSE;
    }
    RSTrect++; qRects++;
    d++;
  } while (d < numbOfDim);
  return TRUE;
}

/***********************************************************************/

boolean DataEncloses(t_RT rt,
                     Rint numbOfDim,
                     const typinterval *RSTrect,
                     const typinterval *qRects,
                     Rint qRectQty,
                     void *qPtr)

{
  int d;
  
  GVdataComparisons++;
  d= 0;
  do {
    if ((*RSTrect).l > (*qRects).l || (*RSTrect).h < (*qRects).h) {
      return FALSE;
    }
    RSTrect++; qRects++;
    d++;
  } while (d < numbOfDim);
  return TRUE;
}

/***********************************************************************/

boolean DirIsContained(t_RT rt,
                       Rint numbOfDim,
                       const typinterval *RSTrect,
                       const typinterval *qRects,
                       Rint qRectQty,
                       void *qPtr)

{
  int d;
  
  GVdirComparisons++;
  d= 0;
  do {
    if ((*RSTrect).l < (*qRects).l || (*RSTrect).h > (*qRects).h) {
      return FALSE;
    }
    RSTrect++; qRects++;
    d++;
  } while (d < numbOfDim);
  return TRUE;
}

/***********************************************************************/

boolean DataIsContained(t_RT rt,
                        Rint numbOfDim,
                        const typinterval *RSTrect,
                        const typinterval *qRects,
                        Rint qRectQty,
                        void *qPtr)

{
  int d;
  
  GVdataComparisons++;
  d= 0;
  do {
    if ((*RSTrect).l < (*qRects).l || (*RSTrect).h > (*qRects).h) {
      return FALSE;
    }
    RSTrect++; qRects++;
    d++;
  } while (d < numbOfDim);
  return TRUE;
}

/***********************************************************************/

boolean AlwaysTrue(t_RT rt,
                   Rint numbOfDim,
                   const typinterval *RSTrect,
                   const typinterval *qRects,
                   Rint qRectQty,
                   void *qPtr)

{
  return TRUE;
}

/***********************************************************************/

boolean J_DirIntersects(t_RT rt1,
                        t_RT rt2,
                        Rint numbOfDim,
                        const typinterval *RST1rect,
                        const typinterval *RST2rect)

{
  int d;
  
  GVdirComparisons++;
  d= 0;
  do {
    if ((*RST1rect).l > (*RST2rect).h || (*RST1rect).h < (*RST2rect).l) {
      return FALSE;
    }
    RST1rect++; RST2rect++;
    d++;
  } while (d < numbOfDim);
  return TRUE;
}

/***********************************************************************/

boolean J_DataIntersects(t_RT rt1,
                         t_RT rt2,
                         Rint numbOfDim,
                         const typinterval *RST1rect,
                         const typinterval *RST2rect)

{
  int d;
  
  GVdataComparisons++;
  d= 0;
  do {
    if ((*RST1rect).l > (*RST2rect).h || (*RST1rect).h < (*RST2rect).l) {
      return FALSE;
    }
    RST1rect++; RST2rect++;
    d++;
  } while (d < numbOfDim);
  return TRUE;
}

/***********************************************************************/

void GVCompCount0(void)

{
  GVdirComparisons= 0;
  GVdataComparisons= 0;
}

/***********************************************************************/
/***********************************************************************/
/* counted parameters output functions */

void PrintLRUStatistics(t_LRU lru) {

  Rlint nReads, nNews, nWrites, nAvails;
  
  LRUGetCountRead(lru,&nReads);
  LRUGetCountNew(lru,&nNews);
  LRUGetCountWrite(lru,&nWrites);
  LRUGetCountAvail(lru,&nAvails);
  fprintf(stdout,strans("LRU: pages read:                  %10L\n",s),nReads);
  fprintf(stdout,strans("LRU: pages newly established:     %10L\n",s),nNews);
  fprintf(stdout,strans("LRU: pages written (incl. Close): %10L\n",s),nWrites);
  fprintf(stdout,strans("LRU: pages having been available: %10L\n",s),nAvails);
}

/***********************************************************************/

void PrintFounds(Rlint min, Rlint max, double sum, double sumsqr)

{
  double avg, var, sd, nsd;
  
  printf(strans("%s%L\n",s),"MIN(found): ",min);
  printf(strans("%s%L\n",s),"MAX(found): ",max);
  avg= sum / GVNumbCalls;
  printf("%s%.2e\n","AVG(found): ",avg);
  var= (sumsqr - GVNumbCalls * avg * avg) / GVNumbCalls;
  printf("%s%.2e\n","VAR(found): ",var);
  sd= sqrt(var);
  printf("%s%.2e\n"," SD(found): ",sd);
  nsd= sd/avg;
  printf("%s%.2e\n","NSD(found): ",nsd);
  printf("\n");
}

/***********************************************************************/

void PrintPer(double value, double div, char *tvalue, char *tdiv)

  /* Call of PrintPer:
     NOTE that the assignment of expressions to value and div respectively
     may require parentheses in the strings tvalue and tdiv respectively. */
{
  char PerfMsg[160];
  
  strcat(strcat(strcpy(PerfMsg,tvalue),"/"),tdiv);
  if (div == 0) {
    printf("%35s%s\n",PerfMsg,": -");
  }
  else {
    printf("%35s%s%.2e\n",PerfMsg,": ",value / div);
  }
}

/***********************************************************************/

void PrintReads(t_RT R, Rlint found)

{
  Rlint dirDemands, dataDemands, dirGets, dataGets, dirReads, dataReads,
        Demands, Gets, Reads;
  boolean success;
  
  success= GetCountRead(R,
                        &dirDemands,&dataDemands,
                        &dirGets,&dataGets,
                        &dirReads,&dataReads);
  Demands= dirDemands+dataDemands;
  Gets= dirGets+dataGets;
  Reads= dirReads+dataReads;
  /* if LRU Buffered, set dir- and data-specific R/W counts to 0 (though
     RSTree does all counts as data counts this case): */
  if (GVisBuffered) {
    dirReads= 0; dataReads= 0;
  }
  
  printf(strans("%s%L\n",s),"TOTAL dirDemands: ",dirDemands);
  printf(strans("%s%L\n",s),"TOTAL dataDemands: ",dataDemands);
  printf(strans("%s%L\n",s),"TOTAL Demands: ",Demands);
  printf(strans("%s%L\n",s),"TOTAL dirGets: ",dirGets);
  printf(strans("%s%L\n",s),"TOTAL dataGets: ",dataGets);
  printf(strans("%s%L\n",s),"TOTAL Gets: ",Gets);
  printf(strans("%s%L\n",s),"TOTAL dirReads: ",dirReads);
  printf(strans("%s%L\n",s),"TOTAL dataReads: ",dataReads);
  printf(strans("%s%L\n",s),"TOTAL Reads: ",Reads);
  printf("\n");
  
  PrintPer(dirDemands,GVNumbCalls,"dirDemands","CALL");
  if (found != -1) {
    PrintPer(dirDemands,found,"dirDemands","FOUND");
  }
  PrintPer(dataDemands,GVNumbCalls,"dataDemands","CALL");
  if (found != -1) {
    PrintPer(dataDemands,found,"dataDemands","FOUND");
  }
  PrintPer(Demands,GVNumbCalls,"Demands","CALL");
  if (found != -1) {
    PrintPer(Demands,found,"Demands","FOUND");
  }
  printf("\n");

  PrintPer(dirGets,GVNumbCalls,"dirGets","CALL");
  if (found != -1) {
    PrintPer(dirGets,found,"dirGets","FOUND");
  }
  PrintPer(dataGets,GVNumbCalls,"dataGets","CALL");
  if (found != -1) {
    PrintPer(dataGets,found,"dataGets","FOUND");
  }
  PrintPer(Gets,GVNumbCalls,"Gets","CALL");
  if (found != -1) {
    PrintPer(Gets,found,"Gets","FOUND");
  }
  printf("\n");
  
  PrintPer(dirReads,GVNumbCalls,"dirReads","CALL");
  if (found != -1) {
    PrintPer(dirReads,found,"dirReads","FOUND");
  }
  PrintPer(dataReads,GVNumbCalls,"dataReads","CALL");
  if (found != -1) {
    PrintPer(dataReads,found,"dataReads","FOUND");
  }
  PrintPer(Reads,GVNumbCalls,"Reads","CALL");
  if (found != -1) {
    PrintPer(Reads,found,"Reads","FOUND");
  }
  printf("\n");
  
  PrintPer(dirReads,dirGets,"dirReads","dirGet");
  PrintPer(dataReads,dataGets,"dataReads","dataGet");
  PrintPer(Reads,Gets,"Reads","Get");
  printf("\n");
}

/***********************************************************************/

void PrintJoinedReads(t_RT Rmain, t_RT Rsec, Rlint found)

{
  Rlint MdirDemands, MdataDemands, MdirGets, MdataGets, MdirReads, MdataReads;
  Rlint SdirDemands, SdataDemands, SdirGets, SdataGets, SdirReads, SdataReads;
  Rlint dirDemands, dataDemands, dirGets, dataGets, dirReads, dataReads,
        Demands, Gets, Reads;
  boolean success;
  
  success= GetCountRead(Rmain,
                        &MdirDemands,&MdataDemands,
                        &MdirGets,&MdataGets,
                        &MdirReads,&MdataReads);
  success= GetCountRead(Rsec,
                        &SdirDemands,&SdataDemands,
                        &SdirGets,&SdataGets,
                        &SdirReads,&SdataReads);
  dirDemands= MdirDemands + SdirDemands;
  dataDemands= MdataDemands + SdataDemands;
  dirGets= MdirGets + SdirGets;
  dataGets= MdataGets + SdataGets;
  dirReads= MdirReads + SdirReads;
  dataReads= MdataReads + SdataReads;
  Demands= dirDemands+dataDemands;
  Gets= dirGets+dataGets;
  Reads= dirReads+dataReads;
  /* if LRU Buffered, set dir- and data-specific R/W counts to 0 (though
     RSTree does all counts as data counts this case): */
  if (GVisBuffered) {
    dirReads= 0; dataReads= 0;
  }
  
  printf(strans("%s%L\n",s),"TOTAL dirDemands: ",dirDemands);
  printf(strans("%s%L\n",s),"TOTAL dataDemands: ",dataDemands);
  printf(strans("%s%L\n",s),"TOTAL Demands: ",Demands);
  printf(strans("%s%L\n",s),"TOTAL dirGets: ",dirGets);
  printf(strans("%s%L\n",s),"TOTAL dataGets: ",dataGets);
  printf(strans("%s%L\n",s),"TOTAL Gets: ",Gets);
  printf(strans("%s%L\n",s),"TOTAL dirReads: ",dirReads);
  printf(strans("%s%L\n",s),"TOTAL dataReads: ",dataReads);
  printf(strans("%s%L\n",s),"TOTAL Reads: ",Reads);
  printf("\n");
  
  PrintPer(dirDemands,GVNumbCalls,"dirDemands","CALL");
  if (found != -1) {
    PrintPer(dirDemands,found,"dirDemands","FOUND");
  }
  PrintPer(dataDemands,GVNumbCalls,"dataDemands","CALL");
  if (found != -1) {
    PrintPer(dataDemands,found,"dataDemands","FOUND");
  }
  PrintPer(Demands,GVNumbCalls,"Demands","CALL");
  if (found != -1) {
    PrintPer(Demands,found,"Demands","FOUND");
  }
  printf("\n");

  PrintPer(dirGets,GVNumbCalls,"dirGets","CALL");
  if (found != -1) {
    PrintPer(dirGets,found,"dirGets","FOUND");
  }
  PrintPer(dataGets,GVNumbCalls,"dataGets","CALL");
  if (found != -1) {
    PrintPer(dataGets,found,"dataGets","FOUND");
  }
  PrintPer(Gets,GVNumbCalls,"Gets","CALL");
  if (found != -1) {
    PrintPer(Gets,found,"Gets","FOUND");
  }
  printf("\n");
  
  PrintPer(dirReads,GVNumbCalls,"dirReads","CALL");
  if (found != -1) {
    PrintPer(dirReads,found,"dirReads","FOUND");
  }
  PrintPer(dataReads,GVNumbCalls,"dataReads","CALL");
  if (found != -1) {
    PrintPer(dataReads,found,"dataReads","FOUND");
  }
  PrintPer(Reads,GVNumbCalls,"Reads","CALL");
  if (found != -1) {
    PrintPer(Reads,found,"Reads","FOUND");
  }
  printf("\n");
  
  PrintPer(dirReads,dirGets,"dirReads","dirGet");
  PrintPer(dataReads,dataGets,"dataReads","dataGet");
  PrintPer(Reads,Gets,"Reads","Get");
  printf("\n");
}

/***********************************************************************/

void PrintAdminReads(t_RT R)

{
  Rint psize;
  Rlint dirDemands, dataDemands, dirGets, dataGets, dirReads, dataReads,
        Reads;
  Rlint admDirReads, admDataReads,
        admReads;
  boolean success;
  
  success= GetCountRead(R,
                        &dirDemands,&dataDemands,
                        &dirGets,&dataGets,
                        &dirReads,&dataReads);
  Reads= dirReads+dataReads;
  success= GetCountAdminRead(R,&psize,&admDirReads,&admDataReads);
  admReads= admDirReads+admDataReads;
  
  printf(strans("%s%d%s%L\n",s),"TOTAL ",psize,"bytes admDirReads: ",admDirReads);
  printf(strans("%s%d%s%L\n",s),"TOTAL ",psize,"bytes admDataReads: ",admDataReads);
  printf(strans("%s%d%s%L\n",s),"TOTAL ",psize,"bytes admReads: ",admReads);
  printf("\n");
  
  PrintPer(admReads,Reads,"adminReads","Read");
  printf("\n");
}

/***********************************************************************/

void PrintWrites(t_RT R, Rlint found)

{
  Rlint dirPuts, dataPuts, dirWrites, dataWrites,
        Puts, Writes;
  boolean success;
  
  success= GetCountWrite(R,&dirPuts,&dataPuts,&dirWrites,&dataWrites);
  Puts= dirPuts+dataPuts;
  Writes= dirWrites+dataWrites;
  /* if LRU Buffered, set dir- and data-specific R/W counts to 0 (though
     RSTree does all counts as data counts this case): */
  if (GVisBuffered) {
    dirWrites= 0; dataWrites= 0;
  }
  
  printf(strans("%s%L\n",s),"TOTAL dirPuts: ",dirPuts);
  printf(strans("%s%L\n",s),"TOTAL dataPuts: ",dataPuts);
  printf(strans("%s%L\n",s),"TOTAL Puts: ",Puts);
  printf(strans("%s%L\n",s),"TOTAL dirWrites: ",dirWrites);
  printf(strans("%s%L\n",s),"TOTAL dataWrites: ",dataWrites);
  printf(strans("%s%L\n",s),"TOTAL Writes: ",Writes);
  printf("\n");
  
  PrintPer(dirPuts,GVNumbCalls,"dirPuts","CALL");
  if (found != -1) {
    PrintPer(dirPuts,found,"dirPuts","FOUND");
  }
  PrintPer(dataPuts,GVNumbCalls,"dataPuts","CALL");
  if (found != -1) {
    PrintPer(dataPuts,found,"dataPuts","FOUND");
  }
  PrintPer(Puts,GVNumbCalls,"Puts","CALL");
  if (found != -1) {
    PrintPer(Puts,found,"Puts","FOUND");
  }
  printf("\n");

  PrintPer(dirWrites,GVNumbCalls,"dirWrites","CALL");
  if (found != -1) {
    PrintPer(dirWrites,found,"dirWrites","FOUND");
  }
  PrintPer(dataWrites,GVNumbCalls,"dataWrites","CALL");
  if (found != -1) {
    PrintPer(dataWrites,found,"dataWrites","FOUND");
  }
  PrintPer(Writes,GVNumbCalls,"Writes","CALL");
  if (found != -1) {
    PrintPer(Writes,found,"Writes","FOUND");
  }
  printf("\n");
  
  PrintPer(dirWrites,dirPuts,"dirWrites","dirPut");
  PrintPer(dataWrites,dataPuts,"dataWrites","dataPut");
  PrintPer(Writes,Puts,"Writes","Put");
  printf("\n");
}

/***********************************************************************/

void PrintAdminWrites(t_RT R)

{
  Rint psize;
  Rlint dirPuts, dataPuts, dirWrites, dataWrites,
        Writes;
  Rlint admDirWrites, admDataWrites,
        admWrites;
  boolean success;
  
  success= GetCountWrite(R,&dirPuts,&dataPuts,&dirWrites,&dataWrites);
  Writes= dirWrites+dataWrites;
  success= GetCountAdminWrite(R,&psize,&admDirWrites,&admDataWrites);
  admWrites= admDirWrites+admDataWrites;
  
  printf(strans("%s%d%s%L\n",s),"TOTAL ",psize,"bytes admDirWrites: ",admDirWrites);
  printf(strans("%s%d%s%L\n",s),"TOTAL ",psize,"bytes admDataWrites: ",admDataWrites);
  printf(strans("%s%d%s%L\n",s),"TOTAL ",psize,"bytes admWrites: ",admWrites);
  printf("\n");
  
  PrintPer(admWrites,Writes,"adminWrites","Write");
  printf("\n");
}

/***********************************************************************/

void PrintAccesses(t_RT R, Rlint found)

{
  Rlint dirDemands, dataDemands, dirGets, dataGets, dirReads, dataReads,
        dirPuts, dataPuts, dirWrites, dataWrites,
        dirIOs, dataIOs, IOs,
        dirAccesses, dataAccesses, Accesses;
  boolean success;
  
  success= GetCountRead(R,
                        &dirDemands,&dataDemands,
                        &dirGets,&dataGets,
                        &dirReads,&dataReads);
  success= GetCountWrite(R,&dirPuts,&dataPuts,&dirWrites,&dataWrites);
  dirIOs= dirGets+dirPuts;
  dataIOs= dataGets+dataPuts;
  IOs= dirIOs+dataIOs;
  dirAccesses= dirReads+dirWrites;
  dataAccesses= dataReads+dataWrites;
  Accesses= dirAccesses+dataAccesses;
  /* if LRU Buffered, set dir- and data-specific R/W counts to 0 (though
     RSTree does all counts as data counts this case): */
  if (GVisBuffered) {
    dirAccesses= 0; dataAccesses= 0;
  }
  
  printf(strans("%s%L\n",s),"TOTAL dirIOs: ",dirIOs);
  printf(strans("%s%L\n",s),"TOTAL dataIOs: ",dataIOs);
  printf(strans("%s%L\n",s),"TOTAL IOs: ",IOs);
  printf(strans("%s%L\n",s),"TOTAL dirAccesses: ",dirAccesses);
  printf(strans("%s%L\n",s),"TOTAL dataAccesses: ",dataAccesses);
  printf(strans("%s%L\n",s),"TOTAL Accesses: ",Accesses);
  printf("\n");
  
  PrintPer(dirIOs,GVNumbCalls,"dirIOs","CALL");
  if (found != -1) {
    PrintPer(dirIOs,found,"dirIOs","FOUND");
  }
  PrintPer(dataIOs,GVNumbCalls,"dataIOs","CALL");
  if (found != -1) {
    PrintPer(dataIOs,found,"dataIOs","FOUND");
  }
  PrintPer(IOs,GVNumbCalls,"IOs","CALL");
  if (found != -1) {
    PrintPer(IOs,found,"IOs","FOUND");
  }
  printf("\n");
  
  PrintPer(dirAccesses,GVNumbCalls,"dirAccesses","CALL");
  if (found != -1) {
    PrintPer(dirAccesses,found,"dirAccesses","FOUND");
  }
  PrintPer(dataAccesses,GVNumbCalls,"dataAccesses","CALL");
  if (found != -1) {
    PrintPer(dataAccesses,found,"dataAccesses","FOUND");
  }
  PrintPer(Accesses,GVNumbCalls,"Accesses","CALL");
  if (found != -1) {
    PrintPer(Accesses,found,"Accesses","FOUND");
  }
  printf("\n");
  
  PrintPer(dirAccesses,dirIOs,"dirAccesses","dirIO");
  PrintPer(dataAccesses,dataIOs,"dataAccesses","dataIO");
  PrintPer(Accesses,IOs,"Accesses","IO");
  printf("\n");
}

/***********************************************************************/
/* Note concerning ReInsertion versions:
   An overflow of the root (of the first data node) cannot cause  ReInsert,
   but directly causes a split. Thus building up a tree from scratch
   to height n, n - 1 times overflow directly causes a split. */

void PrintOverUnder(t_RT R)

{
  Rlint dirOverflows, dataOverflows, Overflows,
        dirUnderflows, dataUnderflows, Underflows,
        dirReInsertCalls, dataReInsertCalls, ReInsertCalls,
        dirSplits, dataSplits, Splits,
        dirS_Area0, dataS_Area0, S_Area0;
  boolean success;
  
  success= GetCountOvUndFlw(R,
                            &dirOverflows,&dataOverflows,
                            &dirUnderflows,&dataUnderflows,
                            &dirReInsertCalls,&dataReInsertCalls,
                            &dirSplits,&dataSplits,
                            &dirS_Area0,&dataS_Area0);
  ReInsertCalls= dirReInsertCalls+dataReInsertCalls;
  Splits= dirSplits+dataSplits;
  Overflows= dirOverflows+dataOverflows;
  Underflows= dirUnderflows+dataUnderflows;
  S_Area0= dirS_Area0 + dataS_Area0;
  
  printf(strans("%s%L\n",s),"TOTAL dirOverflows: ",dirOverflows);
  printf(strans("%s%L\n",s),"TOTAL dataOverflows: ",dataOverflows);
  printf(strans("%s%L\n",s),"TOTAL Overflows: ",Overflows);
  printf(strans("%s%L\n",s),"TOTAL dirUnderflows: ",dirUnderflows);
  printf(strans("%s%L\n",s),"TOTAL dataUnderflows: ",dataUnderflows);
  printf(strans("%s%L\n",s),"TOTAL Underflows: ",Underflows);
  printf(strans("%s%L\n",s),"TOTAL dirReInsertCalls: ",dirReInsertCalls);
  printf(strans("%s%L\n",s),"TOTAL dataReInsertCalls: ",dataReInsertCalls);
  printf(strans("%s%L\n",s),"TOTAL ReInsertCalls: ",ReInsertCalls);
  printf(strans("%s%L\n",s),"TOTAL dirSplits: ",dirSplits);
  printf(strans("%s%L\n",s),"TOTAL dataSplits: ",dataSplits);
  printf(strans("%s%L\n",s),"TOTAL Splits: ",Splits);
  printf(strans("%s%L\n",s),"TOTAL dirS_Area0: ",dirS_Area0);
  printf(strans("%s%L\n",s),"TOTAL dataS_Area0: ",dataS_Area0);
  printf(strans("%s%L\n",s),"TOTAL S_Area0: ",S_Area0);
  printf("\n");
  
  PrintPer(dirOverflows,GVNumbCalls,"dirOverflows","CALL");
  PrintPer(dataOverflows,GVNumbCalls,"dataOverflows","CALL");
  PrintPer(Overflows,GVNumbCalls,"Overflows","CALL");
  printf("\n");
  
  PrintPer(ReInsertCalls,Overflows,"ReInsertCalls","Overflow");
  PrintPer(Splits,Overflows,"Splits","Overflow");
  printf("\n");
  
  PrintPer(dirS_Area0,dirSplits,"dirS_Area0","dirSplit");
  PrintPer(dataS_Area0,dataSplits,"dataS_Area0","dataSplit");
  PrintPer(S_Area0,Splits,"S_Area0","Split");
  printf("\n");
  
  PrintPer(dirUnderflows,GVNumbCalls,"dirUnderflows","CALL");
  PrintPer(dataUnderflows,GVNumbCalls,"dataUnderflows","CALL");
  PrintPer(Underflows,GVNumbCalls,"Underflows","CALL");
  printf("\n");
  
  PrintPer(Splits,Underflows,"Splits","Underflow");
  printf("\n");
}

/***********************************************************************/

void PrintChooseSubtree(t_RT R)

{
  Rlint ChooseSubtreeCalls,
        NoFit,
        UniqueFit,
        SomeFit,
        OvlpEnlOpt,
        P,
        maxP,
        PminusQ,
        OvlpEnlComput,
        P1OvlpEnl0,
        AfterwOvlpEnl0,
        Area0;
  boolean success;
  
  success= GetCountChsSbtr(R,
                           &ChooseSubtreeCalls,
                           &NoFit,
                           &UniqueFit,
                           &SomeFit,
                           &OvlpEnlOpt,
                           &P,
                           &maxP,
                           &PminusQ,
                           &OvlpEnlComput,
                           &P1OvlpEnl0,
                           &AfterwOvlpEnl0,
                           &Area0);
  
  printf(strans("%s%L\n",s),"TOTAL ChooseSubtreeCalls: ",ChooseSubtreeCalls);
  printf(strans("%s%L\n",s),"TOTAL NoFit: ",NoFit);
  printf(strans("%s%L\n",s),"TOTAL UniqueFit: ",UniqueFit);
  printf(strans("%s%L\n",s),"TOTAL SomeFit: ",SomeFit);
  printf(strans("%s%L\n",s),"TOTAL OvlpEnlOpt: ",OvlpEnlOpt);
  printf(strans("%s%L\n",s),"TOTAL P: ",P);
  printf(strans("%s%L\n",s),"  MAX P: ",maxP);
  printf(strans("%s%L\n",s),"TOTAL PminusQ: ",PminusQ);
  printf(strans("%s%L\n",s),"TOTAL OvlpEnlComput: ",OvlpEnlComput);
  printf(strans("%s%L\n",s),"TOTAL P1OvlpEnl0: ",P1OvlpEnl0);
  printf(strans("%s%L\n",s),"TOTAL AfterwOvlpEnl0: ",AfterwOvlpEnl0);
  printf(strans("%s%L\n",s),"TOTAL Area0: ",Area0);
  printf("\n");
  
  PrintPer(ChooseSubtreeCalls,GVNumbCalls,"ChooseSubtreeCalls","CALL");
  PrintPer(OvlpEnlComput,GVNumbCalls,"OvlpEnlComput","CALL");
  printf("\n");
  
  PrintPer(NoFit,ChooseSubtreeCalls,"NoFit","ChooseSubtreeCall");
  PrintPer(UniqueFit,ChooseSubtreeCalls,"UniqueFit","ChooseSubtreeCall");
  PrintPer(SomeFit,ChooseSubtreeCalls,"SomeFit","ChooseSubtreeCall");
  PrintPer(OvlpEnlOpt,ChooseSubtreeCalls,"OvlpEnlOpt","ChooseSubtreeCall");
  PrintPer(OvlpEnlComput,ChooseSubtreeCalls,"OvlpEnlComput","ChooseSubtreeCall");
  PrintPer(Area0,ChooseSubtreeCalls,"Area0","ChooseSubtreeCall");
  printf("\n");
  
  PrintPer(P1OvlpEnl0,NoFit,"P1OvlpEnl0","NoFit");
  PrintPer(OvlpEnlOpt,NoFit,"OvlpEnlOpt","NoFit");
  printf("\n");
  
  PrintPer(AfterwOvlpEnl0,OvlpEnlOpt,"AfterwOvlpEnl0","OvlpEnlOpt");
  PrintPer(P,OvlpEnlOpt,"P","OvlpEnlOpt");
  printf("\n");
  
  PrintPer(PminusQ,OvlpEnlOpt-AfterwOvlpEnl0,"PminusQ","(OvlpEnlOpt-AfterwOvlpEnl0)");
  printf("\n");
}

/***********************************************************************/

void PrintCompares(t_RT R, Rlint found)

{
  Rlint dirComparisons, dataComparisons, Comparisons;
  boolean success;
  
  success= GetCountRectComp(R,&dirComparisons,&dataComparisons);
  Comparisons= dirComparisons + dataComparisons;
  
  printf(strans("%s%L\n",s),"TOTAL dirComparisons: ",dirComparisons);
  printf(strans("%s%L\n",s),"TOTAL dataComparisons: ",dataComparisons);
  printf(strans("%s%L\n",s),"TOTAL Comparisons: ",Comparisons);
  printf("\n");

  PrintPer(dirComparisons,GVNumbCalls,"dirComparisons","CALL");
  if (found != -1) {
    PrintPer(dirComparisons,found,"dirComparisons","FOUND");
  }
  PrintPer(dataComparisons,GVNumbCalls,"dataComparisons","CALL");
  if (found != -1) {
    PrintPer(dataComparisons,found,"dataComparisons","FOUND");
  }
  PrintPer(Comparisons,GVNumbCalls,"Comparisons","CALL");
  if (found != -1) {
    PrintPer(Comparisons,found,"Comparisons","FOUND");
  }
  printf("\n");
}

/***********************************************************************/

void PrintGVCompares(Rlint found)

{
  Rlint Comparisons;
  
  Comparisons= GVdirComparisons + GVdataComparisons;
  printf(strans("%s%L\n",s),"TOTAL dirComparisons: ",GVdirComparisons);
  printf(strans("%s%L\n",s),"TOTAL dataComparisons: ",GVdataComparisons);
  printf(strans("%s%L\n",s),"TOTAL Comparisons: ",Comparisons);
  printf("\n");

  PrintPer(GVdirComparisons,GVNumbCalls,"dirComparisons","CALL");
  if (found != -1) {
    PrintPer(GVdirComparisons,found,"dirComparisons","FOUND");
  }
  PrintPer(GVdataComparisons,GVNumbCalls,"dataComparisons","CALL");
  if (found != -1) {
    PrintPer(GVdataComparisons,found,"dataComparisons","FOUND");
  }
  PrintPer(Comparisons,GVNumbCalls,"Comparisons","CALL");
  if (found != -1) {
    PrintPer(Comparisons,found,"Comparisons","FOUND");
  }
  printf("\n");
}

/***********************************************************************/

void PrintPQelems(t_RT R, Rlint nbrs)

{
  Rint len;
  Rint max;
  Rlint PriorQelems;
  boolean success;
  
  success=  GetCountPriorQ(R,&len,&max,&PriorQelems);
  
  printf(strans("%s%L\n",s),"TOTAL PriorQelems: ",PriorQelems);
  printf("\n");

  PrintPer(PriorQelems,GVNumbCalls,"PriorQelems","CALL");
  PrintPer(PriorQelems,nbrs,"PriorQelems","FOUND");
  printf("\n");
}

/***********************************************************************/

void PrintCoverage(t_RT R)

{
  Rint d, numbEnt;
  typinterval *mbb= allocM(GVrectSize);
  
  if (GetGlobalMBB(R,&numbEnt,mbb) && numbEnt > 0) {
    printf("The root, containing %d entries, builds the following MBB:\n",numbEnt);
    for (d= 0; d < GVnumbOfDim; d++) {
      printf("dimension %d:  % .2f  ..  % .2f\n",d,mbb[d].l,mbb[d].h);
    }
  }
  else {
    printf("The tree is empty.\n");
  }
  printf("\n");
  freeM(&mbb);
}

/***********************************************************************/

void PrintRootMBB(t_RT R)

{
  Rint d, numbEnt;
  typinterval *mbb= allocM(GVrectSize);
  
  if (GetRootMBB(R,&numbEnt,mbb) && numbEnt > 0) {
    printf("The root, containing %d entries, builds the following MBB:\n",numbEnt);
    for (d= 0; d < GVnumbOfDim; d++) {
      printf("dimension %d:  % .2f  ..  % .2f\n",d,mbb[d].l,mbb[d].h);
    }
  }
  else {
    printf("The root is empty.\n");
  }
  printf("\n");
  freeM(&mbb);
}

/***********************************************************************/

void PrintCalls(int numbCalls)

{
  printf("%35s%s%d\n","numbCalls",": ",numbCalls);
  printf("\n");
}

/***********************************************************************/

void PrintIndHeight(t_RT R, int i)

{
  Rint height;
  
  printf("%6d%s",i,"::  ");
  GetHeight(R,&height);
  printf("%s%3d\n","height:",height);
  fflush(stderr);
  fflush(stdout);
}

/***********************************************************************/

void PrintIndHeightFounds(t_RT R, int i, Rlint found)

{
  Rint height;
  
  printf("%6d%s",i,"::  ");
  GetHeight(R,&height);
  printf("%s%3d","height:",height);
  printf(strans("%s%6L\n",s),"  found:",found);
  fflush(stderr);
  fflush(stdout);
}

/***********************************************************************/

void PrintIndHeightMatches(t_RT R, int i, Rlint match)

{
  Rint height;
  
  printf("%6d%s",i,"::  ");
  GetHeight(R,&height);
  printf("%s%3d","height:",height);
  printf(strans("%s%6L\n",s),"  match:",match);
  fflush(stderr);
  fflush(stdout);
}

/***********************************************************************/

int GetChar(void)

{
  int ch= getchar();
  if (ch == EOF) {
    printf("ERROR: unexpected EOF in stdin: exiting.\n");
    exit(1);
  }
  return ch;
}

/***********************************************************************/

void PrintRect(typinterval *r)

{
  int d;
  
  for (d= 0; d < GVnumbOfDim; d++) {
    printf("%24.16e",r[d].l);
    printf("%24.16e",r[d].h);
    printf("\n");
  }
}

/***********************************************************************/

void PrintPoint(typcoord *p)

{
  int d;
  
  for (d= 0; d < GVnumbOfDim; d++) {
    printf("%24.16e",p[d]);
    printf("\n");
  }
}

/***********************************************************************/

boolean RdTextToRect(FILE *stream,
                     typinterval *r) {

  char *endPtr;
  size_t bufSize;
  double dNumber;
  
  getline(&GVstrPtr,&bufSize,stream);
  if (feof(stream)) {
    return FALSE;
  }
  /*** Begin this part must be adapted to the format of the source!! ***/
  /* Format: "int,int,double,double\n" */
  strtoll(GVstrPtr,&endPtr,0); // skip the int
  endPtr++; // skip the komma
  strtoll(endPtr,&endPtr,0); // skip the int
  endPtr++; // skip the komma
  dNumber= strtod(endPtr,&endPtr);
  r[0].l= dNumber;
  r[0].h= dNumber;
  endPtr++; // skip the komma
  dNumber= strtod(endPtr,&endPtr);
  r[1].l= dNumber;
  r[1].h= dNumber;
  /***   End this part must be adapted to the format of the source!! ***/
  
  //printf("%f  %f\n",r[0].l,r[1].l); /* CHECK */
  
  return TRUE;
}

/***********************************************************************/
/* Windows does not provide a reasonable timer for the measurement of a
   process' user- and system-time. */
#if defined (_WIN32) || defined (_WIN64)
/* BEGIN of Windows time measurement block: */
/***********************************************************************/

/* Windows.h has its own boolean (a byte), so kill our own boolean: */
#define boolean RSTboolean
#include <Windows.h>

/* declarations */

double TimerTime(LARGE_INTEGER start, LARGE_INTEGER end, LARGE_INTEGER freq);

/* global variables local to this file, for time measurement: no prefix */

LARGE_INTEGER startRealTval, endRealTval, tValFrequency;

/***********************************************************************/
/* Under Windows only measure the rather useless wall clock time: */

void PrintTimes(t_RT R)

{
  double calcAccessTime,
         realTime;
  
  Rlint dirDemands, dataDemands, dirGets, dataGets, dirReads, dataReads,
        dirPuts, dataPuts, dirWrites, dataWrites;
  boolean success;
  
  // *** Begin edit this section for times adaption ***
# define TIME_PER_ACCESS 5.0e-03
  // *** End   edit this section for times adaption ***
  
  realTime= TimerTime(startRealTval,endRealTval,tValFrequency);
  
  success= GetCountRead(R,
                        &dirDemands,&dataDemands,
                        &dirGets,&dataGets,
                        &dirReads,&dataReads);
  success= GetCountWrite(R,&dirPuts,&dataPuts,&dirWrites,&dataWrites);
  // NOTE that we only count data I/Os, assuming an LRU-buffer, completetly
  // holding the directory pages!
  calcAccessTime= (dataGets + dataPuts) * TIME_PER_ACCESS;
  
  printf("Platform:  Windows:  user time   system time   wallclock time\n");
  printf("Support:                NO           NO             YES\n");
  printf("\n");
  printf("(Timer frequency: %9.2f [MHz])\n",tValFrequency.QuadPart / 1000000.0);
  printf("\n");
  printf("Assumptions:\n");
  printf("Assumed access time: %.2e [s]\n",TIME_PER_ACCESS);
  printf("\n");
  
  printf("Times under ACCESS-TIME ASSUMPTIONS in seconds:\n");
  printf("real:      wall clock time: %9.2f\n",realTime);
  printf("access:        access time: %9.2f\n",calcAccessTime);
  printf("\n");
  PrintPer(realTime,GVNumbCalls,"wallClockTime","CALL");
  PrintPer(calcAccessTime,GVNumbCalls,"accessTime","CALL");
  printf("\n");
  
# undef TIME_PER_ACCESS
}

/***********************************************************************/

double TimerTime(LARGE_INTEGER start, LARGE_INTEGER end, LARGE_INTEGER freq)

{  
  return (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
}

/***********************************************************************/

void StartTimers(void)

{
  if (QueryPerformanceFrequency(&tValFrequency)) {
    QueryPerformanceCounter(&startRealTval);
  }
  else {
    printf("ERROR in TIMES: Could not get frequency\n");
  };
}

/***********************************************************************/

void StopTimers(void)

{
  QueryPerformanceCounter(&endRealTval);
}

/***********************************************************************/
/* END of Windows time measurement block. */
#else
/* BEGIN of Unix time measurement block: */
/***********************************************************************/

/* declarations */

double TimerTime(struct timeval start, struct timeval end);

/* variables of types from <sys/time.h> for time measurement: */

struct itimerval startRealTval, endRealTval, // wall clock time
                 startVirtTval, endVirtTval, // user time
                 startProfTval, endProfTval, // user+system time
                 disableTval;                // for disabling timers

/***********************************************************************/

void PrintTimes(t_RT R)

{
  double calcAccessTime,
         virtTime, profTime, realTime, virtPlusAccsTime;
  
  Rlint dirDemands, dataDemands, dirGets, dataGets, dirReads, dataReads,
        dirPuts, dataPuts, dirWrites, dataWrites;
  boolean success;
  
  // *** Begin edit this section for times adaption ***
# define TIME_PER_ACCESS 5.0e-03
  // *** End   edit this section for times adaption ***
  
  virtTime= TimerTime(startVirtTval.it_value,endVirtTval.it_value);
  profTime= TimerTime(startProfTval.it_value,endProfTval.it_value);
  realTime= TimerTime(startRealTval.it_value,endRealTval.it_value);
  
  success= GetCountRead(R,
                        &dirDemands,&dataDemands,
                        &dirGets,&dataGets,
                        &dirReads,&dataReads);
  success= GetCountWrite(R,&dirPuts,&dataPuts,&dirWrites,&dataWrites);
  // NOTE that we only count data I/Os, assuming an LRU-buffer, completetly
  // holding the directory pages!
  calcAccessTime= (dataGets + dataPuts) * TIME_PER_ACCESS;
  
  virtPlusAccsTime= virtTime + calcAccessTime;
  
  printf("Platform:  Unix:     user time   system time   wallclock time\n");
  printf("Support:                YES          YES            YES\n");
  printf("\n");
  printf("Assumptions:\n");
  printf("Assumed access time: %.2e [s]\n",TIME_PER_ACCESS);
  printf("\n");
  
  printf("Times under ACCESS-TIME ASSUMPTIONS in seconds:\n");
  printf("virtual:         user time: %9.2f\n",virtTime);
  printf("profile:  user+system time: %9.2f\n",profTime);
  printf("real:      wall clock time: %9.2f\n",realTime);
  printf("access:        access time: %9.2f\n",calcAccessTime);
  printf("total:    user+access time: %9.2f\n",virtPlusAccsTime);
  printf("\n");
  PrintPer(virtTime,GVNumbCalls,"userTime","CALL");
  PrintPer(profTime,GVNumbCalls,"profileTime","CALL");
  PrintPer(realTime,GVNumbCalls,"wallClockTime","CALL");
  PrintPer(calcAccessTime,GVNumbCalls,"accessTime","CALL");
  PrintPer(virtPlusAccsTime,GVNumbCalls,"(user+access)","CALL");
  printf("\n");
  
# undef TIME_PER_ACCESS
}

/***********************************************************************/
/* This extended times function additional prints the virtual times
   of a processor with a speed, set by the programmer. */

void PrintTimes2fold(t_RT R)

{
  double calcAccessTime,
         virtTime, profTime, realTime, virtPlusAccsTime,
         assumedVirtTime, assumedTotTime;
  
  Rlint dirDemands, dataDemands, dirGets, dataGets, dirReads, dataReads,
        dirPuts, dataPuts, dirWrites, dataWrites;
  boolean success;
  
  // *** Begin edit this section for times adaption ***
  // *** --> PLEASE ADAPT TO THE REAL TECHNICAL ENVIRONMENT !! ***
# define REAL_CPU "\"AMD64 (i386 comp.)\""
# define REAL_SPEED 2667.0 // MHz in FLOATINGPOINT!!
# define REAL_BITS 64
# define ASSUMED_SPEED 2000.0 // MHz in FLOATINGPOINT!!
# define ASSUMED_BITS 64
# define SPEED_FAC (ASSUMED_SPEED / REAL_SPEED)
# define TIME_PER_ACCESS 5.0e-03
  // *** End   edit this section for times adaption ***
  
  virtTime= TimerTime(startVirtTval.it_value,endVirtTval.it_value);
  profTime= TimerTime(startProfTval.it_value,endProfTval.it_value);
  realTime= TimerTime(startRealTval.it_value,endRealTval.it_value);
  
  assumedVirtTime= virtTime / SPEED_FAC;
  
  success= GetCountRead(R,
                        &dirDemands,&dataDemands,
                        &dirGets,&dataGets,
                        &dirReads,&dataReads);
  success= GetCountWrite(R,&dirPuts,&dataPuts,&dirWrites,&dataWrites);
  // NOTE that we only count data I/Os, assuming an LRU-buffer, completetly
  // holding the directory pages!
  calcAccessTime= (dataGets + dataPuts) * TIME_PER_ACCESS;
  
  virtPlusAccsTime= virtTime + calcAccessTime;
  assumedTotTime= assumedVirtTime + calcAccessTime;
  
  printf("Platform:  Unix:     user time   system time   wallclock time\n");
  printf("Support:                YES          YES            YES\n");
  printf("\n");
  printf("Assumptions:\n");
  printf("Assumed access time: %.2e [s]\n",TIME_PER_ACCESS);
  printf("(Real CPU: %s %.3e MHz processor (%d bits))\n",REAL_CPU,REAL_SPEED,REAL_BITS);
  printf("\n");
  
  printf("Times under ACCESS-TIME ASSUMPTIONS in seconds:\n");
  printf("virtual:         user time: %9.2f\n",virtTime);
  printf("profile:  user+system time: %9.2f\n",profTime);
  printf("real:      wall clock time: %9.2f\n",realTime);
  printf("access:        access time: %9.2f\n",calcAccessTime);
  printf("total:    user+access time: %9.2f\n",virtPlusAccsTime);
  printf("\n");
  PrintPer(virtTime,GVNumbCalls,"userTime","CALL");
  PrintPer(profTime,GVNumbCalls,"profileTime","CALL");
  PrintPer(realTime,GVNumbCalls,"wallClockTime","CALL");
  PrintPer(calcAccessTime,GVNumbCalls,"accessTime","CALL");
  PrintPer(virtPlusAccsTime,GVNumbCalls,"(user+access)","CALL");
  printf("\n");
  
  printf("Assumptions:\n");
  printf("Assumed access time: %.2e [s]\n",TIME_PER_ACCESS);
  printf("Assumed CPU speed: %.3e MHz (%d bits)\n",REAL_SPEED * SPEED_FAC,ASSUMED_BITS);
  printf("\n");
  
  printf("Times under CPU-SPEED AND ACCESS-TIME ASSUMPTIONS in seconds:\n");
  printf("Virtual:         User Time: %9.2f\n",assumedVirtTime);
  printf("Access:        Access Time: %9.2f\n",calcAccessTime);
  printf("Total:    User+Access Time: %9.2f\n",assumedTotTime);
  printf("\n");
  PrintPer(assumedVirtTime,GVNumbCalls,"UserTime","CALL");
  PrintPer(calcAccessTime,GVNumbCalls,"AccessTime","CALL");
  PrintPer(assumedTotTime,GVNumbCalls,"TotalTime","CALL");
  printf("\n");
  
# undef REAL_CPU
# undef REAL_SPEED
# undef REAL_BITS
# undef ASSUMED_SPEED
# undef ASSUMED_BITS
# undef SPEED_FAC
# undef TIME_PER_ACCESS
}

/***********************************************************************/

double TimerTime(struct timeval start, struct timeval end)

{
  long secs, usecs;
  
  secs= start.tv_sec - end.tv_sec;
  usecs= start.tv_usec - end.tv_usec;
  return usecs / 1.0e+06 + secs;
}

/***********************************************************************/

void StartTimers(void)

{
  int res= 0;
  
  // set start values of wall clock time:
  startRealTval.it_interval.tv_sec= 0;
  startRealTval.it_interval.tv_usec= 0;
  startRealTval.it_value.tv_sec= 1000000;
  startRealTval.it_value.tv_usec= 0;
  // set start values of user time:
  startVirtTval.it_interval.tv_sec= 0;
  startVirtTval.it_interval.tv_usec= 0;
  startVirtTval.it_value.tv_sec= 1000000;
  startVirtTval.it_value.tv_usec= 0;
  // set start values of user+system time:
  startProfTval.it_interval.tv_sec= 0;
  startProfTval.it_interval.tv_usec= 0;
  startProfTval.it_value.tv_sec= 1000000;
  startProfTval.it_value.tv_usec= 0;
  
  // first start wall clock timer, then user+system timer, then user timer:
  res+= setitimer(ITIMER_REAL,&startRealTval,NULL);
  res+= setitimer(ITIMER_PROF,&startProfTval,NULL);
  res+= setitimer(ITIMER_VIRTUAL,&startVirtTval,NULL);
  
  if (res != 0) {
    perror("ERROR EXIT: StartTimers");
    exit(1);
  }
}

/***********************************************************************/

void StopTimers(void)

{
  int res= 0;
  
  // first stop user timer, then user+system timer, then wall clock timer:
  res+= getitimer(ITIMER_VIRTUAL,&endVirtTval);
  res+= getitimer(ITIMER_PROF,&endProfTval);
  res+= getitimer(ITIMER_REAL,&endRealTval);

  if (res != 0) {
    perror("ERROR EXIT: StopTimers");
    exit(1);
  }
}

/***********************************************************************/
/* END of Unix time measurement block. */
#endif
/***********************************************************************/

