/* -----  RSTStdTypes.h  ----- */
#//
#// Copyright (c) 1994 - 2014 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTStdTypes_h
#define __RSTStdTypes_h


/**:   Collection of Standard Types and a printing facility
       ====================================================             **/


/**    Implementation:  Norbert Beckmann
              Version:  3.7
                 Date:  05/14                                           **/

/**    Level: hybrid: definitions: bottom, functions: normal use.       **/


/**    Notice: NOTE, SYS_BUILD, RESTRICT.                               **/

/* NOTE:
   Constants and types below labeled with "SYS_BUILD" should be adapted
   partly during library build, to compile the desired version of the
   index structure. */
   

/* Includes:
   Apart from the RSTProvMissingxxx.h files, this header file only
   includes externally, and only what is needed directly here.
   The contents of the RSTProvMissingxxx.h files, originally having been
   part of this header file, were excluded from here for better
   modularity. */


#if defined (_WIN32) || defined (_WIN64) || defined (__FreeBSD__)
   /* Windows and FreeBSD do not provide <values.h>, thus
      get necessary values through RSTProvMissingValues.h */
#else
#  include <values.h> /* MAXINT, MINDOUBLE, .. */
#endif

/*** ------ includes of values, un-/mis-defined on certain platforms ------ */
#if defined (__linux__) || defined (_WIN32) || defined (_WIN64) || defined (__FreeBSD__)
#  include "RSTProvMissingValues.h"
#endif

/*** ------ includes of functions, missing on certain platforms ------ */
#if defined (__linux__) || defined (_WIN32) || defined (_WIN64)
#  include "RSTProvMissingFuncs.h"
   /* includes <sys/types.h> */
#endif


/* Determination of endianess: */
#if defined (__linux__)
#  if __BYTE_ORDER == __LITTLE_ENDIAN
#    define LITTLE_ENDIAN_DETERMINED
#  elif __BYTE_ORDER == __BIG_ENDIAN
#    define BIG_ENDIAN_DETERMINED
#  else
#    error "__BYTE_ORDER information MISSING"
#  endif
#elif defined (__FreeBSD__)
#  if _BYTE_ORDER == _LITTLE_ENDIAN
#    define LITTLE_ENDIAN_DETERMINED
#  elif _BYTE_ORDER != _LITTLE_ENDIAN
#    define BIG_ENDIAN_DETERMINED
#  else
#    error "_BYTE_ORDER information MISSING"
#  endif
#elif defined (_WIN32) || defined (_WIN64)
   /* Windows does not maintain endian information,
      but only seems to know little endian (?):
      Choose the little endian definition: */
#  define LITTLE_ENDIAN_DETERMINED
#else
#  if defined (_LITTLE_ENDIAN) && defined (_BIG_ENDIAN)
#    error "ENDIAN information AMBIGUOUS"
#  elif defined (_LITTLE_ENDIAN)
#    define LITTLE_ENDIAN_DETERMINED
#  elif defined (_BIG_ENDIAN)
#    define BIG_ENDIAN_DETERMINED
#  else
#    error "ENDIAN information MISSING"
#  endif
#endif


#include <stdint.h> /* int64_t, .. */

/* --- RSTree boolean types and constants: --- */

typedef int  boolean;
#define FALSE 0
#define TRUE 1


/* --- RSTree file types and constants: --- */

typedef int  File;

/**:   Standard transitional sizes scheme under 32/64 bit compilation

	size under 32 bit	size under 64 bit	general size

void *		4			8

char		1			1		(surely 1)
short		2			2		(surely 2)
int		4			4		(surely 4)
long		4			8		(surely pointer size)
long long	8			8		(surely 8) */

/* Contrary to Unix, Windows does not serve a transitional interface for
   32/64 bit compilation. The following definitions shall make this
   implementation transitional. But note the following cutout from
   README.Platform:
   In a Windows environment, the implementation assumes that the POINTER SIZE
   OPTION, given to the compiler, be EQUAL TO THE POINTER SIZE OF THE OPERATING
   SYSTEM. */
/* Begin general definitions for Windows and 64 bit compilation
     NOTE that some functions under Windows may need a leading underscore. (?) 
     NOTE that the following definitions may be subject to changes. */
#if defined (_WIN64)
   /* workaround missing transitional integer types:
      see "pointer compatible" below */
   /* workaround missing transitional struct types: */
#  define stat stat64
   /* workaround missing transitional functions: */
#  define fstat fstat64
#  define lseek lseek64
   /* >>> WARNING <<<
      >>> The following transition is already done in minGW64: */
//#  define fseek  fseeko64
#endif
/* End   general definitions for Windows and 64 bit compilation */
/*** ----------------------------------------------------------------- ***/
/*** NOTE that external includes MUST NOT appear behind these defines! ***/
/*** ----------------------------------------------------------------- ***/

/* --- RSTree integer types (SYS_BUILD): --- */

typedef unsigned char 		byte;		/* 1 byte unsigned */

typedef          short 		Rshort;		/* 2 bytes signed */
typedef unsigned short 		Rushort;	/* 2 bytes unsigned */

typedef          int 		Rint;		/* 4 bytes signed */
typedef unsigned int 		Ruint;		/* 4 bytes unsigned */

#if defined (_WIN64)
  typedef    int64_t		Rpint;		/* pointer compatible */
#else
  typedef        long		Rpint;		/* pointer compatible */
#endif

typedef          long long	Rlint;		/* 8 bytes signed */
typedef unsigned long long	Rulint;		/* 8 bytes unsigned */

/* derived simple types: */

typedef          Ruint		Rpnint;		/* see below */

/* It is of basic importance that the integer types be adjusted, such that
   their types confirm to the types requested in the comments.
   Explanatory notes:
   Rpint:	used in the context of memory- and file- pointers.
   Rpnint:	used in the context of subtree pointers, i.e. page numbers;
		the types FPst and (more important) FPun depend on it.
   Rlint:	used in the context of counters and for special purposes. */

/* derived compound types: */

/* FPst: byte order dependend compound type for file and position: */
#if defined (LITTLE_ENDIAN_DETERMINED)
   typedef struct {
                  Rpnint  p;
                  File    f;
                  } FPst;
#elif defined (BIG_ENDIAN_DETERMINED)
   typedef struct {
                  File    f;
                  Rpnint  p;
                  } FPst;
#else
#  error "type FPst undefined: ENDIAN information MISSING"
#endif

/* --- RSTree floating point types and constants: --- */

typedef double  Rfloat;

/* All-round RSTree floating point type.
   Should at least be a double. */


#define MIN_RFLOAT MINDOUBLE

/* The minimum Rfloat > 0 (which is denormalized of course).
   Should be adapted if the definition of type Rfloat is modified. */


#define Infinity  ((Rfloat)(42 / 0.0))

/* ... according to the IEEE definition.
   Note: the text output of this constant, printed with printf(), varies
   between different OSs, but is commonly something like "Infinity". */


/* --- RSTree string types and constants: --- */

#define MaxNameLength 160
typedef char  RSTName[MaxNameLength];

/* String type for names. */


/*** --------------------- function declarations ----------------------- */

char     *strans(char *ch, char *buf);

         /* strans is a wrapper for the format string of the printf function.
            It provides additional conversion characters for the ASCII
            conversion of the integer types defined in RSTStdTypes.h.
            Other characters and format strings are passed unmodified.
            But see the RESTRICTION at the end of this chapter.
            
            ch:
            is a pointer to a character string to be converted.
            
            buf:
            is a pointer to a character buffer to be provided, long enough to
            contain the converted string.
            
            The function returns a pointer to buf.
            
            New conversions in detail:
            ----------------------------------------------------------
            %*I prints an Rint   (decimal),
            %*y prints an Ruint  (hexadecimal),
            
            %*P prints an Rpint  (decimal),
            %*Q prints an Rpint  (hexadecimal),
            
            %*L prints an Rlint  (decimal),
            %*Y prints an Rulint (hexadecimal),
            
            %*N prints an Rpnint (decimal).
            
            The "*" denotes, that additional formatting characters are
            allowed as in the printf function.
            ----------------------------------------------------------
            
            A RESTRICTED subset of the other %-conversions is allowed in the
            same string,
            namely %%, %*c, %*d, %*e, %*f, %*g, %*n, %*o, %*p, %*s, %*u, %*x.
            These are passed through unmodified.
            
            Example:
            char s[80];
            Rlint alpha;
            int beta;
            ...
            
            printf(strans("alpha: %2L   beta: %2d\n",s),alpha,beta); */


void     PrintRSTStdTypesConsts(void);

         /* PrintRSTStdTypesConsts prints a list to stdout, containing
            important sizes and constants defined in this header file. */


/*** ---------------------------- private ------------------------------- */

typedef struct {int i;} t_RTree;	// *t_RT
typedef struct {int i;} t_LRUbuf;	// *t_LRU
typedef struct {int i;} t_DistQuery;	// *t_DQ
typedef struct {int i;} t_FilePageBag;	// *t_FPB
typedef struct {int i;} t_FilePageSet;	// *t_FPS
typedef struct {int i;} t_RpnintBag;	// *t_pnB
typedef struct {int i;} t_RpnintSet;	// *t_pnS
typedef struct {int i;} t_FileBuf;      // *t_FB
/* by union, build a big number from type FPst: */
typedef union {
              FPst   s;
              Rlint  i;
              } FPun;


#endif /* __RSTStdTypes_h */
