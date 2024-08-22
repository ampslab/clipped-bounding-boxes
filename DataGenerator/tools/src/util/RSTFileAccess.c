/* ----- RSTFileAccess.c ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//


#define _XOPEN_SOURCE 500 /* 09/03 for Linux: pread(), pwrite() */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "RSTFileAccess.h"
#include "RSTMemAlloc.h"


/************************************************************************/

boolean OpenRdOnlyF(const char *path,
                    File *f)

{
#if defined (_WIN32) || defined (_WIN64)
  Rint O_MODE= O_RDONLY | O_BINARY;
#else
  Rint O_MODE= O_RDONLY;
#endif
  
  *f= open(path,O_MODE,FILE_PERM);
  if (*f == -1) {
    perror("RSTFileAccess.OpenRdOnlyF: ERROR: OS");
    return FALSE;
  }
  return TRUE;
}

/************************************************************************/

boolean OpenRdWrF(const char *path,
                  File *f)

{
#if defined (_WIN32) || defined (_WIN64)
  Rint O_MODE= O_RDWR | O_BINARY;
#else
  Rint O_MODE= O_RDWR;
#endif
  
  *f= open(path,O_MODE,FILE_PERM);
  if (*f == -1) {
    perror("RSTFileAccess.OpenRdWrF: ERROR: OS");
    return FALSE;
  }
  return TRUE;
}

/************************************************************************/

boolean CreateExclF(const char *path,
                    File *f)

{
#if defined (_WIN32) || defined (_WIN64)
  Rint O_MODE= O_RDWR | O_CREAT | O_EXCL | O_BINARY;     /* do not overwrite */
#else
  Rint O_MODE= O_RDWR | O_CREAT | O_EXCL;     /* do not overwrite */
#endif
  
  *f= open(path,O_MODE,FILE_PERM);
  if (*f == -1) {
    perror("RSTFileAccess.CreateExclF: ERROR: OS");
    return FALSE;
  }
  return TRUE;
}

/************************************************************************/

boolean CreateTruncF(const char *path,
                     File *f)

{
#if defined (_WIN32) || defined (_WIN64)
  Rint O_MODE= O_RDWR | O_CREAT | O_TRUNC | O_BINARY;     /* overwrite */
#else
  Rint O_MODE= O_RDWR | O_CREAT | O_TRUNC;     /* overwrite */
#endif
  
  *f= open(path,O_MODE,FILE_PERM);
  if (*f == -1) {
    perror("RSTFileAccess.CreateTruncF: ERROR: OS");
    return FALSE;
  }
  return TRUE;
}

/************************************************************************/

boolean CloseF(File f)

{
  if (close(f) == -1) {
    perror("RSTFileAccess.CloseF: ERROR: OS");
    return FALSE;
  }
  return TRUE;
}

/************************************************************************/

boolean RenameF(const char *oldPath,
                const char *newPath)

{
  if (rename(oldPath,newPath) == -1) {
    perror("RSTFileAccess.RenameF: ERROR: OS");
    return FALSE;
  }
  return TRUE;
}

/************************************************************************/

boolean SilRenameF(const char *oldPath,
                   const char *newPath)

{
  if (rename(oldPath,newPath) == -1) {
    return FALSE;
  }
  return TRUE;
}

/************************************************************************/

boolean UnlinkF(const char *path)

{
  if (unlink(path) == -1) {
    perror("RSTFileAccess.UnlinkF: ERROR: OS");
    return FALSE;
  }
  return TRUE;
}

/************************************************************************/

boolean SilUnlinkF(const char *path)

{
  if (unlink(path) == -1) {
    return FALSE;
  }
  return TRUE;
}

/************************************************************************/

Rpint LenOfF(File f)

{
  Rint ferr;
  struct stat status;
  
  ferr= fstat(f,&status);
  if (ferr == -1) {
    perror("RSTFileAccess.LenOfF: ERROR: OS");
    return -1;
  }
  else {
    return status.st_size;
  }
}

/************************************************************************/

boolean TruncateF(File f,
                  Rpint length)

{
  if (ftruncate(f,length) == -1) {
    perror("RSTFileAccess.TruncateF: ERROR: OS");
    return FALSE;
  }
  return TRUE;
}

/************************************************************************/

boolean SetPosF(File f,
                Rpint pos)

{
  Rpint p= lseek(f,pos,SEEK_SET); /* manual: returns pos or -1 */
  if (p != pos) {
    perror("RSTFileAccess.SetPosF: ERROR: OS");
    return FALSE;
  }
  return TRUE;
}

/************************************************************************/

boolean GetPosF(File f,
                Rpint *pos)

{
  *pos= lseek(f,0,SEEK_CUR); /* manual: returns pos or -1 */
  if (*pos == -1) {
    perror("RSTFileAccess.GetPosF: ERROR: OS");
    return FALSE;
  }
  return TRUE;
}

/************************************************************************/

boolean RdBytes(File f,
                void *adr,
                Rpint size)

{
  Rpint newPos;
  char s[160];
  
  Rpint nbytes= read(f,adr,size);
  if (nbytes != size) {
    if (nbytes > 0) {
      /* the 0 case is treated as a legal EOF test */
      newPos= lseek(f,0,SEEK_CUR);
      fprintf(stderr,strans("file %d, pos: %P: ",s),f,newPos);
      fprintf(stderr,strans("RSTFileAccess.RdBytes: ERROR: requested: %P, read: %P\n",s),size,nbytes);
    }
    else if (nbytes == -1) {
      fprintf(stderr,"file %d: ",f);
      perror("RSTFileAccess.RdBytes: ERROR: OS");
    }
    return FALSE;
  }
  return TRUE;
}

/************************************************************************/

boolean WrBytes(File f,
                void *adr,
                Rpint size)

{
  Rpint newPos;
  char s[160];
  
  Rpint nbytes= write(f,adr,size);
  if (nbytes != size) {
    if (nbytes >= 0) {
      newPos= lseek(f,0,SEEK_CUR);
      fprintf(stderr,strans("file %d, pos: %P: ",s),f,newPos);
      fprintf(stderr,strans("RSTFileAccess.WrBytes: ERROR: demanded: %P, written: %P\n",s),size,nbytes);
    }
    else {
      fprintf(stderr,"file %d: ",f);
      perror("RSTFileAccess.WrBytes: ERROR: OS");
    }
    return FALSE;
  }
  return TRUE;
}

/************************************************************************/
/* Both, pagesize and pagenr are limited to Rint and Rpnint respectively,
   while their product must be capable to range in Rpint. */

boolean RdPage(File f,
               Rint pagesize,
               Rpnint pagenr,
               void *ptr) {

#if defined (_WIN32) || defined (_WIN64)
  return NonAtomicRdPage(f,pagesize,pagenr,ptr);
}
#else
  char s[80];
  Rpint nbytes= pread(f,ptr,pagesize,(Rpint)pagenr * (Rpint)pagesize);
  // Contrary to lseek and read, pread does NOT change the file pointer!
  
  if (nbytes != pagesize) {
    fprintf(stderr,strans("file %d, page %N: ",s),f,pagenr);
    perror("RSTFileAccess.RdPage: ERROR: OS");
    return FALSE;
  }
  return TRUE;
}
#endif
/************************************************************************/
/* Both, pagesize and pagenr are limited to Rint and Rpnint respectively,
   while their product must be capable to range in Rpint. */

boolean WrPage(File f,
               Rint pagesize,
               Rpnint pagenr,
               void *ptr) {

#if defined (_WIN32) || defined (_WIN64)
  return NonAtomicWrPage(f,pagesize,pagenr,ptr);
}
#else
  char s[80];
  Rpint nbytes= pwrite(f,ptr,pagesize,(Rpint)pagenr * (Rpint)pagesize);
  // Contrary to lseek and write, pwrite does NOT change the file pointer!
  
  if (nbytes != pagesize) {
    fprintf(stderr,strans("file %d, page %N: ",s),f,pagenr);
    perror("RSTFileAccess.WrPage: ERROR: OS");
    return FALSE;
  }
  return TRUE;
}
#endif
/************************************************************************/
/* NonAtomicRdPage: not an atomic operation, but at least: lseek, read.
   To retain the file pointer as RdPage does, the file pointer is saved
   before and set back after the read operation. */
/* Both, pagesize and pagenr are limited to Rint and Rpnint respectively,
   while their product must be capable to range in Rpint. */

boolean NonAtomicRdPage(File f,
                        Rint pagesize,
                        Rpnint pagenr,
                        void *ptr)

{
  char s[80];
  Rpint oldPos;
  Rpint pnb;    // position or nbytes
  
  oldPos= lseek(f,0,SEEK_CUR);
  pnb= lseek(f,(Rpint)pagenr * (Rpint)pagesize,SEEK_SET);
  if (pnb != -1) {
    pnb= read(f,ptr,pagesize);
  }
  if (pnb != pagesize) {
    fprintf(stderr,strans("file %d, page %N: ",s),f,pagenr);
    perror("RSTFileAccess.NonAtomicRdPage: ERROR: OS");
    pnb= lseek(f,oldPos,SEEK_SET);
    return FALSE;
  }
  pnb= lseek(f,oldPos,SEEK_SET);
  return TRUE;
}

/************************************************************************/
/* NonAtomicWrPage: not an atomic operation, but at least: lseek, write.
   To retain the file pointer as WrPage does, the file pointer is saved
   before and set back after the read operation. */
/* Both, pagesize and pagenr are limited to Rint and Rpnint respectively,
   while their product must be capable to range in Rpint. */

boolean NonAtomicWrPage(File f,
                        Rint pagesize,
                        Rpnint pagenr,
                        void *ptr)

{
  char s[80];
  Rpint oldPos;
  Rpint pnb; /* position or nbytes */
  
  oldPos= lseek(f,0,SEEK_CUR);
  pnb= lseek(f,(Rpint)pagenr * (Rpint)pagesize,SEEK_SET);
  if (pnb != -1) {
    pnb= write(f,ptr,pagesize);
  }
  if (pnb != pagesize) {
    fprintf(stderr,strans("file %d, page %N: ",s),f,pagenr);
    perror("RSTFileAccess.NonAtomicWrPage: ERROR: OS");
    pnb= lseek(f,oldPos,SEEK_SET);
    return FALSE;
  }
  pnb= lseek(f,oldPos,SEEK_SET);
  return TRUE;
}

/************************************************************************/

typedef enum {
             blank,
             readBuf,
             writeBuf
             } FileBufUse;

typedef struct {
               Rpint recSize;
               Rpint cap;
               void *anchor;
               void *adr;
               void *defaultNext;
               void *next;
               FileBufUse use;
               File f;
               } filebuf;

typedef filebuf *FILEBUF;

/************************************************************************/

boolean CreateFileBuf(t_FB *buf,
                      Rpint recSize,
                      Rpint numbRecs,
                      File f) {

  FILEBUF FB;
  
  if (recSize < 1 || numbRecs < 1) {
    fprintf(stderr,"RSTFileAccess.CreateFileBuf: ERROR: recSize < 1 OR numbRecs < 1\n");
    return FALSE;
  }
  *buf= allocM(sizeof(filebuf));
  if (*buf == NULL) {
    fprintf(stderr,"RSTFileAccess.CreateFileBuf: ERROR: allocation failed\n");
    return FALSE;
  }
  FB= (FILEBUF)*buf;
  (*FB).recSize= recSize;
  (*FB).cap= recSize * numbRecs;
  (*FB).anchor= allocM((*FB).cap);
  if ((*FB).anchor == NULL) {
    fprintf(stderr,"RSTFileAccess.CreateFileBuf: ERROR: allocation failed\n");
    return FALSE;
  }
  (*FB).defaultNext= (*FB).anchor + (*FB).cap;
  (*FB).next= (*FB).defaultNext;
  (*FB).adr= (*FB).next; // means buffer has to be read in / written back
  (*FB).use= blank;
  (*FB).f= f;
  return TRUE;
}

/***********************************************************************/

boolean DisposeFileBuf(t_FB *buf) {

  FILEBUF FB;
  
  if (*buf == NULL) {
    return FALSE;
  }
  FB= (FILEBUF)*buf;
  if ((*FB).use == writeBuf) {
    if (! FlushBufBytes(*buf)) {
      return FALSE;
    }
  }
  freeM(&(*FB).anchor);
  freeM(buf);
  return TRUE;
}

/***********************************************************************/

Rint RdBufBytes(t_FB buf,
                void *adr) {

  Rpint nbytes, newPos, setPos;
  char s[160];
  FILEBUF FB= (FILEBUF)buf;
  Rint result= 1;
  
  if ((*FB).use != readBuf) {
    if ((*FB).use == blank) {
      (*FB).use= readBuf;
      /* (*FB).adr already initialized for reading by CreateFileBuf */
    }
    else {
      fprintf(stderr,"file %d: ",(*FB).f);
      fprintf(stderr,"RSTFileAccess.RdBufBytes: ERROR: not a read buffer\n");
      return 0;
    }
  }
  if ((*FB).adr == (*FB).next) {
    nbytes= read((*FB).f,(*FB).anchor,(*FB).cap);
    if (nbytes != (*FB).cap) {
      if (nbytes > 0) {
        if (nbytes >= (*FB).recSize) {
          /* at least 1 remaining legal record read */
          /* set buffer capacity corresponding to legal records: */
          (*FB).cap= nbytes / (*FB).recSize * (*FB).recSize;
          (*FB).next= (*FB).anchor + (*FB).cap;
          if (nbytes > (*FB).cap) {
            /* file pointer (at EOF) beyond legal records */
            /* set file pointer corresponding to legal records: */
            newPos= lseek((*FB).f,0,SEEK_CUR);
            setPos= lseek((*FB).f,newPos-nbytes+(*FB).cap,SEEK_SET);
          }
          result= 2;
        }
        else {
          /* error, not a legal record read */
          newPos= lseek((*FB).f,0,SEEK_CUR);
          fprintf(stderr,strans("file %d, pos: %P: ",s),(*FB).f,newPos);
          fprintf(stderr,strans("RSTFileAccess.RdBufBytes.read: ERROR: requested: %P, read: %P\n",s),(*FB).recSize,nbytes);
          /* report (*FB).recSize instead of the really requested (*FB).cap */
          return 0;
        }
      }
      else {
        if (nbytes < 0) {
          /* nbytes == -1  ==> file error */
          fprintf(stderr,"file %d: ",(*FB).f);
          perror("RSTFileAccess.RdBufBytes.read: ERROR: OS");
        }
      //else {
      //  /* nbytes == 0  ==> error: beyond EOF */
      //  /* no message! */
      //}
        return 0;
      }
    }
    (*FB).adr= (*FB).anchor;
  }
  memcpy(adr,(*FB).adr,(*FB).recSize);
  (*FB).adr+= (*FB).recSize;
  return result;
}

/***********************************************************************/

boolean WrBufBytes(t_FB buf,
                   void *adr) {

  Rpint nbytes, newPos, setPos;
  char s[160];
  FILEBUF FB= (FILEBUF)buf;
  
  if ((*FB).use != writeBuf) {
    if ((*FB).use == blank) {
      (*FB).use= writeBuf;
      (*FB).adr= (*FB).anchor; /* initialize (*FB).adr for writing */
    }
    else {
      fprintf(stderr,"file %d: ",(*FB).f);
      fprintf(stderr,"RSTFileAccess.WrBufBytes: ERROR: not a write buffer\n");
      return FALSE;
    }
  }
  else if ((*FB).adr == (*FB).next) {
    nbytes= write((*FB).f,(*FB).anchor,(*FB).cap);
    if (nbytes != (*FB).cap) {
      if (nbytes >= 0) {
        /* pretend buffer not written at all: */
        /* file pointer was moved, reset file pointer: */
        newPos= lseek((*FB).f,0,SEEK_CUR);
        setPos= lseek((*FB).f,newPos-nbytes,SEEK_SET);
        fprintf(stderr,strans("file %d, pos: %P: ",s),(*FB).f,setPos);
        fprintf(stderr,"RSTFileAccess.WrBufBytes.write: ERROR: buffer could not be written\n");
      }
      else {
        /* nbytes == -1  ==> file error */
        /* file pointer was not moved */
        fprintf(stderr,"file %d: ",(*FB).f);
        perror("RSTFileAccess.WrBufBytes.write: ERROR: OS");
      }
      return FALSE;
    }
    (*FB).adr= (*FB).anchor;
  }
  memcpy((*FB).adr,adr,(*FB).recSize);
  (*FB).adr+= (*FB).recSize;
  return TRUE;
}

/***********************************************************************/

boolean FlushBufBytes(t_FB buf) {

  Rpint nbytes, newPos, setPos;
  char s[160];
  FILEBUF FB= (FILEBUF)buf;
  Rpint cap= (*FB).adr - (*FB).anchor;
  
  if ((*FB).use != writeBuf) {
    fprintf(stderr,"file %d: ",(*FB).f);
    fprintf(stderr,"RSTFileAccess.FlushBufBytes: ERROR: blank, or not a write buffer\n");
    return FALSE;
  }
  nbytes= write((*FB).f,(*FB).anchor,cap);
  if (nbytes != cap) {
    if (nbytes >= 0) {
      /* pretend buffer not written at all: */
      /* file pointer was moved, reset file pointer: */
      newPos= lseek((*FB).f,0,SEEK_CUR);
      setPos= lseek((*FB).f,newPos-nbytes,SEEK_SET);
      fprintf(stderr,strans("file %d, pos: %P: ",s),(*FB).f,setPos);
      fprintf(stderr,"RSTFileAccess.FlushBufBytes.write: ERROR: buffer could not be written\n");
    }
    else {
      /* nbytes == -1  ==> file error */
      /* file pointer was not moved */
      fprintf(stderr,"file %d: ",(*FB).f);
      perror("RSTFileAccess.FlushBufBytes.write: ERROR: OS");
    }
    return FALSE;
  }
  (*FB).adr= (*FB).anchor;
  return TRUE;
}

/***********************************************************************/
void PrintFileBufDesc(t_FB buf);

void PrintFileBufDesc(t_FB buf) {

  char s[80];
    
  fprintf(stdout,strans("FileBuf:              %P\n",s),(Rpint)buf);
  if (buf == NULL) {
    return;
  }
  FILEBUF FB= (FILEBUF)buf;
  fprintf(stdout,strans("FileBuf.recSize:      %P\n",s),(*FB).recSize);
  fprintf(stdout,strans("FileBuf.cap:          %P\n",s),(*FB).cap);
  fprintf(stdout,strans("FileBuf.anchor:       %P\n",s),(Rpint)(*FB).anchor);
  fprintf(stdout,strans("FileBuf.adr:          %P\n",s),(Rpint)(*FB).adr);
  fprintf(stdout,strans("FileBuf.defaultNext:  %P\n",s),(Rpint)(*FB).defaultNext);
  fprintf(stdout,strans("FileBuf.next:         %P\n",s),(Rpint)(*FB).next);
  fprintf(stdout,strans("FileBuf.use:          %d\n",s),(*FB).use);
  fprintf(stdout,strans("FileBuf.f:            %d\n",s),(*FB).f);
}

/***********************************************************************/

