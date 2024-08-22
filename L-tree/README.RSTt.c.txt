README for the test program sources RSTt.c and HRTt.c respectively
==================================================================

------------------------------------------------------------------------------
NEW since RSTree7.2:
./rrst/RSTt.c: is a link now to either RSTtBin.c or RSTtTxt.c, where
  RSTtBin.c serves for reading input files containing binaries.
  RSTtTxt.c serves for reading input files containing text.
  The function RdTextToRect() has to be adapted to the individual format,
  provided in the text files, containing the data to be handled.
./hrt/HRTt.c: Changes of ./rrst/RSTt.c are applied analogously.
------------------------------------------------------------------------------

SYNOPSIS
  tqrt [ -v ]
  thrt [ -v ]
  trrst [ -v ]
  trst [ -v ]
  Test programs for
  - Quadratic R-tree (tqrt)
  - Hilbert-R-tree (thrt)
  - RR*-tree (trrst)
  - R*-tree (trst).
  If the programs are called with "-v", several tables are displayed ahead of
  the short instruction, always preceding the first menu prompt. They provide 
  information about basic constants, types, limits, etc. of the RSTree system.

Naming conventions in this text:
In the following text, always trrst is referred to, but could be replaced by
any of the other programs of the list above.
Some similar applies to RSTt.c, it could be replaced by HRTt.c.
RSTt.c is the source of the test programs trrst, trst and tqrt; HRTt.c is the
source of the test program thrt (see "Compilation" in README).
Preface:
The complete set of functions, provided by the RSTree bundle, is used in
RSTt.c with very few exceptions. Exceptions mainly concern functions from the
header files in ./util. They are in parts not used in RSTt.c, because there
was no need for them in conjunction with tests of the RSTree functions.
Hence RSTt.c can serve for a programmer as a collection of examples for the
application of the functions, provided in the concerning header files.
On the other hand trrst is a simple tool for the reproduction of the tests
in the RR*-tree paper. Additionally all the more complex functions,
implemented in the RSTree bundle, e.g. joins, proximity queries, deletions,
can be tested with it.

Here is a brief instruction for the use of trrst:

The program is run from a terminal (window).
All commands are given by typing a single character, followed by the RETURN
key. CAUTION: inadvertently a string provided, the FIRST CHARACTER is taken!

THIS IS IMPORTANT:
--> The input normally TOGGLES between two different main menus.
The first main menu mainly provides opening and closing facilities. The
corresponding commands are explained in the section "First main menu".
The second main menu mainly provides update and query functions. The
corresponding commands are explained in the section "Second main menu".
Both main menus can be left by typing "-" RETURN; then the other main menu is
displayed.
However, commands may also lead to intermediate menus, or may request user
input before they do their task. After the task is done, there is normally a
TOGGLE to the other main menu, but this sometimes depends on the context.

Since trrst is a test program for programmers, capturing erroneous user input
is weak. The most serious issue: If a command sequence has started, there is
no escape.
However:
- Damage protection for active trees is rather strong.
- Typing an unknown command (<char> RETURN) for one of the main menus does not
  harm (nothing happens). Repeating is possible.
- Erroneous input (e.g. a file does not exist) is trapped and leads back to
  the same main menu as displayed before.
- There are a lot of confirming and error notifying messages. But when a main
  menu is (repeatedly) displayed they may appear before (above) it.
See also short instructions at program start.

Hint: Open a terminal now, call "trrst" and then continue to read.

First main menu
---------------
  From left to right, the first main menu is horizontally divided into three
  groups, concerning the three different storage types:
  Secondary memory, secondary memory + LRU buffer, main memory.
  Commands, applicable to more than one of the groups are horizontally
  located, such that their position accounts for their belonging.
  More detailed:
- Trees can be created in secondary memory (c) (REMOVING an EXISTING TREE with
  the same name), or can be opened (o) and then be closed (.).
- Trees can also be created and opened respectively, using an arbitrary sized
  LRU-buffer, (C) and (O) respectively.
- Trees can be created in main memory (m), can be saved to secondary memory
  then (s), or can be destroyed (killed) (K). Secondary memory residing trees
  can be loaded into main memory (l).
- Secondary memory residing trees (their files) can be removed (R); of course
  this should only be done if the concerning tree is not active.
  Processes can synchronize the tree (its files) they manage (S), and
  processes can synchronize themselves (their buffers) to the underlying
  tree (G).
  For the points above, see also "managing secondary memory residing trees"
  and "managing main memory residing trees" in RSTOtherFuncs.h.
  -----
  THIS IS IMPORTANT:
  --> In trrst the name of a tree is always derived from the inserted data
  file, whose name is the only one actually needed for update operations.
  See "Naming and storing conventions using RSTt.c / trrst" at bottom.
  For query file names the user is prompted when query operations are called.
  -----
  Finally:
- A command provides information about the active tree (I). The description
  file, stored with each secondary memory residing tree, can be read (E). Page
  sizes can be computed (`). The menu can be left (-). The program can be
  quit (q).

Hint: If you just studied the first main menu, type "-" RETURN now and then
continue to read.

Second main menu
----------------
  The first part of the second main menu concerns:
  Insertion, deletion, retrieval test for single entries (as explained under
  "Exact Match Queries" below.
  More detailed:
- Insertions can be performed (i) and deletions can be performed (d); for
  both, the file, whose name was given during tree creation/opening must be
  there; it should contain the corresponding rectangles.
  See also "update operations" in RSTOtherFuncs.h.
  The second part of the second main menu concerns different kinds of queries.
  From left to rightt it is horizontally divided into two groups, concerning
  queries being performed for two different purposes:
  1. for test purpose only, which means, that they indeed do their normal job,
     but only count records, accesses, etc.
  2. for normal database purpose, which means, that they (besides counting)
     actually yield the records found.
- There are two different Exact Match Queries:
  1. ExistsRegion (x), implemented by ExistsRegion from RSTOtherFuncs.h, which
     itself does not provide records; using this command, the user is not
     asked for a query file, but automatically the already known insertion
     file is used; ExistsRegion is less expensive than the second alternative,
     at least concerning the necessary rectangle comparisons.
     -----
     For the second alternative and all other simple queries the name of the
     file, containing the query rectangles, has to be provided.
     -----
  2. ExactMatchCount (e) and ExactMatchQuery (E) respectively, merely counting
     and actually yielding records respectively, are implemented by
     RegionCount and RegionQuery respectively; other then ExistsRegion, they
     do not stop after the first matching record was found.
- Region queries may be performed (see RegionQuery in RSTOtherFuncs.h); they
  are implemented as SIMPLE RECTANGLE INTERSECTION QUERIES in RSTt.c. (i.e.
  not utilizing the full complexity provided); as always, there is a counting
  command (r) which was used in the RR*-tree tests, and a command actually
  providing records ([).
- Different joins may be performed; in this case a second tree has to be
  provided (which may also be the same however); as always, joins are provided
  only counting and really yielding records respectively.
  The merely counting SpatialJoinCount (j) and the records yielding
  SpatialJoin (=) respectively, are implemented by SpJoinCount and SpJoin
  respectively from RSTOtherFuncs.h.
  The merely counting XJoinCount (J) and the records yielding
  XJoin (#) respectively, are implemented by XJoinCount and XJoin respectively
  from RSTOtherFuncs.h.
- Two different commands using the proximity query (see NewDistQuery in
  RSTDistQueryFuncs.h) may be performed;
  1. n-NearestTest (N) demands a RECTANGLE query file (the file type at hand),
     but actually passes the LOW CORNERs of the rectangles; it only counts.
  2. DistanceQuery (D) activates part of the complexity of the distance query;
     it yields records.
  Finally:
- CheckConsistency (C) checks the consistency of the tree. PathsDump (P)
  provides information about the internally stored path. ASCIIdump (A) shows a
  dump of the complete tree. DirLevelDump (L) dumps the rectangles of the
  directory levels of the tree level by level in files with suffixes lv<n>,
  where <n> = level(1 .. RootLevel). MediaReorganization (M) performs a media
  reorganization of the RAM disks and files respectively of the stored trees;
  if deletions were performed, this shrinks the length of the files, storing
  the directory and data level of the tree. The menu can be left (-).
  
  Naming conventions of RSTree
  ----------------------------
  There are 5 files, representing a tree in secondary memory.
  They are named as follows:
  <name>	    main name (from creation)	[directory levels]
  <name>.Data					[data level]
  <name>.DataPD					[data Page Directory (*)]
  <name>.Desc	    description file, updated upon open and close
  <name>.DirPD					[directory Page Directory (*)]
  where the suffixes stem from the RSTree implementation and <name> is freely
  choosable.
   *) contains page numbers to be re-used after deletions
  
  Naming and storing conventions using RSTt.c / trrst
  ---------------------------------------------------
  THIS IS IMPORTANT:
  1. The main name of a tree, i.e. <name> from above, is set to the
     (path-)name of the data file inserted, but suffixed with ".RSF".
     Hence:
     When a tree is created, inserting rectangles from data file "foo", due to
     the naming convention of trrst combined with the internal naming
     conventions of the RSTree implementation, the following 5 new files are
     maintained:
     - "foo.RSF":         directory levels (main tree name)
     - "foo.RSF.Data":    data level
     - "foo.RSF.DataPD":  data Page Directory (*)
     - "foo.RSF.Desc":    description file
     - "foo.RSF.DirPD":   directory Page Directory (*)
     
     This obviously means that the tree files are stored in the same
     directory, where the data files reside. To avoid that, a directory should
     be created for the tests, and the data files (and query files) should be
     linked there. Then trrst should to be called in that directory, so that
     all files are local.
  THIS IS IMPORTANT:
  2. When a tree name is requested by trrst, you must not (never have to) type
     the suffix ".RSF", the "main tree name" (see above) is labeled with.
     Instead, simply type the name of the data file, the tree is/was created
     from. In other words, trrst automatically adds the suffix ".RSF" when
     trees are concerned.
     Example: First step:
              Creating a tree from data file "foo" in main memory, then saving
              it to secondary memory named "foo.RSF".
              Second step:
              Opening the secondary memory residing tree "foo.RSF".
     --------------------
     m RETURN		[creation in main memory]
     foo RETURN
     ...
     i RETURN		[insertion]
     ...
     s RETURN		[saving to secondary memory]
     -----------
     K RETURN		[killing the (still) main memory residing tree]
     o RETURN		[opening the just saved secondary memory residing tree]
     foo RETURN
     ...
     --------------------
     This is a peculiarity of trrst for convenience.
     However, there is a single exception: The option "E" showing a
     description file expects the full name, so that description files of
     arbitrary trees can be viewed.
     
     Btw:
     Directly using the functions of the RSTree implementation, the main name
     of a tree is freely selectable, of course.

