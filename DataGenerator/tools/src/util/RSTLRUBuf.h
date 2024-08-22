/* -----  RSTLRUBuf.h  ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTLRUBuf_h
#define __RSTLRUBuf_h


/**:   LRU Buffer
       ==========                                                       **/


/**    Implementation:  Norbert Beckmann
              Version:  9.1
                 Date:  07/14                                           **/

/** Definitions:
    ============
    "page", "page size":
    The content of a file is considered as a sequence of equal sized pages.
    The size of a page is the number of bytes it contains.
    "available":
    A page is available in a buffer if a (possibly meanwhile modified) copy
    of the page of the corresponding file is stored in the buffer. After file
    extensions have been performed using the buffer, pages may be available,
    but not (yet) stored in the underlying file.
    "EOF":
    The end of a file is called EOF.
    "page fault":
    The event, that a requested page is not available is called page fault.
    In case of a page fault the buffer algorithm may be able to fetch the
    requested page from the corresponding file (if the request is located in
    front of EOF), or not (if the request points behind EOF (file extension
    during writes)).
      
    Main Principle:
    ===============
    An LRU buffer partly mirrors the pages of certain files in main memory.
    At start time the buffer is empty. Requested pages, not available in the
    buffer, are fetched from an indicated file or created for an indicated
    file (extension), and then kept in the buffer. This increases the number
    of buffered pages. If the number of buffered pages meets the capacity of
    the buffer, the next request of a not available page leads to the
    displacement of the page not requested the longest time ago.
    
    Interface:
    ==========
    Identification of buffered pages:
    Pages are of a fixed size, defined when the LRU buffer is created. They
    are identified by variables of type FPst, a struct, containing the file
    identifier f and the page number p. The number p of a page is assumed to
    be calculated as p = pos / size, where pos is the start position of the
    page in the indicated file, and size is its size (both in bytes).
    
    Accessing buffered pages:
    Buffered pages are internally stored in buffer entries, and the fully
    equipped buffer accessing functions provide a reference to such an entry
    through a variable of type refLRUentry (see below).
    Simplified and less powerful functions are also provided. They directly
    return a pointer to the requested page.
    
    Properties:
    ===========
    - Buffer pages are accessed via reference.
      With the call of a buffer accessing function, e.g. HashLRUEntry(), the
      address of the buffered page is provided in the field
      (*refLRUentry).adr. This means, the caller does not get a copy of the
      buffer page, but directly gets access to the internal storage.
      NOTE: Obviously, this also means that the complete buffer may be
      destroyed (and even the underlying file may be corrupted) if the number
      of bytes, written to a page, exceeds the page size, passed during
      buffer creation.
      
    - Buffer entries may be (multiple) locked in the buffer.
      This means that buffered pages may be prevented from being displaced.
      This feature is not intended for normal LRU operation, but reserved for
      special purposes.
      Note that, apart from initializing NEW buffer entries to "not locked",
      none of the functions in this module modifies the lock state.
      
    Level: intermediate.
    Higher levels: RSTLRUPageIO.h                                       **/


#include "RSTStdTypes.h"
#include "RSTFPstSet.h"
#include "RSTFPstBag.h"


/* ----- types ----- */
/*
typedef struct {
               File    f;
               Rpnint  p;
               } FPst;
               
This type is analogously declared in RSTStdTypes.h
*/
typedef struct {
               void     *adr;
               boolean  write;
               Rint     locked;
               } LRUentry, *refLRUentry;
               
               /* LRUentry:     LRU buffer entry
                  
                  LRU buffer entries are internally allocated by the LRU
                  buffer algorithm. To get access to a buffer entry, the
                  address of a variable of type refLRUentry has to be passed
                  to the buffer accessing functions. These functions assign
                  the address of the internal buffer entry to the variable.
                  
                  fields:
                  adr:     Reference to the requested buffered page, whose
                           content may be rewritten (in this case also write
                           should be set to TRUE).
                  write:   Flag: if write == TRUE, the page is written to the
                           corresponding file if it is displaced, or if the
                           corresponding file is synchronized, see also
                           SaveLRU() and LRUCloseFile().
                  locked:  Flag: if locked > 0, displacement is inhibited.
                           This inhibits the LRU mechanism for the concerning
                           page, i.e. the page is kept in the buffer, even if
                           it is never demanded again. The use of locks is
                           not intended for normal LRU operation, and has to
                           be designed very carefully! */


typedef t_LRUbuf  *t_LRU;

        /* abstract data type LRUbuffer */


/* ---- other ---- */

/* The parameter "Done"
   ====================
   The LRU buffer maintains an internal state, which is initially set to
   noError, but during operation may be set to some value indicating an
   error. Most of the functions of this header file contain a boolean
   parameter Done, which is set to FALSE if the internal state indicates an
   error.
   NOTE that the parameter Done is intended to maintain a success-variable in
   the callers environment, once set to TRUE at program start. It should keep
   FALSE after an error occurred, though passed through multiple functions.
   Hence Done is NEVER set to TRUE.
   It is set to FALSE if an error occurred.
   Errors additionally produce messages on stderr.
   
   Displacement Errors
   ===================
   The following only concernes buffer handling using locks.
   Pages of entries with LRUentry.locked > 0 are not displaced.
   When the number of locked entries meets the capacity of the buffer,
   displacement is inhibited completely. See "locked" in type LRUentry.
   When displacement is impossible, a newly requested page triggering a
   page fault is ANYHOW MADE AVAILABLE in the buffer. Actually the
   occurence of displacement errors does not affect the usability of the
   buffer. For strongly overfilled buffers however, the response time will
   suffer because hashing degenerates.
   In case of a displacement error (actually only a warning), a message is
   emitted on stderr and the parameter Done, passed to the applied buffer
   utilization function, is set to FALSE as for all other errors.
   In this case the buffer may be newly created with a greater
   capacity by the sequence SaveLRU(), DisposeLRU(), NewLRU(), or (better)
   its capacity may be extended by ModifLRUCap().
   
   This is a lower level module and speed is a main issue. As a consequence
   - given pointers are considered to be valid references without check
   - page sizes are assumed to be met
   and so on. */


/* ----- functions ----- */

int LRUError(t_LRU  LRU);

/* Returns the number of the last error. See also RSTErrors.h. */


/*** --------------------------------------------------------------------- */
/*** ----------------------- Buffer administration ----------------------- */

void NewLRU(t_LRU    *LRU,
            Rpnint   cap,
            Rint     pageSize,
            boolean  *Done);

/* NewLRU() initializes an LRU buffer, allocates its memory, and in LRU
   provides a reference to it.
   If successful, it initializes the internal state to noError, whereas Done
   is left untouched (see parameter "Done").
   
   cap:
   capacity, maximum number of buffered entries (pages).
   
   pageSize:
   size of a page in bytes. */


void DisposeLRU(t_LRU  *LRU);

/* DisposeLRU() removes the LRU buffer, referenced by LRU, deallocates its
   memory, and sets LRU to NULL. */


void SaveLRU(t_LRU    LRU,
             boolean  *Done);

/* SaveLRU() synchronizes the associated files with the buffer.
   It searches the LRU buffer for entries with LRUentry.write == TRUE.
   For each such entry, it writes the concerning page to its file and resets
   LRUentry.write to FALSE. */


void LRUCloseFile(t_LRU    LRU,
                  File     f,
                  boolean  *Done);

/* LRUCloseFile() searches the LRU buffer for entries, buffering pages of
   file f. For each of these entries: If LRUentry.write == TRUE, the
   corresponding page is written to f. Finally all entries, buffering pages
   of file f, are removed from the buffer.
   If Done == FALSE, pages to be written to f remain in the buffer, and there
   is probably an issue with the file (system).
   NOTE: The call of this function is MANDATORY BEFORE CLOSING a buffered
         file. Failing to call this function may lead to the following
         defects:
         - The file may be corrupted due to pending writes.
         - Since file identifiers are re-used, a subsequently opened file may
           also be corrupted when it is buffered.
         - The buffer's performance may shrink due to the storage of unused
           pages. */


void LRUSyncFile(t_LRU lru,
                 File f,
                 boolean *Done);

/* LRUSyncFile() searches the LRU buffer for entries, buffering pages of
   file f. For each of these entries: If LRUentry.write == TRUE, the
   corresponding page is written to f. The entries (pages) stay in the
   buffer.
   If Done == FALSE, some page could not be written, and there is probably
   an issue with the file (system). */


void LRUSuspendFile(t_LRU lru,
                    File f,
                    boolean *Done);

/* LRUSuspendFile() searches the LRU buffer for entries, buffering pages of
   file f, and tries to delete them. None of these entries should have to be
   written back.
   The function will only succeed if for all concerned entries
   LRUentry.write == FALSE. Otherwise Done is set to FALSE, a message is
   emitted on stderr, the function returns immediately, and pages to be
   written to f remain in the buffer. */


void ModifLRUCap(t_LRU    LRU,
                 Rpnint   cap,
                 boolean  *Done);

/* ModifLRUCap() tries to modify the number of buffer pages of the LRU buffer
   referenced by LRU.
   
   cap:
   new capacity of the LRU buffer referenced by LRU.
   
   If successful, ModifLRUCap() (re)initializes the internal state to
   noError, whereas Done is left untouched (see parameter "Done").
   
   The function does not cause any secondary memory I/O operation.
   The function fails if cap is smaller than the number of entries
   currently in use, or if the concerning memory allocation fails.
   
   ModifLRUCap() may be called during normal operation, or when a growing
   number of locks inhibited displacement. See "locked" in type LRUentry.
   See also GetLRUCapUse(). */


boolean LRUConsistent(t_LRU    LRU,
                      boolean  *Done);

/* LRUConsistent() checks whether the LRU buffer is consistent and returns
   TRUE if it is, otherwise FALSE.
   Errors inhibiting the consistence check itself, additionally set Done to
   FALSE.
   
   LRUConsistent() sets the internal state to noError at its entrance,
   whereas Done is left untouched (see parameter "Done").
   
   NOTE: LRUConsistent() is expensive. Its cost depends on the buffer size
         and on the ratio (buffer size) / (file size).
         LRUConsistent() was designed for tests after code modifications. It
         should not be called during normal operation. */


/*** --------------------------------------------------------------------- */
/*** -------------------- Buffer accessing functions --------------------- */

/***************************************************************************/
/* NOTE that there is NO OPENING FUNCTION like                             */
/* void OpenLRUBuf(t_LRU lru, File file);                                  */
/* The demand of buffered access to a file is simply accomplished by calls */
/* of one of the following buffer accessing functions, without any opening */
/* celebration before. It is obvious however, that the underlying file     */
/* should be open (and possibly writable) when these functions are called. */
/*                              ---------                                  */
/* NOTE that BEFORE CLOSING a buffered file, LRUCloseFile() MUST BE        */
/* CALLED.                                                                 */
/***************************************************************************/


/*** ------------ Buffer accessing functions (fully equipped) ------------ */

/***************************************************************************/
/* +++++   Note, that for merely normal LRU operation, simplified    +++++ */
/* +++++  buffer access functions are provided in the next section.  +++++ */
/***************************************************************************/

boolean /*AVAILABLE*/ HashLRUEntry(t_LRU        LRU,
                                   FPst         filePage,
                                   refLRUentry  *refEntry,
                                   boolean      *Done);

/* HashLRUEntry() makes the page, identified by filePage, accessible through
   refEntry.
   //----------------------------------------------------------------
   In case of a page fault, the page is attempted to be read from the
   corresponding file.
   //----------------------------------------------------------------
   The function returns TRUE if the page is AVAILABLE in the buffer,
   otherwise FALSE; i.e. FALSE ON PAGE FAULT.
   Entry initialization:
   Available entries are provided as they are.
   Entries of pages, fetched from secondary memory, are initialized.
   See below: "Concerns both functions".
   
   Apart from the result of the function there is a twofold success feedback.
   See below: "Concerns both functions". */


boolean /*NEW*/ NewLRUEntry(t_LRU        LRU,
                            FPst         filePage,
                            refLRUentry  *refEntry,
                            boolean      *Done);

/* NewLRUEntry() makes the page, identified by filePage, accessible through
   refEntry.
   //----------------------------------------------------------------------
   In case of a page fault, the page is NOT attempted to be READ from the
   corresponding file. Instead, an empty page is established in the buffer.
   //----------------------------------------------------------------------
   The function returns TRUE if the page is NEWLY CREATED in the buffer,
   otherwise FALSE; i.e. TRUE ON PAGE FAULT.
   Entry initialization:
   Available entries are provided as they are.
   Entries of newly created pages, are initialized.
   See below: "Concerns both functions".
   
   Apart from the result of the function there is a twofold success feedback.
   See below: "Concerns both functions". */


/* Concerns both functions, HashLRUEntry() and NewLRUEntry():
   ==========================================================
   Except for the page mapping, the LRU buffer "knows" precisely nothing
   about the files it mirrors. Hence the caller is responsible for the
   complete file-administration and -monitoring.
   When the LRU buffer tries to access a certain page on behalf of the
   caller, the accessibility of the page is fed back through parameter
   refEntry (see below), and in case of an error, a message is printed on
   stderr.
   
   Application of the functions
   ----------------------------
   The intended task of HashLRUEntry() is reading and writing (updating)
   pages in front of EOF.
   The intended task of NewLRUEntry() is writing (appending) pages behind
   EOF.
   
   However, both functions, first of all, try to deliver a page being
   available (the reference to the buffer entry containing it), as the
   following table shows:
   
   page status:          available                 not available
   --------------------------------------------------------------------------
   HashLRUEntry:       page provided     page tried to be fetched from file
   NewLRUEntry:        page provided           page newly established
   
   Both functions yield 3 different independend feedbacks:
   - their returned result (TRUE / FALSE)   
   - the value of the parameter Done (TRUE / FALSE)
   - the value of the parameter refEntry (non-NULL / NULL).
   This allows a very controlled, and thus save, operation on buffered files,
   especially when locks are in use.
   - The result is returned, as follows:
     HashLRUEntry() returns TRUE if a page is available, otherwise FALSE.
     NewLRUEntry() returns FALSE if a page is available, otherwise TRUE.
   - For any kind of error, Done is set to FALSE, and a message is written to
     stderr. But there are fatal and non fatal errors. In case of a non fatal
     error, the requested page is anyhow accessible.
   - If a requested page is not accessible (be it an existing or a new one),
     refEntry is set to NULL, otherwise it is set to a valid buffer entry
     reference.
     
     Examples of fatal errors:
     HashLRUEntry() cannot access the underlying file.
     NewLRUEntry() cannot allocate enough memory for a new buffer entry.
     
     The sole non fatal error occurs when displacement fails.
     Displacement errors may only occur when locks are in use.
     If Done == FALSE and refEntry != NULL, a displacement error has
     occurred, which actually corresponds to a warning. For further
     information see: LRUError(), GetLRUCapUse(), ModifLRUCap(),
     "displacement" in RSTErrors.h.
     
   After writing to a page, provided in (*refentry).adr, and setting
   (*refentry).write to TRUE, the page HAS BEEN UPDATED in the buffer. The
   corresponding file will then be synchronized later, when the page might be
   displaced, triggered by another call of HashLRUEntry() or NewLRUEntry().
   Synchronization for all buffered pages (of a file) may be forced by the
   functions SaveLRU() or LRUCloseFile(); the latter MUST be called before a
   buffered file is closed.
   
   Entry initialization
   --------------------
   New LRU buffer entries are initialized as follows:
   LRUentry.adr = (address of the requested buffer page);
   LRUentry.write = FALSE;
   LRUentry.locked = 0.
   
   At their entrance, both functions set the internal state to noError,
   whereas Done is left untouched (see parameter "Done"). */


boolean /*AVAILABLE*/ DisposeLRUEntry(t_LRU    LRU,
                                      FPst     filePage,
                                      boolean  *Done);

/* DisposeLRUEntry() works as follows:
   If the entry corresponding to filePage is available in the LRU buffer and
   is locked at most once, it is deleted from it. The entry is not written to
   the underlying file before, even if the write flag of the entry is set to
   TRUE.
   The function returns TRUE if the page is available in the buffer,
   otherwise FALSE; i.e. FALSE on page fault.
   The function does nothing under the following conditions:
   - the page is not available, i.e. FALSE is returned;
   - LRUentry.locked > 1, in this case Done is set to FALSE.
   
   NOTE that the use of this function is, like using LRUentry.locked, not
   intended for normal LRU operation. */


/*** -------------- Buffer accessing functions (simplified) -------------- */

void *LRUPgRead(t_LRU   LRU,
                File    f,
                Rpnint  pageNr);

/* LRUPgRead() provides simplified access to the address of a buffered page
   of file f. It does neither provide detailed feedback, nor does it provide
   access to the lock facility.
   The function is intended for READING from a page of file f. Thus the
   write-flag of the buffer entry is left untouched.
   
   If successful, the function returns the address of a buffered page. If a
   page is not available, it is attempted to be read from secondary memory.
   If the function fails, it returns NULL, and a message is printed to
   stderr.
   
   LRU:       The reference of an LRU buffer, previously created by the
              function NewLRU().
   f:         A file, already being buffered or newly to be buffered.
   pageNr:    The page number of the page to be read.
   
   The function expects:
   - an LRU buffer, previously created by NewLRU()
   - an open and (at least) readable file. */


void *LRUPgWrite(t_LRU   LRU,
                 File    file,
                 Rpnint  pageNr);

/* LRUPgWrite() provides simplified access to the address of a buffered page
   of file f. It does neither provide detailed feedback, nor does it provide
   access to the lock facility.
   The function is intended for WRITING to a page of file f. Thus the
   write-flag of the buffer entry is set to TRUE.
   
   If successful, the function returns the address of a buffered page. If a
   page is not available, the approach depends on the position of the page in
   the file. Pages in front of EOF are attempted to be read from secondary
   memory. Pages behind EOF are newly created in the buffer.
   If the function fails, it returns NULL, and a message is printed to
   stderr.
   
   LRU:       The reference of an LRU buffer previously created by the
              function NewLRU().
   f:         A file, already being buffered or newly to be buffered.
   pageNr:    The page number of the page to be (re-)written.
      
   The function expects:
   - an LRU buffer, previously created by NewLRU()
   - an open and readable and writable file. */


/*** --------------------------------------------------------------------- */
/*** --------------------------- Information ----------------------------- */

Rint LRUPageSize(t_LRU  LRU);

/* LRUPageSize() returns the size of a page in the buffer referenced by
   LRU. */


void GetLRUCapUse(t_LRU   LRU,
                  Rpnint  *cap,
                  Rpnint  *use);

/* GetLRUCapUse() provides information about the LRU buffer referenced by
   LRU.

   cap:
   is set to the preset capacity.
   
   use:
   is set to the number of entries actually in use.
   Displacement errors cause use > cap, whereas use = cap would be the
   maximum capacity for efficient operation. */


void InquireLRUDesc(t_LRU   LRU,
                    Rint    *pageSize,
                    Rpnint  *cap,
                    Rpnint  *use);

/* InquireLRUDesc() provides information about the LRU buffer referenced by
   LRU.
   
   pageSize:
   is set to the size of a page in the buffer.
    
   cap:
   is set to the preset capacity.
   
   use:
   is set to the number of entries actually in use.
   Displacement errors cause use > cap, whereas use = cap would be the
   maximum capacity for efficient operation. */


t_FPS LRUPages(t_LRU    LRU,
               boolean  *Done);

/* LRUPages() as an ordered set, returns file and page number of all pages
   currently stored in the LRU buffer referenced by LRU. For further
   information see: RSTFPstSet.h.
   
   Success feedback:
   - The detection of errors sets Done to FALSE.
   - If due to errors the set could not be build (completely), an empty set
     is returned. */


t_FPB LRUPagesLocked(t_LRU    LRU,
                     boolean  *Done);

/* LRUPagesLocked() provides information about locked pages in the LRU buffer
   referenced by LRU.
   As a by (file, page) ordered bag, it returns the tuple
   (#locks, (file, page)) of all entries whose number of locks != 0. For
   further information see: RSTFPstBag.h.
   
   Success feedback:
   - The detection of errors sets Done to FALSE.
   - If due to errors the bag could not be build (completely), an empty bag
     is returned. */


void PrintLRUPriority(t_LRU  LRU,
                      FILE   *stream,
                      Rint epl);

/* PrintLRUPriority() provides information about the priority list of the LRU
   buffer referenced by LRU.
   It prints the elements of the priority list to stream as a text, in lines
   of epl elements per line.
   The notation is: " f<file>.p<page> f<file>.p<page> ...". */


/*** --------------------------------------------------------------------- */
/*** ------------------- Performance control routines -------------------- */

/*** ------ Counts-Switch: */

void LRUCountsOn0(t_LRU  LRU);
         /* switch ON, set 0 */

void LRUCountsOn(t_LRU  LRU);
         /* switch ON */

void LRUCountsOff(t_LRU  LRU);
         /* switch OFF */
         
         /* The Counts-Switch applies to the complete counting facility.
            Counted values are obtainable by the LRUGetCount...  functions.
            
            NewLRU() initializes counting as with the sequence
            LRUCountsOn0(), LRUCountsOff(). */


/*** ------ Counts-Return: */

void LRUGetCountRead(t_LRU  LRU,
                     Rlint  *readCount);

/* LRUGetCountRead() provides information about read accesses triggered by
   the LRU buffer referenced by LRU.
   
   readCount:
   is set to the number of pages having been read from secondary memory. */


void LRUGetCountNew(t_LRU lru,
                    Rlint *newCount);

/* LRUGetCountNew() provides information about new pages (behind EOF)
   established in the buffer.
   
   newCount:
   is set to the number of pages having been newly established. */


void LRUGetCountWrite(t_LRU  LRU,
                      Rlint  *writeCount);

/* LRUGetCountWrite() provides information about write access triggered by
   the LRU buffer referenced by LRU.
   
   writeCount:
   is set to the number of pages having been written to secondary memory. */


void LRUGetCountAvail(t_LRU lru,
                      Rlint *availCount);

/* LRUGetCountAvail() provides information about pages having been available
   in the LRU buffer referenced by LRU.
   
   availCount:
   is set to the number of pages having been available. */

#endif /* __RSTLRUBuf_h */

