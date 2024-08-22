/* -----  RSTTypes.h  ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTTypes_h
#define __RSTTypes_h


/**:   RSTree - a C frame for R-tree like structures, MAIN INTERFACE
       =============================================================    **/


/**    Implementation:  Norbert Beckmann
              Version:  2.2
                 Date:  12/13                                           **/
                 
/**    Implemented tree:                                                **/

/**:   1D-key R-tree
                                                                        **/
/**:   INTERFACE (types except QueryFunc, distance query types)
       ========================================================         **/

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

/*** -------------------- operating system version --------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <sys/time.h> /* times measurement by getitimer() */
#include <time.h> /* gettimeofday() */

/* This header file concentrates the external includes for the complete
   RSTree system. */

/*** ----------------------- internal includes ------------------------- */
#include "RSTStdTypes.h"

/* NOTE that the file may also provide some quasi standard functions,
   variables and constants, not available on certain platforms
   (see there). */
/*** ------------------------------------------------------------------- */

/*** ----------------------------- basics ------------------------------ */

/* NOTE:
   Except for type char, RSTree exclusively uses its self defined standard
   types. The type char is only used in association with character strings.
   The following definitions of constants, types and functions are included
   from RSTStdTypes.h.
   
   - a boolean type and boolean constants:
         boolean, TRUE, FALSE
   - file identifiers:
         File
   - standard integer types:
         byte, Rshort, Rushort, Rint, Ruint, Rpint, Rlint, Rulint, Rpnint
   - standard floating point types:
         Rfloat
   - a string type for names:
         MaxNameLength, RSTName
   and the declaration of the function
         strans,
         which helps to print the self defined integer types. */


/*** --------------------------- constants ----------------------------- */


/*** ----------------------------- types ------------------------------- */

/* NOTE that calculations of areas, distances, etc. are done with type Rfloat
  (see RSTStdTypes.h). */


typedef Rulint OneDKey; /* SYS_BUILD */

        /* 1D key type */


typedef Rfloat  typatomkey;

        /* SYS_BUILD: typatomkey may be of any type as far as the standard
           comparison operators are applicable. */


typedef struct {
               typatomkey  l, h;
               } typinterval;

        /* A multidimensional interval (rectangle) is the key type the index
           structure handles. Rectangles are passed as arrays of intervals
           [l, h], l <= h.
           Scheme:
           typedef typinterval typRectangle[Number_of_Dimensions]; */


typedef union {
              Rpnint  tag;
              char    c[1];          /* c[] flexible */
              } typinfo, *refinfo;

        /* A typinfo, to be passed with each rectangle, may contain arbitrary
           information associated with a data record. Its size has to be
           defined during tree creation, but is at least sizeof(Rpnint). */


typedef Rfloat  typcoord;

        /* Multidimensional points are passed as arrays of coordinates.
           Scheme:
           typedef typcoord typPoint[Number_of_Dimensions];
           Points are only used in the context of distance queries.
           SYS_BUILD: the elements of typPoint are fixed to Rfloat. */


typedef t_RTree  *t_RT;

        /* abstract data type R-Tree */

typedef struct {
  Rint      dims;       // dimensionality
  Rint      corner;     // corner nr.
  Rint      flag;       // 0 -> not allocated, 1 -> allocated, 2 -> allocated and initialized (valid)
  Rfloat    score;      // dpoint's deadliness
  typcoord *ccoord;     // corner coordinates
  typcoord *dpoint;     // deadly point coordinates
} DPoint;

#endif /* __RSTTypes_h */
