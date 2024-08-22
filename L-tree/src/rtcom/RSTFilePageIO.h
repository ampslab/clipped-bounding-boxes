/* -----  RSTFilePageIO.h  ----- */
#//
#// Copyright (c) 1994 - 2015 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTFilePageIO_h
#define __RSTFilePageIO_h


/**:   Pagewise I/O operations from/to Files
       =====================================                            **/


/**    Implementation:  Norbert Beckmann
              Version:  2.2
                 Date:  12/13                                           **/

/** Functionality:
    The functions of this headerfile provide pagewise read and write
    from/to secondary memory residing files.
    Properties:
    A file can be thought to consist of a set of equal sized pages.
    A page is qualified by its number.
    Level: intermediate.
    Higher levels: RSTPageInOut.h                                       **/


#include "RSTBase.h"


void ReadPage(RSTREE R,
              typstorage *str,
              Rpnint pagenr,
              void *block);

/* NOTE that due to typstorage in both (*R).DIR and (*R).DIR.PA,
                                       (*R).DATA and (*R).DATA.PA
   respectively, administrative counts go separate! */


void WritePage(RSTREE R,
               typstorage *str,
               Rpnint pagenr,
               void *block);

/* NOTE that due to typstorage in both (*R).DIR and (*R).DIR.PA,
                                       (*R).DATA and (*R).DATA.PA
   respectively, administrative counts go separate! */


#endif /* __RSTFilePageIO_h */
