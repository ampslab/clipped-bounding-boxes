/* -----  RSTOtherFuncs.h  ----- */
#//
#// Copyright (c) 1994 - 2015 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTOtherFuncs_h
#define __RSTOtherFuncs_h


/**:   RSTree - a C frame for R-tree like structures, MAIN INTERFACE
       =============================================================    **/


/**    Implementation:  Norbert Beckmann
              Version:  4.2
                 Date:  06/15                                           **/
                 
/**    Implemented tree:                                                **/

/**:   Quadratic R-tree, R*-tree, RR*-tree
                                                                        **/
/**:   INTERFACE (functions, except distance query function)
       =====================================================            **/

/**    Level: normal use.                                               **/


/**    Notice: NOTE, SYS_BUILD, RESTRICT.                               **/

/* NOTE:
   The implementation should work under all modern C based 64 and 32
   bit operating systems.
   It is tested under Solaris, openindiana, ubuntu, FreeBSD,
   Windows 7 with minGW installed.
   
   Constants and types below labeled with "SYS_BUILD" should be adapted
   partly during library build, to compile the desired version of the
   index structure. */

/*** *********************************************************************/

#include "RSTTypes.h"
#include "RSTQueryFuncType.h"
#include "RSTLRUBuf.h"

/*** --------------------- function declarations ----------------------- */

/*
                                 NOTE:
      Almost all functions return a boolean result. If a function returns
      TRUE, its task was successfully completed. If a function returns FALSE,
      an error has occurred. RSTree attempts to detect errors before any
      update operations are performed. In case of an error the function
      immediately returns to the caller and emits a message to stderr. The
      number of the last error may be inquired by the function RSTError.
      See also RSTErrors.h.
*/

int      RSTError(t_RT  rst);

         /* Returns the number of the last error. See also RSTErrors.h. */


void     InitRSTreeIdent(t_RT  *rst);

         /* Initializes a tree identifier (at least sets rst to NULL).
            Each function which requires a tree identifier checks the value
            of this identifier at its entry.
            OpenRST for example demands an initial identifier while CloseRST
            demands a non initial identifier. */


/*** ----------- managing secondary memory residing trees -------------- */

boolean  CreateRST(const char  *name,
                   Rint        dirPageSize,
                   Rint        dataPageSize,
                   Rint        numbOfDim,
                   Rint        infoSize,
                   boolean     unique);

         /* CreateRST creates a tree on secondary memory.
            To work on it, it has to be opened by OpenRST or OpenBufferedRST.
            
            name:
            is the main filename under which the created tree will be stored,
            additionally some files named filename.suffix with different
            suffixes will be created. name is not fixed in the internal
            parameter table, thus after renaming the files, the tree may be
            opened under another name.
            
            dirPageSize, dataPageSize:
            is the size in bytes, a page (directory, data resp.) will occupy.
            
            numbOfDim:
            is the dimensionality, the number of dimensions of the rectangles
            to be stored.
            
            infoSize:
            is the size (in bytes) of the information part, to be passed with
            each rectangle. The smallest possible size is sizeof(Rpnint).
            
            unique:
            if unique is set TRUE the function InsertRecord will deny to
            store a record with a rectangle (key) already existing in the
            tree (rectangles will be real keys).
            The unique flag may be reset by the function SetUnique
            without further internal tests. */


boolean  RemoveRST(const char  *name);

         /* RemoveRST removes all files corresponding to the tree named
            name. */


boolean  TryRemoveRST(const char  *name);

         /* TryRemoveRST works like RemoveRST, but does not emit error
            messages. */


boolean  OpenRST(t_RT        *rst,
                 const char  *name);

         /* OpenRST opens the tree named name and in rst provides a
            reference to it. */


boolean  OpenBufferedRST(t_RT        *rst,
                         const char  *name,
                         t_LRU       LRU);

         /* OpenBufferedRST opens the tree named name and in rst provides
            a reference to it.
            Contrary to OpenRST, IOs are buffered in main memory.
            A sufficiently large LRU buffer has to be provided, whose
            reference is demanded in LRU.
            NOTE that the buffer serves for both, directory an data pages,
            which are expected to be of the same size. Thus trees with
            different sized directory and data pages cannot be opened by
            OpenBufferedRST.
            See also RSTLRUBuf.h. */


boolean  CloseRST(t_RT  *rst);

         /* CloseRST tries to close the tree referenced by rst. It
            - writes back important parameters
            - writes back path buffer and root
            - (possibly) writes back the LRU buffer.
            If all went fine, it closes the tree referenced by rst,
            reinitializes rst and returns TRUE.
            Otherwise it leaves the tree open and returns FALSE. */


/*   --- Synchronization ---   */
/*   Synchronization may be a faster alternative for closing a tree and
     re-opening it by another process.
     
     One and the same existing secondary memory residing R-tree can be opened
     by multiple processes, and such processes can perform queries
     concurrently. This is guaranteed by the operating system.
     One of these processes can also update the concerned tree if a suitable
     transaction management guarantees the other processes being suspended
     during the update period.
     After a process has performed updates, it should synchronize the tree
     (its files), whereas the other processes should attach to the newly
     synchronized tree. Thereafter concurrent querying can be continued.
   
     The following two functions provide the necessary operations for
     synchronization; the management of LRU buffers (see RSTLRUBuf.h),
     possibly utilized by the calling processes for the tree, is included. */

boolean  SyncRST(t_RT  rst);

         /* SyncRST synchronizes the files corresponding to rst with the
            buffered context (files adopt buffered context). */


boolean  GetRSTSync(t_RT  rst);

         /* GetRSTSync synchronizes the buffered context with the files
            corresponding to rst (buffers adopt file context). */


/*** ------------ managing main memory residing trees --------------- */

/*   Main memory residing trees are kept on two separate RAM disks; one for
     the directory levels, the other for the data level. In other respects
     they are treated like hard disk residing trees. NOTE that particularly
     the behavior of the Get/Put counters is the same.
     See also "performance control routines".
     
     RSTree provides early detection of memory limitation, which sets a lock,
     and a procedure to extend the RAM disks if they are locked. The
     constants below control the behavior of this facility. */

#define LO_RAM_SIZE      100000000
#define HI_RAM_EXT_FAC   2.0

#define STD_RAM_EXT_FAC  M_SQRT2

#define HI_RAM_SIZE     1000000000
#define LO_RAM_EXT_FAC   1.1

/*   The lock is tested in advance of any update operation (queries are
     possible on locked trees). Whensoever one of the RAM disks is detected
     to be locked, it is attempted to be extended by a specific factor,
     depending on its current size, as follows:
     if RAM_disk_size < LO_RAM_SIZE, HI_RAM_EXT_FAC is applied;
     else if RAM_disk_size > HI_RAM_SIZE, LO_RAM_EXT_FAC is applied;
     else STD_RAM_EXT_FAC is applied.
     The extension is performed once per lock detection (SYS_BUILD: requires
     a sufficiently large extension factor). */


boolean  CreateMainMemRST(t_RT     *rst,
                          Rint     dirPageSize,
                          Rint     dataPageSize,
                          Rint     numbOfDim,
                          Rint     infoSize,
                          boolean  unique,
                          Rpint    dirRAMsize,
                          Rpint    dataRAMsize,
                          boolean  verboseRAMdiskExt);

         /* CreateMainMemRST creates (and opens) a tree on RAM disks in the
            main memory. See also CreateRST.
            
            dirRAMsize, dataRAMsize:
            number of bytes to be allocated for the directory and data part
            of the tree respectively. The minimum is 3 * page size.
            
            verboseRAMdiskExt:
            If verboseRAMdiskExt is set to TRUE, a message is written to
            stderr at each RAM disk extension, otherwise a message is only
            emitted if the concerning attempt fails. */


boolean  RemoveMainMemRST(t_RT  *rst);

         /* RemoveMainMemRST removes a main memory residing tree and
            deallocates the concerning RAM disks. It reinitializes rst. */


boolean  LoadRST(t_RT        *rst,
                 const char  *name,
                 Rpint       dirRAMsize,
                 Rpint       dataRAMsize);

         /* LoadRST loads a secondary memory residing tree into the main
            memory.
            LoadRST may be an alternative for OpenRST, if access intensive
            operations have to be performed.
            See also OpenRST, CreateMainMemRST, SaveRST.
            
            dirRAMsize, dataRAMsize:
            see CreateMainMemRST. If these parameters are set to 0, the RAM
            disks are allocated with the corresponding file sizes and
            locked. Queries are possible on locked trees. */


boolean  SaveRST(t_RT        rst,
                 const char  *name);

         /* SaveRST stores a main memory residing tree on secondary memory
            under a given name. See also CreateMainMemRST. */


/*** -------- information concerning main memory residing trees -------- */

boolean  GetRAMdiskLimits(t_RT     rst,
                          Rpint    *dirRAMsize,
                          Rpint    *dataRAMsize,
                          Rpint    *dirRAMutil,
                          Rpint    *dataRAMutil,
                          boolean  *dirLocked,
                          boolean  *dataLocked);

         /* GetRAMdiskLimits provides information about size and utilization
            of the RAM disk.
            
            dirRAMsize, dataRAMsize:
            are set to the size in bytes of the directory and data RAM disk.
            
            dirRAMutil, dataRAMutil:
            are set to the size in bytes of the part of the directory and
            data RAM disk currently utilized.
            
            dirLocked, dataLocked:
            A RAM disk may be locked due to early detection of memory
            limitation. Then the corresponding flag is set to TRUE.
            
            If the tree does not reside in main memory, the function returns
            FALSE.
            If the function returns FALSE, all variables are set to 0. */


/*** ---------------------- universal information ---------------------- */

void     PrintRSTImplLimits(void);

         /* PrintRSTImplLimits prints the limits of this implementation
            to stdout. */


boolean  InquireRSTDesc(t_RT     rst,
                        char     *name,
                        Rint     *subtreePtrSize,
                        Rint     *infoSize,
                        Rint     *numbOfDimensions,
                        Rint     *dirPageSize,
                        Rint     *dataPageSize,
                        Rint     *netDirPageSize,
                        Rint     *netDataPageSize,
                        Rint     *dirEntrySize,
                        Rint     *dataEntrySize,
                        Rint     *maxDirFanout,
                        Rint     *maxDataFanout,
                        Rint     *minDirFanout,
                        Rint     *minDataFanout,
                        Rint     *minDirDELrest,
                        Rint     *minDataDELrest,
                        Rpnint   *numbOfDirPages,
                        Rpnint   *numbOfDataPages,
                        Rlint    *numbOfRecords,
                        Rint     *rootLevel,
                        boolean  *unique,
                        Rpnint   pagesPerLevel[]);

         /* InquireRSTDesc provides some useful information about the tree
            referenced by rst.
            
            name: see CreateRST.
            
            subtreePtrSize:
            contains the size (in bytes) of a page reference.
            
            infoSize:
            contains the size (in bytes) of the information part, associated
            with each rectangle.
            
            numbOfDimensions:
            contains the number of dimensions of the tree referenced by rst,
            i.e. the value, the variable numbOfDim was set to, when it was
            created.
            
            dirPageSize, dataPageSize: see CreateRST.
            
            netDirPageSize, netDataPageSize:
            contain the size (in bytes) of the part of a directory and data
            page respectively, reserved for entries (a fraction of the page
            size).
            
            dirEntrySize, dataEntrySize:
            contain the size (in bytes) a directory and data entry
            respectively actually requires in a page (alignment gaps
            included). 
            
            maxDirFanout, maxDataFanout:
            contain the maximum possible number of entries a directory
            and data page respectively can store.
            
            minDirFanout, minDataFanout:
            contain the minimum number of entries a directory and data page
            respectively will store.
            
            minDirDELrest, minDataDELrest:
            contain the minimum number of entries of a directory and data
            page respectively during deletion. Pages which after deletion
            contain less than min?DELrest entries are treated as underfilled.
            
            numbOfDirPages, numbOfDataPages:
            total number of directory and data pages respectively in use.
            
            numbOfRecords:
            total number of data records stored in the tree.
            
            rootLevel:
            root level of the tree, the lowest possible root level is 0, the
            tree then exclusively consists of the root as a data page.
            Levels count bottom up.
            
            unique: see CreateRST.
            
            pagesPerLevel:
            For each level i, beginning at the data level, pagesPerLevel[i]
            contains the number of pages in use. i: [0 .. rootLevel]. */


boolean  GetCreatRSTDesc(t_RT     rst,
                         char     *name,
                         Rint     *dirPageSize,
                         Rint     *dataPageSize,
                         Rint     *numbOfDimensions,
                         Rint     *infoSize,
                         boolean  *unique);

         /* GetCreatRSTDesc provides the subset of the tree characteristics
            set during creation.
            Except for name and unique, for which the actual values are
            reported, these parameters are constant over the life cycle of a
            tree.
            
            See InquireRSTDesc for parameter description. */


boolean  GetVarRSTDesc(t_RT     rst,
                       char     *name,
                       Rpnint   *numbOfDirPages,
                       Rpnint   *numbOfDataPages,
                       Rlint    *numbOfRecords,
                       Rint     *rootLevel,
                       boolean  *unique,
                       Rpnint   pagesPerLevel[]);

         /* GetVarRSTDesc provides the subset of the tree characteristics
            which may vary during use.
            
            See InquireRSTDesc for parameter description. */


boolean  GetCapParams(t_RT  rst,
                      Rint  *subtreePtrSize,
                      Rint  *infoSize,
                      Rint  *numbOfDimensions,
                      Rint  *dirPageSize,
                      Rint  *dataPageSize,
                      Rint  *netDirPageSize,
                      Rint  *netDataPageSize,
                      Rint  *dirEntrySize,
                      Rint  *dataEntrySize);

         /* GetCapParams provides the most important size information.
            
            See InquireRSTDesc for parameter description. */


boolean  GetPageSizes(t_RT  rst,
                      Rint  *dirPageSize,
                      Rint  *dataPageSize);

         /* GetPageSizes provides the size of a directory and data page
            respectively.
            
            See InquireRSTDesc for parameter description. */


boolean  GetPagesRecords(t_RT    rst,
                         Rpnint  *numbOfDirPages,
                         Rpnint  *numbOfDataPages,
                         Rlint   *numbOfRecords);

         /* GetPagesRecords provides volume information.
            
            See InquireRSTDesc for parameter description.
            If the function returns FALSE, the variables are set to 0. */


boolean  GetMaxFanout(t_RT  rst,
                      Rint  *maxDirFanout,
                      Rint  *maxDataFanout);

         /* GetMaxFanout provides fanout information.
            
            See InquireRSTDesc for parameter description.
            If the function returns FALSE, the variables are set to 0. */


boolean  GetRootLevel(t_RT  rst,
                      Rint  *rootLevel);

         /* rootLevel:
            is set to the root level of the tree, the lowest possible root
            level is 0 (the tree then exclusively consists of the root as a
            data page).
            
            If the function returns FALSE, rootLevel is set to 0. */


boolean  GetHeight(t_RT  rst,
                   Rint  *height);

         /* height:
            is set to the height of the tree, the minimal height is 1 (the
            tree then exclusively consists of the root as a data page).
            
            If the function returns FALSE, height is set to 0. */


boolean  GetName(t_RT  rst,
                 char  *name);

         /* name:
            is set to the name of the tree. Main memory residing trees bear
            some system defined name. */


boolean  GetGlobalMBB(t_RT         rst,
                      Rint         *numbOfRootEntries,
                      typinterval  *mbb);

         /* GetGlobalMBB provides the MBB the tree covers.
            The MBB is newly calculated from the entries of the root.
            NOTE: GetRootMBB will provide the information faster.
            
            numbOfRootEntries:
            contains the number of entries of the root.
            
            mbb:
            contains the minimum bounding box of the root if
            numbOfRootEntries > 0, otherwise contains garbage. */


boolean  GetRootMBB(t_RT         rst,
                    Rint         *numbOfRootEntries,
                    typinterval  *mbb);

         /* GetRootMBB provides the MBB the tree covers.
            The MBB is copied from the internal storage.
            
            numbOfRootEntries:
            contains the number of entries of the root.
            
            mbb:
            contains the minimum bounding box of the root if
            numbOfRootEntries > 0, otherwise contains garbage. */


boolean  GetStorageKind(t_RT  rst,
                        char  *token);

         /* GetStorageKind provides information about the storage of the tree.
            A valid (i.e. non initial) rst being passed,
              token is set
                - to "pri" if the tree resides in main memory
                - to "LRU" if the tree resides in secondary memory and is
                  buffered in LRU memory
                - to "sec" if the tree resides unbuffered in secondary memory,
              and TRUE is returned.
            Otherwise,
              token is set to "", an error message is emitted to stderr,
              and FALSE is returned. */


boolean  TryGetStorageKind(t_RT  rst,
                           char  *token);

         /* TryGetStorageKind works like GetStorageKind, but does not emit
            error messages. */


boolean  ExamRSTDescFile(const char  *name);

         /* The functions CreateRST and SaveRST create a description file
            named <name>.Desc (name: see CreateRST). Each CloseRST appends
            the fragment of the description which may have changed during the
            open phase.
            Except for the first few bytes the file consists of ASCII
            characters. Thus read with an ordinary editor, the first few
            lines will be unreadable.
            
            ExamRSTDescFile interprets the complete description file and
            writes the result to stdout.
            For further information please read the description file. */


/*** ----------------------- update operations ------------------------- */

/** --- Resetting the unique flag: --- **/

boolean  SetUnique(t_RT     rst,
                   boolean  mode);

         /* The unique state, defined during tree creation, may be reset
            by this function. The unique flag is set without internal checks
            (even to TRUE). See also CreateRST. */


/** --- Insertion of records: --- **/

boolean  InsertRecord(t_RT               rst,
                      const typinterval  *rectangle,
                      refinfo            info,
                      boolean            *inserted,
                      refinfo            infoStored);

         /* InsertRecord inserts a new record in the tree. Its behavior
            depends on the setting of the unique flag.
            If the unique flag is set TRUE (see CreateRST), a new record is
            not inserted if a record with the same rectangle is found.
            
            rectangle:
            passes the rectangle part of the new record.
            
            info:
            passes the information part of the new record.
            
            inserted:
            - yields TRUE if the record could be inserted
            - yields FALSE if the insertion in a unique tree was refused.
            
            infoStored:
            is the address of a variable to be provided. If inserted yields
            FALSE, the variable will contain the information part of the
            stored record which caused the insertion to be refused.
            
            If the function returns FALSE, an error occurred. In this case the
            record possibly was not inserted although the inserted flag yields
            TRUE. */


/** --- Deletion of records: --- **/

typedef boolean (*InfoCmpFunc) (t_RT            /* rst */,
                                const typinfo*  /* infoStored */,
                                Rint            /* infoSize */,
                                const typinfo*  /* infoSearched */,
                                void*           /* delRefAny */);

        /* rst:
           contains the tree identifier passed to the function DeleteRecord.
           
           infoStored:
           contains a pointer to the information part stored with the record
           just encountered.
           
           infoSize:
           contains the size (in bytes) of the stored information part.
           
           infoSearched:
           contains a pointer to the information part passed to the function
           DeleteRecord by info.
           
           delRefAny:
           contains the parameter delRefAny passed to the function
           DeleteRecord. */


boolean  DeleteRecord(t_RT               rst,
                      const typinterval  *rectangle,
                      const typinfo      *info,
                      InfoCmpFunc        Qualified,
                      void               *delRefAny,
                      boolean            *deleted);

         /* DeleteRecord is responsible for deleting a record from the tree.
            Its behavior depends on the setting of info and the result of a
            comparison function of type InfoCmpFunc (see above) respectively.
            DeleteRecord will never delete more than a single record.
            
            rectangle:
            must contain the rectangle part of the record to be deleted.
            
            info:
            if info == NULL:
              DeleteRecord deletes the first record with the given rectangle
              it finds. This provides a fast deletion in trees where entries
              are unique and may be used in trees where entries are not
              unique, to iteratively delete all entries with the same
              rectangle as passed.
            if info != NULL:
              info should contain the information part of the record to be
              deleted. In this case a record is only deleted if the comparison
              function Qualified returns TRUE. This should mean that the
              record contains both, the right rectangle and the right
              information part.
            
            Qualified:
            is a function of type InfoCmpFunc (see above) to be provided.
            If Qualified returns TRUE, the record just encountered will be
            deleted, and DeleteRecord will return.
            If Qualified returns FALSE, the next record with the same
            rectangle (if present) will be checked.
            
            delRefAny:
            is an arbitrary reference passed through to the function
            Qualified.

            deleted:
            - yields TRUE if a record to be deleted was found (and deleted)
            - yields FALSE if a record with the given attribute(s) could not
              be found.
            
            If the function returns FALSE, an error occurred. In this case the
            record possibly was not deleted although the deleted flag yields
            TRUE. */


/** --- Shrinking storage size to its minimum: --- **/

#define FileReductionFactor M_SQRT2


boolean  ReorganizeMedium(t_RT     rst,
                          boolean  shortenAnyway,
                          boolean  *reorganized,
                          boolean  *shortened);

         /* If records are deleted from the tree, the files (RAM disks)
            holding the directory and data pages will not shrink. Free
            pages are reoccupied by following insertions.
            ReorganizeMedium implements a reorganization of the media.
            Files are shortened to their minimum possible size.
            
            The RAM disks of main memory residing trees are not shortened,
            but are reorganized such that a following SaveRST will lead to
            files of the minimum possible size.
            
            shortenAnyway:
            If shortenAnyway is set to TRUE, the files are shortened if their
            length is greater than necessary, otherwise the files are only
            shortened if their length is at least FileReductionFactor times
            greater than needed.
            
            reorganized:
            is set to TRUE if a reorganization was performed on at least one
            of the files or RAM disks respectively.
            
            shortened:
            is set to TRUE if at least one of the files was attempted to
            truncate. If the function returns TRUE, the concerning files were
            successfully truncated. */


/*** ------------------------ standard queries ------------------------- */

/*** ---------- function types needed by the standard queries ----------- */

/* typedef boolean (*QueryFunc) (this type is declared in
                                 RSTQueryFuncType.h,
                                 see there); */


typedef void (*QueryManageFunc) (t_RT                /* rst */,
                                 Rint                /* numbOfDim */,
                                 const typinterval*  /* rectangle */,
                                 refinfo             /* info */,
                                 Rint                /* infoSize */,
                                 void*               /* manageRefAny */,
                                 boolean*            /* modify */,
                                 boolean*            /* finish */);

        /* rst:
           contains the tree identifier passed to the function RegionQuery.
           
           numbOfDim:
           contains the number of dimensions of the considered tree.
           
           rectangle:
           contains the data rectangle currently matching the query
           condition.
           
           info:
           points to the associated information part.
           
           infoSize:
           contains the size of the information part in bytes.
           
           manageRefAny:
           contains the parameter manageRefAny passed to the function
           RegionQuery.
           
           modify:
           points to a flag labeling the information part to be written back
           when the function is left. See also RegionQuery.
           
           finish:
           points to a flag labeling the query to be finished when the
           function is left. */


/*** ------------------ standard query functions ----------------------- */

boolean  ExistsRegion(t_RT               rst,
                      const typinterval  *queryRects,
                      Rint               numbOfQueryRects,
                      void               *queryRefAny,
                      QueryFunc          DirQuery,
                      QueryFunc          DataQuery,
                      boolean            *recordfound);

         /* ExistsRegion performs a RegionQuery on the tree referenced by rst.
            It stops after the first record satisfying the query condition is
            found. Parameter description: See RegionQuery.
            
            recordfound:
            is set to TRUE if a record satisfying the query condition
            exists, otherwise FALSE. */


boolean  RegionQuery(t_RT               rst,
                     const typinterval  *queryRects,
                     Rint               numbOfQueryRects,
                     void               *queryRefAny,
                     QueryFunc          DirQuery,
                     QueryFunc          DataQuery,
                     void               *manageRefAny,
                     QueryManageFunc    Manage);

         /* RegionQuery performs a RegionQuery on the tree referenced by rst.
            An array of query rectangles may be passed by queryRects.
            Two different functions have two be provided (DirQuery, DataQuery)
            which perform the query in the directory and the data level
            respectively. A third function (Manage) must be provided to deal
            with the records successively found.
            A query is closed either if it did not find an additional
            record satisfying the query condition or if the finish flag was
            set by the function Manage. See also QueryFunc, QueryManageFunc.
            
            queryRects:
            Passed through to the comparison functions DirQuery and
            DataQuery.
            Query rectangles to be compared by DirQuery/DataQuery with the
            directory- and data rectangles encountered during the query.
            
            numbOfQueryRects:
            Passed through to the comparison functions DirQuery and
            DataQuery.
            Number of query rectangles actually passed.
            
            queryRefAny:
            Arbitrary reference passed through to the comparison functions
            DirQuery and DataQuery.
            
            DirQuery, DataQuery:
            Function parameters, passing comparison functions of type
            QueryFunc, returning a boolean. See RSTQueryFuncType.h.
            
            manageRefAny:
            Arbitrary reference passed through to the function Manage.
            
            Manage:
            Function parameter passing a management function.
            Manage is called each time a new data rectangle satisfying the
            query condition is found.
            Examples of tasks of the management function:
            Inspection of the data record's rectangle and info part.
            Communication to another structure via manageRefAny.
            Modification of the info part (the rectangle cannot be modified).
            Finishing the query. */


boolean  AllQuery(t_RT             rst,
                  void             *manageRefAny,
                  QueryManageFunc  Manage);

         /* AllQuery performs a fast query which returns all records stored
            in the tree referenced by rst. See also RegionQuery.
            
            manageRefAny, Manage:
            See RegionQuery. */


boolean  RegionCount(t_RT               rst,
                     const typinterval  *queryRects,
                     Rint               numbOfQueryRects,
                     void*              queryRefAny,
                     QueryFunc          DirQuery,
                     QueryFunc          DataQuery,
                     Rlint              *recordcount);

         /* RegionCount performs a RegionQuery on the tree referenced by rst.
            It does not return records but only counts the number of records
            found. See also RegionQuery.
            
            recordcount:
            is set to the number of records satisfying the query
            condition. */


/*** ---------------------------- joins -------------------------------- */

/*** --------------- function types needed by the joins ---------------- */

typedef boolean (*JoinFunc) (t_RT                /* rst1 */,
                             t_RT                /* rst2 */,
                             Rint                /* numbOfDim */,
                             const typinterval*  /* rst1Rect */,
                             const typinterval*  /* rst2Rect */);

        /* rst1, rst2:
           contain the tree identifiers passed to the corresponding Join
           function.
           
           numbOfDim:
           contains the dimensionality of the trees, as defined at their
           creation.
           
           rst1Rect:
           contains the directory/data rectangle of the first tree (rst1)
           currently considered.

           rst2Rect:
           contains the directory/data rectangle of the second tree (rst2)
           currently considered. */


typedef void (*JoinManageFunc) (t_RT                /* rst1 */,
                                t_RT                /* rst2 */,
                                Rint                /* numbOfDim */,
                                const typinterval*  /* rectangle1 */,
                                const typinterval*  /* rectangle2 */,
                                refinfo             /* info1 */,
                                refinfo             /* info2 */,
                                Rint                /* info1Size */,
                                Rint                /* info2Size */,
                                void*               /* manageRefAny */,
                                boolean*            /* finish */);

        /* rst1, rst2:
           contain the tree identifiers passed to the join functions.
           
           numbOfDim:
           contains the number of dimensions of the joined trees.
           
           rectangle1, rectangle2:
           contain the two rectangles currently matching the join condition.
           
           info1, info2:
           point to the associated information parts.
           
           info1Size, info2Size:
           contain the sizes of the information parts.
           
           manageRefAny:
           contains the parameter manageRefAny passed to the join functions.
           
           finish:
           points to a flag labeling the join to be finished when the
           function is left. */


/*** ------------------------ join functions---------------------------- */

boolean SpJoin(t_RT            rst1,
               t_RT            rst2,
               void            *manageRefAny,
               JoinManageFunc  Manage);

         /* SpJoin performs a spatial join(*) on the two trees referenced
            by rst1 and rst2.
            It is optimized in that it recursively restricts the search space
            to the intersection of the MBBs of the sub-trees.
            See also publication:
            "Efficient Processing of Spatial Joins Using R-trees"
            [T. Brinkhoff, H. Kriegel, B. Seeger; 1993].
            It does not offer any additional facilities.
            It works considerably faster than XSpJoin if two complete trees
            have to be joined.
            Remark:
            The number of internal rectangle comparisons may be obtained
            with function GetCountRectComp. See also XSpJoin and XJoin.
            
            manageRefAny, Manage: see XJoin.
            
            *) A spatial join is a geometrical join which finds all objects
               that have a common intersection. */


boolean  XSpJoin(t_RT               rst1,
                 const typinterval  *rst1FilterRects,
                 Rint               rst1numbOfFilterRects,
                 void               *rst1FilterRefAny,
                 QueryFunc          Dir1Filter,
                 QueryFunc          Data1Filter,
                 t_RT               rst2,
                 const typinterval  *rst2FilterRects,
                 Rint               rst2numbOfFilterRects,
                 void               *rst2FilterRefAny,
                 QueryFunc          Dir2Filter,
                 QueryFunc          Data2Filter,
                 /*                 DirJoin cannot be set by the user */
                 JoinFunc           DataJoin,
                 void               *manageRefAny,
                 JoinManageFunc     Manage);

         /* XSpJoin performs a partly generalized spatial join on the two
            trees referenced by rst1 and rst2.
            It is optimized in that it restricts the search space to the
            intersection of the trees.
            Hence it works considerably faster than XJoin.
            Regarding SpJoin, XSpJoin offers two additional features:
            The join may be restricted on parts of the two trees by
            comparison functions.
            The join condition may be restricted to something, more
            restrictive than intersection (see below).
            Remark:
            The number of internal rectangle comparisons may be obtained
            with function GetCountRectComp. See also XJoin.
            
            (DirJoin:)
            is fixed to an internal function of type boolean, which returns
            TRUE if two directory rectangles, one of rst1, the other of rst2
            have a common intersection, otherwise FALSE.
            
            DataJoin:
            has to determine whether two data rectangles, one of rst1, the
            other of rst2, satisfy the join condition.
            The join condition must include that the rectangles intersect
            each other. In other words, DataJoin must yield FALSE if the
            rectangles do not intersect.
            
            The other parameters have the same meaning as in XJoin, see
            there. */


boolean  XJoin(t_RT               rst1,
               const typinterval  *rst1FilterRects,
               Rint               rst1numbOfFilterRects,
               void               *rst1FilterRefAny,
               QueryFunc          Dir1Filter,
               QueryFunc          Data1Filter,
               t_RT               rst2,
               const typinterval  *rst2FilterRects,
               Rint               rst2numbOfFilterRects,
               void               *rst2FilterRefAny,
               QueryFunc          Dir2Filter,
               QueryFunc          Data2Filter,
               JoinFunc           DirJoin,
               JoinFunc           DataJoin,
               void               *manageRefAny,
               JoinManageFunc     Manage);

         /* The functionality of XJoin can be considered as follows:
            On each of the two trees to be joined an in-line pre-query is
            performed. On the resulting restricted sets of records of the two
            trees the join is applied, depending on the given join condition.
            XJoin performs a join on the two trees referenced by rst1 and
            rst2. A join is closed either if it did not find an additional
            pair of records satisfying the join condition or if the finish
            flag was set by the function Manage.
            See also QueryFunc, JoinFunc, JoinManageFunc.
            
            rst1FilterRects:
            Used in connection with the pre-query on rst1.
            Passed through to the comparison functions Dir1Filter and
            Data1Filter.
            Query rectangles to be compared by Dir1Filter/Data1Filter with
            the directory- and data rectangles encountered during the join.
            
            rst1numbOfFilterRects:
            Number of rectangles actually passed by rst1FilterRects[].
            Passed through to the functions Dir1Filter and Data1Filter.
            
            rst1FilterRefAny:
            Arbitrary reference passed through to the functions Dir1Filter
            and Data1Filter.
            
            Dir1Filter, Data1Filter:
            Function parameters passing comparison functions of type
            boolean.
            The two functions have to perform the pre-query on rst1.   
            See also QueryFunc and RegionQuery.
            
            rst2FilterRects:
            Used in connection with the pre-query on rst2.
            Analogous to rst1FilterRects[].
            
            rst2numbOfFilterRects:
            Used in connection with the pre-query on rst2.
            Analogous to rst1numbOfFilterRects.
            
            rst2FilterRefAny:
            Used in connection with the pre-query on rst2.
            Analogous to rst1FilterRefAny.
            
            Dir2Filter, Data2Filter:
            Used in connection with the pre-query on rst2.
            Analogous to Dir1Filter, Data1Filter.
            
            DirJoin, DataJoin:
            Function parameters passing comparison functions of type
            boolean.
            DirJoin has to determine whether two directory rectangles, one
            of rst1, the other of rst2, satisfy the join condition.
            DataJoin has to determine whether two data rectangles, one of
            rst1, the other of rst2, satisfy the join condition.
            
            manageRefAny:
            Arbitrary reference passed through to the function Manage.
            
            Manage:
            Function parameter passing a management function.
            Manage is called each time a new pair of data rectangles
            satisfying the join condition is found.
            Typical tasks of the management function in joins:
            Inspection of the data records' rectangles and info parts.
            Communication to another structure via manageRefAny.
            Finishing the join. */


boolean SpJoinCount(t_RT   rst1,
                    t_RT   rst2,
                    Rlint  *paircount);

         /* Like SpJoin, SpJoinCount performs a spatial join on the two trees
            referenced by rst1 and rst2. It does not return record pairs, but
            only counts the number of record pairs found. See also SpJoin.
            
            paircount:
            is set to the number of record pairs whose rectangles
            intersect. */


boolean  XSpJoinCount(t_RT               rst1,
                      const typinterval  *rst1FilterRects,
                      Rint               rst1numbOfFilterRects,
                      void               *rst1FilterRefAny,
                      QueryFunc          Dir1Filter,
                      QueryFunc          Data1Filter,
                      t_RT               rst2,
                      const typinterval  *rst2FilterRects,
                      Rint               rst2numbOfFilterRects,
                      void               *rst2FilterRefAny,
                      QueryFunc          Dir2Filter,
                      QueryFunc          Data2Filter,
                      /*                 DirJoin cannot be set by the user */
                      JoinFunc           DataJoin,
                      Rlint              *paircount);
                          
         /* Like XSpJoin, XSpJoinCount performs a partly generalized spatial
            join on the two trees referenced by rst1 and rst2. It does not
            return record pairs but only counts the number of record pairs
            found. See also XSpJoin.
            
            paircount:
            is set to the number of record pairs satisfying the join
            condition. */


boolean  XJoinCount(t_RT               rst1,
                    const typinterval  *rst1FilterRects,
                    Rint               rst1numbOfFilterRects,
                    void               *rst1FilterRefAny,
                    QueryFunc          Dir1Filter,
                    QueryFunc          Data1Filter,
                    t_RT               rst2,
                    const typinterval  *rst2FilterRects,
                    Rint               rst2numbOfFilterRects,
                    void               *rst2FilterRefAny,
                    QueryFunc          Dir2Filter,
                    QueryFunc          Data2Filter,
                    JoinFunc           DirJoin,
                    JoinFunc           DataJoin,
                    Rlint              *paircount);

         /* Like XJoin, XJoinCount performs a join on the two trees
            referenced by rst1 and rst2. It does not return record pairs but
            only counts the number of record pairs found. See also XJoin.
            
            paircount:
            is set to the number of record pairs satisfying the join
            condition. */


/*   Joins may result in a vast number of record pairs ... The following
     constant controls how entertaining the ...JoinCount functions work. */

#define VerboseJoinCount TRUE

/*   The messages go to stdout, and come logarithmically stepped. */


/*** ------------------ performance control routines ------------------- */

/*** ------ Counts-Switch: */

boolean  CountsOn0(t_RT  rst);
         /* switch ON, set 0 */

boolean  CountsOn(t_RT  rst);
         /* switch ON */

boolean  CountsOff(t_RT  rst);
         /* switch OFF */
         
         /* NOTES:
            The Counts-Switch applies to the complete counting facility.
            Count results are obtained by the GetCount...  functions below.
            
            Functions that create/open/load a tree do not count for their
            initial IO operations, and finally initialize counting as with
            the sequence CountsOn0, CountsOff. */


boolean  GetCountRead(t_RT   rst,
                      Rlint  *dirDemandCount,
                      Rlint  *dataDemandCount,
                      Rlint  *dirGetCount,
                      Rlint  *dataGetCount,
                      Rlint  *dirReadCount,
                      Rlint  *dataReadCount);

         /* RSTree keeps a minimal INTERNAL buffer, consisting of one single
            path(*). This path buffer is a necessary part of any R-tree
            implementation, and normally never cleared. See also ClearPath().
            But NOTE:
            For RAM disk as for LRU buffered operation, this buffer is
            virtual, as it is implemented as a path of pointers into the
            anyway existing main memory storage.
            
            Reads, Writes:
              A Read is counted whenever a page is read from secondary memory
              (a file, being part of the stored tree) in conjunction with a
              query- or update-operation. A Write is counted whenever a page
              is written to secondary memory (a file, being part the stored
              tree) in conjunction with a query- or update-operation.
            Gets, Puts:
              Gets and Puts mirror the Reads and Writes, arising for working
              on secondary memory, but are counted independently of the
              storage kind.
              A Get is counted whenever a page is (would be) read into the
              internal path. A Put is counted whenever a page is (would be)
              written back from the internal path.
              However, while an exact mapping Reads --> Gets can be achieved
              easily, a totally exact mapping Writes --> Puts would be rather
              complex, especially for insertions. The reason is: When entries
              are inserted consecutively to secondary memory, the algorithm
              may (depending on the data) be able to save writes by exploiting
              the internal path, which would have to be simulated, working on
              LRU buffer or RAM disk.
              Since counting should not measurably impact performance, the
              implementation allows a very small inaccuracy when Puts are
              concerned. The following table depicts the theoretically
              possible maximum differences of the counted Puts in comparison
              to the Writes, counted when working on secondary memory.
                                   |  secondary memory  |   LRU and RAM
              Insertions: ----------------------------------------------
                        data Puts  |         0          |        +1
                   directory Puts  |         0          |  +(height - 2)
              Deletions: -----------------------------------------------
                        data Puts  |         0          |         0
                   directory Puts  |         0          |         0
              In practice, the difference will be frequently 0 for data Puts,
              and very rarely exceed 2 for directory Puts. Differences may
              also dissolve when additional insertions are performed.
            Demands:
              A Demand is counted whenever the running query- or update-
              operation (newly) demands a page in a certain level of the
              internal path. Demands are counted independently of the fact,
              that the page might already be there. A Demand does not cause
              further action if the correct page is already there; it causes a
              Get if another page is there which is unmodified; it causes a
              Put followed by a Get if another page is there which has been
              modified.
              Hence, for operations visiting a certain page at most once, the
              number of Demands is equal to the number of Gets (Reads in case
              of unbuffered working on secondary memory), which would arise if
              the internal path had been cleared before the operation.
              This equivalence holds e.g. for the standard queries.
              
            dirDemandCount, dataDemandCount:
            are set to the number of directory- and data-page Demands
            respectively.
            
            dirGetCount, dataGetCount:
            are set to the number of directory- and data-page Gets
            respectively.
            
            dirReadCount, dataReadCount:
            are set to the number of directory- and data-page Reads
            respectively.
            
            If the function returns FALSE, these variables are set to 0. 
            
            *) There are two operations for which a second path is used:
            - Deletion, if it is done by reinsertion.
            - Reorganization of sparse tree files, when gaps were caused
              by deletion. */


boolean  GetCountWrite(t_RT   rst,
                       Rlint  *dirPutCount,
                       Rlint  *dataPutCount,
                       Rlint  *dirWriteCount,
                       Rlint  *dataWriteCount);

         /* See GetCountRead for introductive information.
            
            dirPutCount, dataPutCount:
            are set to the number of directory- and data-page Puts
            respectively.
            
            dirWriteCount, dataWriteCount:
            are set to the number of directory- and data-page Writes
            respectively.
            
            If the function returns FALSE, these variables are set to 0. */


boolean  GetCountRectComp(t_RT   rst,
                          Rlint  *dirCompCount,
                          Rlint  *dataCompCount);

         /* dirCompCount is set to the number of directory rectangle
            comparisons.
            dataCompCount is set to the number of data rectangle comparisons.
            
            If the function returns FALSE, these variables are set to 0.
            
            NOTE that dirCompCount, dataCompCount are counted only for the
            spatial join functions. The count applies whenever an internal
            rectangle comparison function is called.
            (Except for the spatial join, all queries/joins apply user
            implemented comparison functions.) */


boolean  GetCountPriorQ(t_RT   rst,
                        Rint   *PriorQlen,
                        Rint   *PriorQmax,
                        Rlint  *PriorQelems);

         /* GetCountPriorQ provides information on the priority queue of the
            distance query. Length here refers to the number of elements.
            
            PriorQlen:
            contains the current length of the priority queue during a single
            distance query. It is initialized by NewDistQuery and is updated
            at every call of GetDistQueryRec (independently of the state of
            the Counts-Switch).
            
            PriorQmax:
            contains the maximum length of the priority queue during a single
            distance query. It is initialized by NewDistQuery and is updated
            at every call of GetDistQueryRec (independently of the state of
            the Counts-Switch).
            
            PriorQelems:
            accumulates the maximum priority queue lengths of all distance
            queries during a count phase. It is incremented at every call of
            DisposeDistQuery by PriorQmax. This happens independently of the
            state of the Counts-Switch. Hence PriorQelems always refers to
            the period since the previous call of CountsOn0.
            
            If the function returns FALSE, the variables are set to 0. */


boolean  ClearPath(t_RT  rst);

         /* Clears the main memory residing path of pages (except for the
            root). The function may be used to achieve an initial state in
            advance of operations whose performance shall be measured. But
            NOTE that in case of e.g. standard queries the count of Demands
            will do the job. */


/*** ---------------- counting of administrational IOs ----------------- */

/*   NOTE that the following functions concerning administrational IOs should
     not be considered as a part of the performance monitoring routines.
     They are only here for sake of completeness. Administrational IOs mainly
     arise due to a special feature of this implementation, which stores
     administrational information in pages, maintained during normal
     operation. An alternative approach would read this information once when
     the tree is opened and write it back when the tree is closed. */

boolean  GetCountAdminRead(t_RT   rst,
                           Rint   *adminPageSize,
                           Rlint  *dirAdminReadCount,
                           Rlint  *dataAdminReadCount);

         /* Some operations lead to (very few) administrational reads of
            pages, e.g. from the free memory list.
            
            adminPageSize:
            is set to the size of an administrational page. The size of
            administrational pages is a constant.
            
            dirAdminReadCount:
            is set to the number of administrational Reads concerning the
            directory level.
            
            dataAdminReadCount:
            is set to the number of administrational Reads concerning the
            data level. */


boolean  GetCountAdminWrite(t_RT   rst,
                            Rint   *adminPageSize,
                            Rlint  *dirAdminWriteCount,
                            Rlint  *dataAdminWriteCount);

         /* Some operations lead to (very few) administrational writes of
            pages, e.g. to the free memory list.
            
            adminPageSize:
            is set to the size of an administrational page. The size of
            administrational pages is a constant.
            
            dirAdminWriteCount:
            is set to the number of administrational Writes concerning the
            directory level.
            
            dataAdminWriteCount:
            is set to the number of administrational Writes concerning the
            data level. */


/*** *********************************************************************/

/* The structure of directory- and data-pages
   ++++++++++++++++++++++++++++++++++++++++++
   
   For the following section the original standard type setting of the release
   is assumed. Then the size of all types mentioned below is 8 bytes, apart
   from two exceptions:
   sizeof(Rpnint) = 4,
   sizeof(typinfo) >= 4, chosen by the user.
   See also RSTTypes.h and RSTStdTypes.h.
   
   Naming conventions:
   -------------------
   pageSize: size of a page,
   numbOfDim: number of dimensions (dimensionality),
   dir: directory levels,
   data: data level.
   
   Definitions:
   ------------
   Align(v, a) is a function, returning the smallest possible integer i >= v,
   i mod a = 0.
   
   
   Net page sizes
   ==============
   Both, directory- and data-pages contain a small administrational part,
   reserved for internals; the remainder is usable for entries.
   The net page size (NPS), i.e. the size of the fraction of a page usable
   for entries, is computed as follows:
   
   R-tree, R*-tree, Hilbert R-tree:
   --------------------------------
   NPS = pageSize - sizeof(typatomkey).
   
   RR*-tree:
   ---------
   NPS = pageSize - (numbOfDim + 1) * sizeof(typatomkey).
   
   
   Entry sizes
   ===========
   The size of an entry (ES) is computed as follows:
   
   R-tree, R*-tree, RR*-tree:
   --------------------------
   ES(dir) =  Align(numbOfDim * 2 * sizeof(typatomkey) + sizeof(Rpnint), 8),
   ES(data) = Align(numbOfDim * 2 * sizeof(typatomkey) + sizeof(typinfo), 8).
   
   Hilbert R-tree:
   ---------------
   ES(dir) =  Align(numbOfDim * 2 * sizeof(typatomkey) + sizeof(Rulint) +
   sizeof(Rpnint), 8),
   ES(data) = Align(numbOfDim * 2 * sizeof(typatomkey) + sizeof(Rulint) +
   sizeof(typinfo), 8).
   
   
   Capacity Examples
   =================
   The assumptions
     pageSize = 4096,
     numbOfDim = 3,
     sizeof(typinfo) = 4,
   lead to the following results for a data page of the different index
   structures:
   
     R-tree, R*-tree:
     ----------------
     NPS = 4096 - 8 = 4088,
     ES(data) = Align(3 * 2 * 8 + 4, 8) = Align(52, 8) = 56,
     Capacity = 4088 / 56 = 73.
     
     RR*-tree:
     ---------
     NPS = 4096 - (3 + 1) * 8 = 4064,
     ES(data) = Align(3 * 2 * 8 + 4, 8) = Align(52, 8) = 56,
     Capacity = 4064 / 56 = 72.
     
     Hilbert R-tree:
     ---------------
     NPS = 4096 - 8 = 4088,
     ES(data) = Align(3 * 2 * 8 + 8 + 4, 8) = Align(60, 8) = 64,
     Capacity = 4088 / 64 = 63.
   
   
   Minimum of the maximum number of entries per page
   =================================================
   The capacity M of a page is determined by the page size, as previously
   explained.
   The smallest possible DIRECTORY page is a page where M = 3,
   the smallest possible DATA page is a page where M = 1.
   
   
   Minimum number of entries per page
   ==================================
   The minimum page occupation m is introduced by m%, a percentage of M, and
   can take values in the following range:
   DIRECTORY levels:
   R-tree, R*-tree, RR*-tree:  2 = min(m) <= m <= max(m) = ceiling(M/2),
   Hilbert R-tree:             min(m) = max(m) = m = ceiling(M/2),
   DATA level:
   R-tree, R*-tree, RR*-tree:  1 = min(m) <= m <= max(m) = ceiling(M/2),
   Hilbert R-tree:             min(m) = max(m) = m = ceiling(M/2).
   
   The parameter m%
   ================
   The percental minimum page occupation m% is a crucial parameter, whose
   appropriate setting effects performance. For each of the different R-tree
   variants, m% is defined as an individual constant. The following table
   gives an overview.
   
                     |     m%
   -------------------------------
   Quadratic R-tree  |     15
   R*-tree           |     30
   RR*-tree          |     20
   Hilbert R-tree    |    (50)
   
   The parameter m is calculated from m% by rounding to the nearest integer.
   NOTE: For small M these settings will be overridden by min(m).
*/

/*** *********************************************************************/

/* LACKS:

   The implementation does not provide packing. Thus depending on the
   alignment of the machine and the compiler, the definition of typatomkey,
   and the choice of typinfo, pages may have gaps, i.e. the fanout may be
   smaller than expected.
   See also "The structure of directory- and data-pages".
   Information about important parameters in this context may be obtained
   by calling InquireRSTDesc.
   
   Tree identifiers are currently only checked for NULL and non NULL. Thus
   passing a non NULL invalid tree identifier is not detected as an error.
   
   Opening a tree multiple is possible, and queries may be performed
   concurrently. Concurrent update operations however will damage
   consistency. But the implementation does neither detect nor inhibit this
   mistake.
   Btw, passing the same tree identifier twice to the join functions is save:
   If a join is performed on one and the same tree, an additional reference of
   it is created internally.
   
   Queries and update operations have to be treated as atomic actions.
   The mistake however of calling one of these operations from inside of
   another (e.g. from a query management function) is not detected as an
   error.
   
   The implementation restricts the informational part of a data record to
   contain at least an Rpnint sized content.

*/
/* KNOWN BUGS:
*/

/*** *********************************************************************/
/*** ------------------------- consistency check ----------------------- */

boolean  CheckConsistency(t_RT         rst,
                          boolean      *consistent,
                          boolean      *rootMBBok,
                          boolean      *otherMBBsOk,
                          typinterval  *storedRootMBB,
                          typinterval  *rootsMBB,
                          Rint         entryNumberPath[],
                          Rint         *parentLevel,
                          typinterval  *storedMBB,
                          Rpnint       *parentPageNr,
                          Rint         *childLevel,
                          typinterval  *childsMBB,
                          Rpnint       *childPageNr,
                          Rpnint       pagesPerLevel[]);

         /* CheckConsistency first checks whether the stored root MBB
            correctly represents the root and sets rootMBBok accordingly.
            Then, recursively tracing the tree, CheckConsistency checks
            whether all other MBBs correctly represent their child pages.
            If it finds an incorrect MBB, it breaks at the first such and sets
            otherMBBsOk to FALSE.
            
            consistent:
            contains TRUE if all is correct, otherwise FALSE.
            
            rootMBBok:
            contains TRUE if the stored root MBB correctly represents the
            root, otherwise FALSE.
            
            otherMBBsOk:
            contains TRUE if all MBBs correctly represent their child pages,
            otherwise FALSE.
            
            storedRootMBB:
            contains the root MBB stored.
            
            rootsMBB:
            contains the root MBB calculated from the root entries.
            
            entryNumberPath[]:
            for each level i (i = parentLevel .. root level) contains the
            index (0 .. maxDirFanout - 1) of the entry pointing to the child
            page (see maxDirFanout in InquireRSTDesc).
            
            parentLevel:
            contains the level (1 .. root level) where the error was
            encountered. If all is correct, parentLevel is set to 1.
            
            storedMBB:
            is the inconsistent MBB in the parent page.
            
            parentPageNr:
            is the page number of the parent page.
            
            childLevel:
            contains the level (0 .. rootLevel - 1) above which the error was
            encountered. If all is correct, childLevel is set to 0.
            
            childsMBB:
            is the MBB calculated from the child page.
            
            childPageNr:
            is the page number of the child page.
            
            pagesPerLevel[i]:
            (i = 0 .. root level) represents the part of the tree
            (number of pages per level) already traced
            (see InquireRSTDesc). */


/*** ------------------------------- dumps ----------------------------- */

boolean  ASCIIdump(t_RT  rst,
                   FILE  *stream);

         /* Recursively prints all entries of the tree. */


typedef void (*RectConvFunc) (t_RT                /* rst */,
                              Rint                /* numbOfDim */,
                              const typinterval*  /* rectangle */,
                              void*               /* bufAdr */,
                              Rint                /* bufSize */);

        /* Rectangle format conversion function which is called by
           DirLevelDump, in order to store the dumped rectangles in the
           desired format.
           
           rst:
           contains the tree identifier passed to the function DirLevelDump.
           
           numbOfDim:
           is the number of dimensions defined when the tree was created.
           
           rectangle:
           is the current rectangle (tree format) to be dumped.
           
           bufAdr:
           is the address of a buffer for the converted representation of a
           rectangle. It is passed to (and passed through by) the function
           DirLevelDump.
           
           bufSize:
           is the size of the provided buffer. It is passed to (and passed
           through by) the function DirLevelDump. */


boolean  DirLevelDump(t_RT          rst,
                      void          *bufAdr,
                      Rint          bufSize,
                      RectConvFunc  Convert);

         /* Dumps the directory rectangles of a tree level by level in
            separate files named <tree name>.lv<#level>.
            
            bufAdr:
            is the address of a buffer to be provided, big enough to store
            the converted representation of a rectangle.
            
            bufSize:
            is the size of the provided buffer.
            
            Convert:
            Function parameter passing a conversion function of type
            RectConvFunc. See there. */


boolean  PathsDump(t_RT  rst);

         /* This implementation normally maintains a single path of page
            references (nodes) in main memory:
            Path "L", page references: "L.N".
            Nodes are members of the path of pages, stored in main memory,
            and depending on the storage kind, they are either pointers to
            allocated blocks, storing copies of secondary memory pages, or
            pointers to LRU buffer pages, or pointers to RAM pages.
            In two special situations a second path of nodes is maintained:
            1. Deletions, if they are performed by reinsertion:
               Path "LDel", page references: "LDel.N".
            2. Media reorganization when gaps were caused by deletions:
               Path "L1", page references: "L1.N".
            
            Additionaly, for the different levels of the tree, several data
            have to be maintained. They are generally stored in (struct)
            variables besides the page references, but for media
            reorganization there is yet an extra data path: "LRrg".
            
            The function, from left to right, starting at the root, level by
            level, prints the contents of the paths as follows:
            Level:       HEADING: the level of the column
            L.N:         page reference (node)
            L.P:         page number
            L.E:         number of the referencing entry
            L.Modif:     flag: modified
            L.ReInsert:  flag: has to be re-inserted (R*-tree)
            LDel.N:      page reference (node)
            L1.N:        page reference (node)
            L1.P:        page number
            L1.Modif:    flag: modified
            LRrg.P:      page number (intermediate)
            LRrg.E:      number of the referencing entry (intermediate)
            #pages:      COUNTER: number of pages
            Level:       FOOTING: the level of the column
            
            Remember that levels count bottom up, and level 0 is the data
            level. */


/*** --------------------- private purpose functions ------------------- */

boolean  GetCountOvUndFlw(t_RT   rst,
                          Rlint  *dirOverflow,  Rlint *dataOverflow,
                          Rlint  *dirUnderflow, Rlint *dataUnderflow,
                          Rlint  *dirReInsert,  Rlint *dataReInsert,
                          Rlint  *dirSplit,     Rlint *dataSplit,
                          Rlint  *dirS_Area0,   Rlint *dataS_Area0);

         /* Provides information about private counters concerning insertion
            and deletion. */


boolean  GetCountChsSbtr(t_RT   rst,
                         Rlint  *call,
                         Rlint  *noFit,
                         Rlint  *uniFit,
                         Rlint  *someFit,
                         Rlint  *ovlpEnlOpt,
                         Rlint  *P,
                         Rlint  *maxP,
                         Rlint  *PminusQ,
                         Rlint  *ovlpEnlComput,
                         Rlint  *P1OvlpEnl0,
                         Rlint  *afterwOvlpEnl0,
                         Rlint  *area0);

         /* Provides information about private counters concerning the
            ChooseSubtree algorithm. */


#endif /* __RSTOtherFuncs_h */
