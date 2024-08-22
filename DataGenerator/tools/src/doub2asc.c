/* doub2asc.c: convert binary doubles to ASCII numbers */
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

#define MAX_FPSTR_SIZE 6

Rpint Work(FILE *inFile,
           FILE *outFile,
           boolean isFformat,
           char *formatStr,
           int doubsPerRec,
           int *recRemain);
Rpint PreView(FILE *inFile,
              char *inName,
              FILE *outFile,
              char *outName,
              int doubsPerRec);
Rpint LenOfFile(FILE *f);
void ExitPrintUsage(char *arg);

/************************************************************************/

int main(int argc, char *argv[])

{
  FILE *inFile, *outFile;
  int recRemain, doubsPerRec, clres, nDigits;
  Rpint numbDoubs, doubCount;
  char fpStr[MAX_FPSTR_SIZE];
  char *token;
  char formatStr[MAX_FPSTR_SIZE];
  boolean isFformat= FALSE;
  char s[160];
  
  if (argc < 4 || argc > 5 || atoi(argv[1]) <= 0) {
    ExitPrintUsage(argv[0]);
  }
  
  doubsPerRec= atoi(argv[1]);
  if (strcmp(argv[2],"-") == 0) {
    inFile= stdin;
  }
  else {
    inFile= fopen(argv[2],"rb");
    if (inFile == NULL) {
      perror(argv[2]);
      exit(2);
    }
  }
  if (strcmp(argv[3],"-") == 0) {
    outFile= stdout;
  }
  else {
    outFile= fopen(argv[3],"w+b");
    if (outFile == NULL) {
      perror(argv[3]);
      exit(2);
    }
  }
  if (argc == 5) {
    if (strlcpy(fpStr,argv[4],MAX_FPSTR_SIZE) >= MAX_FPSTR_SIZE) {
      ExitPrintUsage(argv[0]);
    }
    /* does option start with "-f."?: */
    token= strtok(fpStr,".");
    if (strcmp(token,"-f") != 0) {
      ExitPrintUsage(argv[0]);
    }
    /* get remainder of option: */
    token= strtok(NULL,".");
    if (token != NULL) {
      nDigits= atoi(token);
    }
    else {
      ExitPrintUsage(argv[0]);
    }
    if (nDigits < 0 || nDigits > 15) {
      ExitPrintUsage(argv[0]);
    }
    else {
      /* set non constant format string for printf: */
      isFformat= TRUE;
      sprintf(formatStr," %%.%df",nDigits);
    }
  }
  
  numbDoubs= 0;
  if (inFile == stdin) {
    doubCount= Work(inFile,
                    outFile,
                    isFformat,
                    formatStr,
                    doubsPerRec,
                    &recRemain);
  }
  else {
    numbDoubs= PreView(inFile,
                       argv[2],
                       outFile,
                       argv[3],
                       doubsPerRec);
    doubCount= Work(inFile,
                    outFile,
                    isFformat,
                    formatStr,
                    doubsPerRec,
                    &recRemain);
  }
  
  clres= fclose(inFile);
  if (clres != 0) {
    perror(argv[2]);
    exit(2);
  }
  clres= fclose(outFile);
  if (clres != 0) {
    perror(argv[3]);
    exit(2);
  }
  
  fprintf(stderr,strans("%P records, ",s),doubCount / doubsPerRec);
  fprintf(stderr,strans("%P doubles resp. processed:\n",s),doubCount);
  if (inFile == stdin) {
    if (recRemain == 0) {
      fprintf(stderr,"Done.\n");
    }
    else {
      fprintf(stderr,"WARNING: incomplete last record (%d doubles).\n",recRemain);
      fprintf(stderr,"Done.\n");
      /* no control for incomplete items (doubles here) continuously reading */
    }
  }
  else {
    if (doubCount == numbDoubs) {
      fprintf(stderr,"Done.\n");
    }
    else {
      fprintf(stderr,"Checksum ERROR detected.\n");
      exit(2);
    }
  }
  return 0;
}

/************************************************************************/

Rpint Work(FILE *inFile,
           FILE *outFile,
           boolean isFformat,
           char *formatStr,
           int doubsPerRec,
           int *recRemain)

{
  int nitems;
  Rpint doubCount, nbytes, i;
  double Double;
  int lastRecDoub= doubsPerRec - 1;
  char s[160];
  
  doubCount= 0;
  for (;;) {
    for (i= 0; i < doubsPerRec; i++) {
      nitems= fread(&Double,sizeof(double),1,inFile);
      if (feof(inFile) != 0) {
        /* failing to read AFTER(!) EOF ==> i == 0: normal exit! */
        *recRemain= i;
        if (i != 0) {
          fprintf(outFile,"\n");
        }
        return doubCount;
      }
      if (nitems != 1) {
        fprintf(stderr,"nitems = %d: ",nitems);
        perror("ERROR reading");
        return doubCount;
      }
      if (isFformat) {
        nbytes= fprintf(outFile,formatStr,Double);
      }
      else {
        nbytes= fprintf(outFile,"% .15e",Double);
      }
      if (nbytes < 0) {
        fprintf(stderr,strans("nbytes = %P: ",s),nbytes);
        perror("ERROR writing");
        return doubCount;
      }
      doubCount++;
      
      if (i < lastRecDoub) {
        nbytes= fprintf(outFile," ");
      }
      else {
        nbytes= fprintf(outFile,"\n");
      }
      if (nbytes < 0) {
        fprintf(stderr,strans("nbytes = %P: ",s),nbytes);
        perror("ERROR writing");
        return doubCount;
      }
    }
  }
  /* No return, see "normal exit" above! */
}

/************************************************************************/

Rpint PreView(FILE *inFile,
              char *inName,
              FILE *outFile,
              char *outName,
              int doubsPerRec)

{
  int recSize, doubRemain, recRemain;
  Rpint lof, numbDoubs, numbRecs;
  char s[160];
  
  lof= LenOfFile(inFile);
  numbDoubs= lof / sizeof(double);
  doubRemain= lof % sizeof(double);     /* in bytes! */
  recSize= doubsPerRec * sizeof(double);
  numbRecs= lof / recSize;
  recRemain= (lof % recSize) / sizeof(double);     /* in doubles! */
  
  fprintf(stderr,strans("Reading %P records, %P doubles resp. from %s.\n",s),numbRecs,numbDoubs,inName);
  fprintf(stderr,"Writing to %s\n",outName);
  if (recRemain != 0) {
    fprintf(stderr,"WARNING: remainder (incomplete last record) of %d doubles.\n",recRemain);
  }
  if (doubRemain != 0) {
    fprintf(stderr,"WARNING: IGNORING remainder (incomplete last double) of length %d bytes.\n",doubRemain);
  }
  fprintf(stderr,"WORKING ...\n");
  return numbDoubs;
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

void ExitPrintUsage(char *arg) {

  fprintf(stderr,
"Usage:  %s %s %s %s %s\n\n",arg,
"doubsPerRec","inFile","outFile","[ -f.<precision> ]"
  );
  fprintf(stderr,"%s%s\n",
arg," reads binary doubles from inFile, converts them to ASCII number"
  );
  fprintf(stderr,"%s\n",
"strings (numbers), and writes these to outFile in lines (records) of"
  );
  fprintf(stderr,"%s\n",
"doubsPerRec numbers."
  );
  fprintf(stderr,"%s\n",
"The records are separated by LINEFEEDs, whereas the numbers are separated"
  );
  fprintf(stderr,"%s\n",
"by BLANKs."
  );
  fprintf(stderr,"%s\n",
"If inFile is set to \"-\" the input is taken from stdin."
  );
  fprintf(stderr,"%s\n",
"If outFile is set to \"-\" the output is directed to stdout."
  );
  fprintf(stderr,"%s\n",
"Accompanying messages are directed to stderr."
  );
  fprintf(stderr,"%s\n",
"Options:"
  );
  fprintf(stderr,"%s\n",
"  -f.<p>  \"precision\""
  );
  fprintf(stderr,"%s\n",
"  The number output defaults to the printf format \"%.15e\", which preserves"
  );
  fprintf(stderr,"%s\n",
"  the complete precision of a binary double (as far as this is possible)."
  );
  fprintf(stderr,"%s\n",
"  With the option -f.<p> the output format is set to the printf format"
  );
  fprintf(stderr,"%s\n",
"  \"%.<p>f\", where the integer p can be set to values between 0 and 15."
  );
fprintf(stderr,"%s\n",
"Exit status:"
);
  fprintf(stderr,"%s\n",
"  Returns 0 on success;"
  );
  fprintf(stderr,"%s\n",
"  Returns 1 if a parameter mismatch was detected;"
  );
  fprintf(stderr,"%s\n",
"  Returns 2 if a fatal error occured."
  );
  exit(1);
}

/************************************************************************/



