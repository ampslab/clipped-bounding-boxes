/* -----  RSTProvMissingValues.h  ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTProvMissingValues_h
#define __RSTProvMissingValues_h


/**:   Collection of Standard Values, missing on certain platforms
       ===========================================================      **/


/**    Implementation:  Norbert Beckmann
              Version:  1.5
                 Date:  06/14                                           **/

/**    Level: normal use.                                               **/


/* This header file only includes externally, and only what is needed
   directly here. Actually nothing. */


/* ----- constants ----- */

/* Windows does not provide:
   MAXINT and MINDOUBLE. */
/* Linux does not provide:
   the MINDOUBLE we need. Instead of the minimum double > 0, it provides the
   minimum NORMALIZED double > 0. */
#if defined (_WIN32) || defined (_WIN64)
#  define MAXINT (2147483647)
#endif
#if defined (__linux__)
#  undef MINDOUBLE
#endif

#define MINDOUBLE 4.9406564584124654e-324
/* This definition establishes the following hexadecimal representation of
   a double (first two columns together = most significant nibble):
                           0  0 0 0  0 0 0 0 0 0 0 0 0 0 0 0 1   where
   number of bits used =   1  3 4 4  4 4 4 4 4 4 4 4 4 4 4 4 4   and thus
   maximum per position =  1  7 f f  f f f f f f f f f f f f f
   with meaning          sign  exp.            mant.
   This is actually the minimum double > 0, but has a precision of merely
   one bit of course.
   NOTE:
   The same binary representation results from
   2.4703282292062328e-324 up to 7.4109846876186981e-324 decimal.
   Btw:
   the minimum NORMALIZED double > 0, hexadecimal representation like above:
                           0  0 0 1  0 0 0 0 0 0 0 0 0 0 0 0 0
                         sign  exp.            mant.
   results in decimal: 2.2250738585072014e-308.
   The same binary representation results from
   2.2250738585072012e-308 up to 2.2250738585072016e-308 decimal. */

#endif /* __RSTProvMissingValues_h */
