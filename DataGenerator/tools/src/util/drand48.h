/* ----- drand48.h ----- */
#//
#// Copyright (c) 2013 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//

#ifndef __drand48_h
#define __drand48_h

/**:   Unix Random Generator drand48
       =============================                                    **/

/** Header file derived from Unix's manuals and stdlib.h respectively.
    There are old 16 bit versions of the source drand48.c in the net,
    without any copyright or permission notice.
    Example: http://www.win.tue.nl/~aeb/ftpdocs/linux-local/libc.archive/libc/libc-4.4.1/misc/drand48.c                                                   **/

/** Re-Implementation:  Norbert Beckmann
              Version:  1.3
                 Date:  06/14                                           **/

/** Properties:
    This is a 64 bit and thus strongly simplified re-implementation
    of Unix's 48 bit random generator drand48. It provides the same
    16 bit interface as the original, being integral part of any Unix
    implementation, and yields exactly the same number sequences.       **/


#include <stdlib.h>

/* -----------------------------------------------------------------------
   Comments below are brief descriptions and address people, being used to
   drand48. For detailed information see manuals drand48(3) / drand48(3C).
   ----------------------------------------------------------------------- */

double drand48(void);
/* call by call, returns non-negative double-precision floating-point values,
   uniformly distributed over the interval [0.0, 1.0). */

double erand48(unsigned short X[3]);
/* Number sequence: see drand48(). Differences: A state buffer has to be
   provided in X, this way allowing multiple independent number sequences.
   The 48 bit state, initially provided in X, determines which number
   sequence is returned consecutively. */

long lrand48(void);
/* call by call, returns non-negative long integers uniformly distributed
   over the interval [0, 2^31 - 1]. */

long nrand48(unsigned short X[3]);
/* Number sequence: see lrand48(). Differences: see erand48(). */

long mrand48(void);
/* call by call, returns signed long integers uniformly distributed over the
   interval [-2 ^31, 2 ^31 - 1]. */

long jrand48(unsigned short X[3]);
/* Number sequence: see mrand48(). Differences: see erand48(). */

void srand48(long seedval);
/* sets the 48 bit state via the 32 bit provided seedval.
   Additionally resets the internal parameters to their defaults. */

unsigned short *seed48(unsigned short seed16v[3]);
/* 1. sets the 48 bit state.
   2. returns a pointer to the previous 48 bit state, allowing re-entrance
      into the running random sequence.
      Additionally resets the internal parameters to their defaults. */

void lcong48(unsigned short param[7]);
/* sets the complete set of parameters of the random generator. To retain
   the original behavior, param should contain
   - a 48 bit state in param[0-2]
   and the following internal parameters:
   - the multiplier 0x5DEECE66D in param[3-5],
   - the addend 0xB in param[6].
   Notice: drand48 expects 32 and 48 bit integers in little endian format.
   Example: multiplier 0x5DEECE66D = [0xE66D] [0xDEEC] [0x5]. */

#endif /* __drand48_h */

