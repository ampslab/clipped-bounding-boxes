/* -----  RSTFileAccess.h  ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTFileAccess_h
#define __RSTFileAccess_h


/**:   File Access library
       ===================                                              **/


/**    Implementation:  Norbert Beckmann
              Version:  3.3
                 Date:  06/14                                           **/

/** Properties:
    If appropriate, the functions return a boolean, TRUE on success,
    FALSE otherwise. Exceptions are explicitly mentioned below.
    Apart from the exceptions mentioned in the comments, the functions
    emit messages to stderr if an error occurs.
    Level: bottom, intermediate, normal use (function-depending).
    Higher levels: RSTFilePageIO.h < RSTPageInOut.h
                   RSTLRUPageIO.h                                       **/


#include "RSTStdTypes.h"


/* constants */

#define FILE_PERM 0644

/* rw-r--r-- file permission */


/*** ---------- creating, opening, closing files ---------- ***/

boolean OpenRdOnlyF(const char *path,
                    File *f);

/* Opens the file named path for reading only.
   On success, returns TRUE,
   otherwise returns FALSE and prints a message to stderr. */


boolean OpenRdWrF(const char *path,
                  File *f);

/* Opens the file named path for reading and writing.
   On success, returns TRUE,
   otherwise returns FALSE and prints a message to stderr. */


boolean CreateExclF(const char *path,
                    File *f);

/* Creates the file named path for reading and writing.
   Does not overwrite an existing file with the same path.
   On success, returns TRUE,
   otherwise returns FALSE and prints a message to stderr. */


boolean CreateTruncF(const char *path,
                     File *f);

/* Creates the file named path for reading and writing.
   Overwrites (truncates) an existing file with the same path.
   On success, returns TRUE,
   otherwise returns FALSE and prints a message to stderr. */


boolean CloseF(File f);

/* Closes the file f.
   On success, returns TRUE,
   otherwise returns FALSE and prints a message to stderr. */


/*** ---------- miscellaneous operations on files ---------- ***/

boolean RenameF(const char *oldPath,
                const char *newPath);

/* Changes the name of a file.
   On success, returns TRUE,
   otherwise returns FALSE and prints a message to stderr. */


boolean SilRenameF(const char *oldPath,
                   const char *newPath);

/* Changes the name of a file.
   On success, returns TRUE, otherwise returns FALSE.
   Does not produce error messages. */


boolean UnlinkF(const char *path);

/* Removes (a link to) a file.
   On success, returns TRUE,
   otherwise returns FALSE and prints a message to stderr. */


boolean SilUnlinkF(const char *path);

/* Removes (a link to) a file.
   On success, returns TRUE, otherwise returns FALSE.
   Does not produce error messages. */


Rpint LenOfF(File f);

/* Returns the length of file f.
   If an error occurs, returns -1 and prints a message to stderr. */


boolean TruncateF(File f,
                  Rpint length);

/* Truncates the file f to have length bytes.
   On success, returns TRUE,
   otherwise returns FALSE and prints a message to stderr. */


boolean SetPosF(File f,
                Rpint pos);

/* Sets the file pointer of file f to pos.
   On success, returns TRUE,
   otherwise returns FALSE and prints a message to stderr. */


boolean GetPosF(File f,
                Rpint *pos);

/* Provides the current position of the file pointer of file f in pos.
   On success, returns TRUE,
   otherwise returns FALSE and prints a message to stderr. */


/*** ---------- reading, writing SUCCESSIVELY ---------- ***/

boolean RdBytes(File f,
                void *adr,
                Rpint size);

/* In file f, reads size bytes at the position of the file pointer.
   The file pointer (initially 0) is incremented by size.
   The data is written to adr.
   On success, returns TRUE.
   Returns FALSE if the read failed completely, or if less then size bytes
   are delivered.
   Emits a message to stderr if the read failed completely, or if the number
   of bytes delivered is less than size, but is not 0.
   The latter especially means that a read across EOF triggers emitting a
   message, but a read after EOF does not, though FALSE is returned.
   This allows a message free check for EOF without calling LenOfF if
   appropriate blocks are read. */


boolean WrBytes(File f,
                void *adr,
                Rpint size);

/* In file f, writes size bytes at the position of the file pointer.
   The file pointer (initially 0) is incremented by size.
   The data is read from adr.
   On success, returns TRUE,
   otherwise returns FALSE and prints a message to stderr. */


/*** ---------- reading, writing PAGES ---------- ***/

boolean RdPage(File f,
               Rint pagesize,
               Rpnint pagenr,
               void *ptr);

/* In file f, reads pagesize bytes at position (pagesize * pagenr).
   The file pointer is not used and is left unchanged.
   The data is written to ptr.
   On success, returns TRUE,
   otherwise returns FALSE and prints a message to stderr. */
/* IMPORTANT NOTE only concerning Windows:
   Under Windows the underlying system call "pread" is not available.
   Workaround: RdPage simply calls NonAtomicRdPage. */


boolean WrPage(File f,
               Rint pagesize,
               Rpnint pagenr,
               void *ptr);

/* In file f, writes pagesize bytes at position (pagesize * pagenr).
   The file pointer is not used and is left unchanged.
   The data is read from ptr.
   On success, returns TRUE,
   otherwise returns FALSE and prints a message to stderr. */
/* IMPORTANT NOTE only concerning Windows:
   Under Windows the underlying system call "pwrite" is not available.
   Workaround: WrPage simply calls NonAtomicWrPage. */


boolean NonAtomicRdPage(File f,
                        Rint pagesize,
                        Rpnint pagenr,
                        void *ptr);
/* NOT RECOMMENDED, UNUSED. But see NOTE under RdPage(). Behaves like the
   sequence GetPosF(*old), SetPosF(new), RdBytes(pagesize), SetPosF(old).
   The result is like that of RdPage().
   On success, returns TRUE,
   otherwise returns FALSE and prints a message to stderr. */


boolean NonAtomicWrPage(File f,
                        Rint pagesize,
                        Rpnint pagenr,
                        void *ptr);
/* NOT RECOMMENDED, UNUSED. But see NOTE under WrPage(). Behaves like the
   sequence GetPosF(*old), SetPosF(new), WrBytes(pagesize), SetPosF(old).
   The result is like that of WrPage().
   On success, returns TRUE,
   otherwise returns FALSE and prints a message to stderr. */


/*** ---------- BUFFERED reading/writing of EQUAL SIZED RECORDS
                         from/to the SAME FILE SUCCESSIVELY ---------- ***/
  /* Remark:
     Consecutively reading/writing equal sized small entities from/to the
     same file, using RdBytes()/WrBytes() (wrappers for system calls read and
     write respectively) may, despite all caching by the disk and the
     operating system, lead to considerable extra cost.
     For each system call two mode-switches have to be performed: the first
     from User-Mode to privileged Kernel-Mode, and then, after the call has
     been executed, back from Kernel-Mode to User-Mode. Since for the special
     system calls considered here, data has to be transferred to/from
     specific memory addresses, even memory management (the MMU) will be
     involved. Altogether this explains the time consume.
     
     
     The functions RdBufBytes(), WrBufBytes() and FlushBufBytes(), provided
     in this section, offer buffered operations for the special purpose,
     mentioned above.
     NOTE that these functions serve for buffered reading and writing only,
     whereas all other file operations, e.g. opening, closing, positioning of
     the file pointer, have to be performed on the bare file as usually. */


typedef t_FileBuf  *t_FB;

        /* abstract data type FileBuffer */


boolean CreateFileBuf(t_FB *buf,
                      Rpint recSize,
                      Rpint numbRecs,
                      File f);

/* CreateFileBuf() initializes a buffer to be used by RdBufBytes(),
   WrBufBytes() and FlushBufBytes(). It allocates its memory, and in buf
   provides a reference to it.
   
   Parameters:
   buf:       the file buffer provided,
   recSize:   the size of a record,
   numbRecs:  the maximum number of records the file buffer will store,
   f:         the file to be read/written from/to.
   
   The size of the file buffer is recSize * numbRecs bytes.
   
   Since the original number of system calls to read and write, respectively
   is divided by numbRecs in conclusion, the size of such a file buffer can
   be kept relatively small. */


boolean DisposeFileBuf(t_FB *buf);

/* DisposeFileBuf() removes a buffer created by CreateFileBuf(), and
   deallocates its memory.
   If the buffer has been used for writing (see WrBufBytes()),
   FlushBufBytes() is called before the buffer is disposed.
   
   Result of the function:
   The function returns FALSE if
   - buf is a NULL pointer
   - FlushBufBytes() failed; then the buffer is left intact.
   Otherwise TRUE is returned.
   
   The function itself does not emit error messages. */


Rint RdBufBytes(t_FB buf,
                void *adr);

/* RdBufBytes() is designed for CONSECUTIVELY reading EQUAL SIZED RECORDS
   from the SAME FILE. It is NOT an all purpose function for buffered reads.
   
   Parameters:
   The size of the records to be read, and the file to be read from,
   respectively are determined by CreateFileBuf().
   buf:  the reference to a buffer to be provided (see CreateFileBuf()),
   adr:  the memory address, the function writes to.
   
   When RdBufBytes() is applied to a virgin buffer, the buffer is marked
   for reading. Read buffers cannot be used for writing.
   
   Result of the function:
   - The function returns 1 on success.
   - The function returns 0 for reads across or behind EOF, or if a severe
     error occured.
     A message is only emitted (stderr) if an illegal (i.e. incomplete)
     record was found at EOF, or if a severe error occured (comparable to
     RdBytes()).
   - The function returns 2 if the buffer could not be filled completely,
     which commonly occurs near EOF. In that case the capacity of the buffer
     is REDUCED to the number of legal records up to the end of the file.
     
   NOTE that the buffer is only operative for a single continuous run on a
   single file. After e.g. repositioning the file pointer, the buffer has to
   be disposed, and a new one has to be created.
   
   Remark: Laxly, the result of the function can be treated as a boolean in a
   C if statement, this way reading up to the end of the file without
   handling the "result == 2" case. */


boolean WrBufBytes(t_FB buf,
                   void *adr);

/* WrBufBytes() is designed for CONSECUTIVELY writing EQUAL SIZED RECORDS
   to the SAME FILE. It is NOT an all purpose function for buffered writes.
   
   Parameters:
   The size of the records to be written, and the file to be written to,
   respectively are determined by CreateFileBuf().
   buf:  the reference to a buffer to be provided (see CreateFileBuf()),
   adr:  the memory address, the function reads from.
   
   When WrBufBytes() is applied to a virgin buffer, the buffer is marked
   for writing. Write buffers cannot be used for reading.
   
   Result of the function:
   - The function returns TRUE on success.
   - The function returns FALSE if an error occured.
     In this case a message is emitted on stderr.
   
   NOTE that the buffer is only operative for a single continuous run on a
   single file. After e.g. repositioning the file pointer, the buffer has to
   be disposed, and a new one has to be created. */


boolean FlushBufBytes(t_FB buf);

/* FlushBufBytes() flushes the buffer buf, i.e. it writes the pending records
   to the corresponding file. If the buffer is not a  write buffer, FALSE is
   returned, and the buffer is left untouched.
   
   Result of the function:
   - The function returns TRUE on success.
   - The function returns FALSE if an error occured.
     In this case a message is printed to stderr. */


#endif /* __RSTFileAccess_h */

