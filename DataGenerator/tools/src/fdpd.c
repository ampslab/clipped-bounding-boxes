/* fdpd.c: create a table of identical intervals in a points file */
#//
#// Copyright (c) 1994 - 2013 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//-----
#include "RSTStdTypes.h"
/* RSTStdTypes.h mainly provides R*-tree standard constants, types and
   functions, normally not needed here.
   But type Rpint is used to provide a pointer compatible integer type on
   platforms, not serving a 32/64 bit transitional long integer and concerning
   I/O functions (Windows).
   Note that if a source includes RSTStdTypes.h, linking requires libPltfHelp
   or libUTIL. */


#define boolean int
#define FALSE 0
#define TRUE 1

typedef struct {
               double v;
               Rpint n;
               } doubleCount;

void PrintHelp(char *arg);
void PrintUsage(char *arg);
void PrintRectFormat(void);
Rpint Work(FILE *file,
           boolean verbose,
           boolean nonAbr,
           int numbDim,
           int ptSize,
           Rpint numbPts);
Rpint PreView(FILE *file,
              char *name,
              int ptSize,
              int numbDim);
Rpint LenOfFile(FILE *f);
void QSort_doubles(Rpint begin,
                   Rpint end,
                   double value[]);
static void Exchange_doubles(double *x, double *y);
static double Median(double v[],
                     Rpint *b,
                     Rpint m,
                     Rpint *e);
void QSort_doubleCountByCount(Rpint begin,
                              Rpint end,
                              doubleCount value[]);
static void Exchange_doubleCounts(doubleCount *x, doubleCount *y);
static doubleCount Median_doubleCountByCount(doubleCount v[],
                                             Rpint *b,
                                             Rpint m,
                                             Rpint *e);
boolean StrCorrect(char *inStr,
                   char start,
                   char *remaind,
                   int len);

/************************************************************************/

int main(int argc, char *argv[])

{
  FILE *file;
  int numbDim, ptSize, clres;
  Rpint numbPts, ptCount;
  char s[160];
  boolean verbose= FALSE;
  boolean nonAbr= FALSE;
  
  if (argc == 2 && (strcmp(argv[1],"-help") == 0 || strcmp(argv[1],"-h") == 0)) {
    PrintHelp(argv[0]);
    exit(0);
  }
  else if (argc < 3 || argc > 5) {
    // incorrect #args
    PrintUsage(argv[0]);
    exit(1);
  }
  else if (atoi(argv[1]) <= 0) {
    // incorrect numbDim
    PrintUsage(argv[0]);
    exit(1);
  }
  if (argc == 4) {
    if (! StrCorrect(argv[3],'-',"av",3)) {
      // [-av] option incorrect
      PrintUsage(argv[0]);
      exit(1);
    }
    verbose= strchr(argv[3],'v') != NULL;
    nonAbr= strchr(argv[3],'a') != NULL;
  }
  else if (argc == 5) {
    if (! StrCorrect(argv[3],'-',"av",3) || ! StrCorrect(argv[4],'-',"av",3)) {
      // one [-av] option incorrect
      PrintUsage(argv[0]);
      exit(1);
    }
    verbose= strchr(argv[3],'v') != NULL || strchr(argv[4],'v') != NULL;
    nonAbr= strchr(argv[3],'a') != NULL || strchr(argv[4],'a') != NULL;
  }
  
  numbDim= atoi(argv[1]);
  ptSize= numbDim * 2 * sizeof(double);
  
  file= fopen(argv[2],"rb");
  if (file == NULL) {
    perror(argv[2]);
    exit(2);
  }
  
  numbPts= 0;
  numbPts= PreView(file,
                   argv[2],
                   ptSize,
                   numbDim);
  ptCount= Work(file,
                verbose,
                nonAbr,
                numbDim,
                ptSize,
                numbPts);
  
  clres= fclose(file);
  if (clres != 0) {
    perror(argv[2]);
    exit(2);
  }
  
  fprintf(stderr,strans("%P Points processed:\n",s),ptCount);
  if (ptCount == numbPts) {
    fprintf(stderr,"Done.\n");
  }
  else {
    fprintf(stderr,"Checksum ERROR detected.\n");
    exit(2);
  }
  return 0;
}

/************************************************************************/

void PrintHelp(char *arg) {

  fprintf(stdout,
"SYNOPSYS\n");
  fprintf(stdout,"%s%s%s",
"     Usage: ",arg," -help | -h\n");
  fprintf(stdout,"%s%s%s",
"            ",arg," numbOfDim file [ -av ]\n");
  fprintf(stdout,"\n");
  fprintf(stdout,"%s%s\n",
arg," reads nD rectangles of the format"
  );
  PrintRectFormat();
  fprintf(stdout,"%s\n",
"from file."
  );
  fprintf(stdout,"%s\n",
"The rectangles actually should be POINTS, i.e. l = h should apply."
  );
  fprintf(stdout,"%s\n",
"If non-points are found during operation, execution stops with a warning."
  );
  fprintf(stdout,"%s\n",
"For each dimension, a (possibly) abridged report of multiple occurrences is"
  );
  fprintf(stdout,"%s\n",
"printed. It contains the 10 most frequent multiple occurrences, primarily"
  );
  fprintf(stdout,"%s\n",
"sorted by descending frequency, secondarily sorted by descending values."
  );
  fprintf(stdout,"%s\n",
"Options:"
  );
  fprintf(stdout,"%s\n",
"  -a  \"all\"/\"non-abridged\": A complete report of all multiple occurrences"
  );
  fprintf(stdout,"%s\n",
"      is printed, sorted as explained above."
  );
  fprintf(stdout,"%s\n",
"  -v  \"verbose\": Ahead of the abridged report, all multiple occurrences are"
  );
  fprintf(stdout,"%s\n",
"      given out, ascendingly sorted by their value."
  );
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

/************************************************************************/

void PrintUsage(char *arg) {

  fprintf(stderr,"%s%s%s","Usage: ",arg," -help | -h\n");
  fprintf(stderr,"%s%s%s","       ",arg," numbOfDim file [ -av ]\n");
}

/************************************************************************/

void PrintRectFormat(void)

{
  fprintf(stdout,"struct {double l, h;} rectangle[NumbOfDim];\n");
}

/***********************************************************************/

Rpint Work(FILE *file,
           boolean verbose,
           boolean nonAbr,
           int numbDim,
           int ptSize,
           Rpint numbPts)

{
  int offset, dim, res, nitems;
  boolean linefeed;
  Rpint ptCount, i, j, itemCount, duplInd, first, numbMultReport;
  double toBeCmprd;
  double buf[2];
  double *coordArr= malloc(numbPts * sizeof(double));
  doubleCount *cmpArr;
  char reportStr1[160], reportStr2[160];
  char s[160];
  int itemsPerLine= 5;
  int abrReportCap= 10;
  Rpint maxMulOcc= numbPts / 2;
  
  cmpArr= malloc(maxMulOcc * sizeof(doubleCount));
  
  for (dim= 0; dim < numbDim; dim++) {
    /* Fill coordArr with single coords: */
    offset= dim * 2 * sizeof(double);
    ptCount= 0;
    while (ptCount < numbPts) {
      res= fseek(file,ptCount*ptSize+offset,SEEK_SET);
      if (res != 0) {
        perror("ERROR reading");
        exit(2);
      }
      nitems= fread(buf,sizeof(double),2,file);
      if (nitems != 2) {
        fprintf(stderr,"nitems = %d: ",nitems);
        perror("ERROR reading");
        return ptCount;
      }
      /* Check if we have points (not rectangles): */
      if (buf[0] != buf[1]) {
        fprintf(stderr,"ERROR: coordinate mismatch: interval: l != h\n");
        return ptCount;
      }
      coordArr[ptCount]= *buf;
      ptCount++;
    }
    /* Sort: */
    QSort_doubles(0,numbPts-1,coordArr);
    /* Search coordArr for multiple occurrences: */
    fprintf(stdout,"DIMENSION %2d:\n",dim);
    ptCount= 0; duplInd= 0;
    while (ptCount < numbPts) {
      toBeCmprd= coordArr[ptCount];
      ptCount++;
      /* count #duplicates: */
      i= 0;
      while (ptCount < numbPts && toBeCmprd == coordArr[ptCount]) {
        i++; ptCount++;
      }
      if (i > 0) {
        cmpArr[duplInd].v= toBeCmprd;
        /* store #occurrences: */
        cmpArr[duplInd].n= i + 1;
        duplInd++;
      }
    }
    if (duplInd == 0) {
      fprintf(stdout," NO MULTIPLE OCCURRENCES\n");
    }
    else {
      if (verbose) {
        fprintf(stdout,"Multiple Occurrences SORTED by ASCENDING VALUES in dimension %2d:\n",dim);
        fprintf(stdout,"Complete collection (value number):\n");
        itemCount= 1;
        for (j= 0; j < duplInd; j++) {
          linefeed= FALSE;
          fprintf(stdout,strans(" (%.7e %P)",s),cmpArr[j].v,cmpArr[j].n);
          if (itemCount % itemsPerLine == 0) {
            fprintf(stdout,"\n");
            linefeed= TRUE;
          }
          itemCount++;
        }
        if (! linefeed) {
          fprintf(stdout,"\n");
        }
      }
      if (nonAbr || duplInd <= abrReportCap) {
        numbMultReport= duplInd;
        first= 0;
        sprintf(reportStr1,"%s","NON abridged report");
        sprintf(reportStr2,strans("%s %P",s),"ALL",numbMultReport);
      }
      else {
        numbMultReport= abrReportCap;
        first= duplInd - numbMultReport;
        sprintf(reportStr1,"%s","ABRIDGED report");
        sprintf(reportStr2,strans("%s %P %s %P",s),"THE",numbMultReport,"MOST FREQUENT of",duplInd);
      }
      fprintf(stdout,"Multiple Occurrences SORTED by DESCENDING OCCURRENCE (descending value)\n");
      fprintf(stdout,"%s:\n",reportStr1);
      fprintf(stdout,"%s",reportStr2);
      fprintf(stdout," multiple occurrences  (value number):\n");
      QSort_doubleCountByCount(0,duplInd-1,cmpArr);
      itemCount= 1;
      for (j= duplInd-1; j >= first; j--) {
        linefeed= FALSE;
        fprintf(stdout,strans(" (%.7e %P)",s),cmpArr[j].v,cmpArr[j].n);
        if (itemCount % itemsPerLine == 0) {
          fprintf(stdout,"\n");
          linefeed= TRUE;
        }
        itemCount++;
      }
      if (! linefeed) {
        fprintf(stdout,"\n");
      }
    }
    fprintf(stdout,"\n");
  }
  free(coordArr); coordArr= NULL;
  free(cmpArr); cmpArr= NULL;
  return ptCount;
}

/************************************************************************/

Rpint PreView(FILE *file,
              char *name,
              int ptSize,
              int numbDim)

{
  int ptRemain;
  Rpint lof, numbPts;
  char s[160];
  
  lof= LenOfFile(file);
  numbPts= lof / ptSize;
  ptRemain= lof % ptSize;
  
  fprintf(stderr,strans("Reading %P points of %d bytes from %s.\n",s),numbPts,ptSize,name);
  if (ptRemain != 0) {
    fprintf(stderr,"WARNING: IGNORING remainder (incomplete last point) of length %d.\n",ptRemain);
  }
  fprintf(stdout,"NOTE that the %d dimensions count 0 .. %d!!\n",numbDim,numbDim-1);
  fprintf(stderr,"WORKING ...\n");
  fprintf(stderr,"\n");
  return numbPts;
}

/************************************************************************/

Rpint LenOfFile(FILE *f)

{
  int res;
  Rpint pos, lof;
  
  
  pos= ftell(f);
  if (pos == -1) {
    perror("ERROR checking file length");
    exit(2);
  }
  
  res= fseek(f,0,SEEK_END);
  if (res != 0) {
    perror("ERROR checking file length");
    exit(2);
  }
  lof= ftell(f);
  if (lof == -1) {
    perror("ERROR checking file length");
    exit(2);
  }
  
  res= fseek(f,pos,SEEK_SET);
  if (res != 0) {
    perror("ERROR checking file length");
    exit(2);
  }
  
  return lof;
}

/************************************************************************/

void QSort_doubles(Rpint b,
                   Rpint e,
                   double value[])
/* Sorts value. */

{
  double m;
  Rpint i, j;
  
  i= b; j= e;
  /* m= value[(i+j) >> 1]; */
  /* profitable for #elements > 7: */
  m= Median(value,&i,(i+j) >> 1,&j);
  if (i < j) {
    do {
      while (value[i] < m) {
        i++;
      }
      while (value[j] > m) {
        j--;
      }
      if (i < j) {
        Exchange_doubles(&value[i],&value[j]);
        i++; j--;
      }
      else if (i == j) {
        i++; j--;
      }
    } while (i <= j);
    if (b < j) {
      if (j - b > 1) {
        QSort_doubles(b,j,value);
      }
      else {
        if (value[b] > value[j]) {
          Exchange_doubles(&value[b],&value[j]);
        }
      }
    }
    if (i < e) {
      if (e - i > 1) {
        QSort_doubles(i,e,value);
      }
      else {
        if (value[i] > value[e]) {
          Exchange_doubles(&value[i],&value[e]);
        }
      }
    }
  }
}

/***********************************************************************/

static void Exchange_doubles(double *x, double *y)

{
  double z;
  
  z= *x; *x= *y; *y= z;
}

/***********************************************************************/
/************* median calculation (comparison optimized): **************/

static double Median(double v[],
                     Rpint *b,
                     Rpint m,
                     Rpint *e) {

  if (v[*b] <= v[m]) {
    if (v[m] <= v[*e]) {                 //bme (sorted)
    }
    else {
      if (v[*b] <= v[*e]) {              //bem
        Exchange_doubles(&v[m],&v[*e]);
      }
      else {                             //ebm (b == m ==> x(b,e) --> x opt.)
        Exchange_doubles(&v[m],&v[*e]);
        Exchange_doubles(&v[*b],&v[m]);
      }
    }
  }
  else {
    if (v[*b] <= v[*e]) {                //mbe
      Exchange_doubles(&v[*b],&v[m]);
    }
    else {
      if (v[m] < v[*e]) {                //meb (NOTE the "<"!!)
        Exchange_doubles(&v[*b],&v[m]);
        Exchange_doubles(&v[m],&v[*e]);
      }
      else {                             //emb
        Exchange_doubles(&v[*b],&v[*e]);
      }
    }
  }
  (*b)++; (*e)--;
  return v[m];
}

/***********************************************************************/

void QSort_doubleCountByCount(Rpint b,
                              Rpint e,
                              doubleCount v[])
/* Sorts v
   primarily   by v[i].n,
   secondarily by v[i].v. */

{
  doubleCount m;
  Rpint i, j;
  
  i= b; j= e;
  /* m= v[(i+j) >> 1]; */
  /* profitable for #elements > 7: */
  m= Median_doubleCountByCount(v,&i,(i+j) >> 1,&j);
  if (i < j) {
    do {
      while (v[i].n < m.n || v[i].n == m.n && v[i].v < m.v) {
        i++;
      }
      while (v[j].n > m.n || v[j].n == m.n && v[j].v > m.v) {
        j--;
      }
      if (i < j) {
        Exchange_doubleCounts(&v[i],&v[j]);
        i++; j--;
      }
      else if (i == j) {
        i++; j--;
      }
    } while (i <= j);
    if (b < j) {
      if (j - b > 1) {
        QSort_doubleCountByCount(b,j,v);
      }
      else {
        if (v[b].n > v[j].n || v[b].n == v[j].n && v[b].v > v[j].v) {
          Exchange_doubleCounts(&v[b],&v[j]);
        }
      }
    }
    if (i < e) {
      if (e - i > 1) {
        QSort_doubleCountByCount(i,e,v);
      }
      else {
        if (v[i].n > v[e].n || v[i].n == v[e].n && v[i].v > v[e].v) {
          Exchange_doubleCounts(&v[i],&v[e]);
        }
      }
    }
  }
}

/***********************************************************************/

static void Exchange_doubleCounts(doubleCount *x, doubleCount *y)

{
  doubleCount z;
  
  z= *x; *x= *y; *y= z;
}

/***********************************************************************/
/************* median calculation (comparison optimized): **************/

static doubleCount Median_doubleCountByCount(doubleCount v[],
                                             Rpint *b,
                                             Rpint m,
                                             Rpint *e) {

  if (v[*b].n < v[m].n || v[*b].n == v[m].n && v[*b].v <= v[m].v) {
    if (v[m].n < v[*e].n || v[m].n == v[*e].n && v[m].v <= v[*e].v) {
      //bme (sorted)
    }
    else {
      if (v[*b].n < v[*e].n || v[*b].n == v[*e].n && v[*b].v <= v[*e].v) {
        //bem
        Exchange_doubleCounts(&v[m],&v[*e]);
      }
      else {
        //ebm (b == m ==> x(b,e) --> x opt.)
        Exchange_doubleCounts(&v[m],&v[*e]);
        Exchange_doubleCounts(&v[*b],&v[m]);
      }
    }
  }
  else {
    if (v[*b].n < v[*e].n || v[*b].n == v[*e].n && v[*b].v <= v[*e].v) {
      //mbe
      Exchange_doubleCounts(&v[*b],&v[m]);
    }
    else {
      if (v[m].n < v[*e].n || v[m].n == v[*e].n && v[m].v < v[*e].v) {
        //meb (NOTE the "<"!!)
        Exchange_doubleCounts(&v[*b],&v[m]);
        Exchange_doubleCounts(&v[m],&v[*e]);
      }
      else {
        //emb
        Exchange_doubleCounts(&v[*b],&v[*e]);
      }
    }
  }
  (*b)++; (*e)--;
  return v[m];
}

/***********************************************************************/
/* The function analyses the correctness of inStr as follows:
   - The maximum allowed length of inStr is len.
   - If start != 0, start must be the first char of inStr.
   - Apart from the (possibly) leading start char, inStr may only contain
     char's which are also present in remain. */
   
boolean StrCorrect(char *inStr,
                   char start,
                   char *remain,
                   int len) {

  int i, j, begin;
  boolean inRemain;
  
  if (start != 0 && inStr[0] != start) {
    return FALSE;
  }
  if (strnlen(inStr,len + 1) > len) {
    return FALSE;
  }
  begin= (start == 0) ? 0 : 1;
  for (i= begin; i < len; i++) {
    inRemain= FALSE;
    for (j= 0; j < len; j++) {
      if (inStr[i] == remain[j]) {
        inRemain= TRUE;
        break;
      }
    }
    if (! inRemain) {
      return FALSE;
    }
  }
  return TRUE;
}

/***********************************************************************/

