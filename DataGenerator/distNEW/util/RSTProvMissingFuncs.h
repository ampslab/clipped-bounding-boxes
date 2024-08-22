/* -----  RSTProvMissingFuncs.h  ----- */
#//
#// Copyright (c) 1994 - 2013 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTProvMissingFuncs_h
#define __RSTProvMissingFuncs_h


/**:   Collection of Standard Functions, missing on certain platforms
       ==============================================================   **/


/**    Implementation:  Norbert Beckmann
              Version:  1.4
                 Date:  11/13                                           **/

/**    Level: normal use.                                               **/


#include <sys/types.h> /* size_t */

/* This header file only includes externally, and only what is needed
   directly here. */


/*** ------------------------------ ***/
/* No strlcpy() for you, Linux programmer?
   strlcpy() is a buffer overflow save string copying function.
   OpenBSD, FreeBSD, Solaris, Mac OS X, QNX provide it, and it is even used
   internally in the Linux kernel.
   [http://en.wikipedia.org/wiki/C_string_handling] */
/* No strnlen in minGW32 though available in minGW64?
   strnlen() is a buffer overflow save string length test function. */
/*** ------------------------------ ***/
/*********************************************************************/
/* The following copyright notices and permission notice
   exclusively apply to the functions:
   strlcpy
   strlcat
   strnlen
   declared below! */
/*	$OpenBSD: strlcpy.c,v 1.11 2006/05/05 15:27:38 millert Exp $	*/
/*	$OpenBSD: strlcat.c,v 1.13 2005/08/08 08:05:37 espie Exp $	*/
/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 */
/*	$OpenBSD: strnlen.c,v 1.3 2010/06/02 12:58:12 millert Exp $     */
/*
 * Copyright (c) 2010 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*********************************************************************/

/* ----- declarations ----- */

size_t  strlcpy(char *dst, const char *src, size_t siz);
/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
 
 
size_t  strlcat(char *dst, const char *src, size_t siz);
/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */

size_t strnlen(const char *str, size_t maxlen);

/* From FreeBSD Man Page:
     The strlen() function computes the length of the string s.  The strnlen()
     function attempts to compute the length of s, but never scans beyond the
     first maxlen bytes of s.
*/

#endif /* __RSTProvMissingFuncs_h */
