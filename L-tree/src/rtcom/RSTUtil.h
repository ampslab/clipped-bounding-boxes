/* -----  RSTUtil.h  ----- */
#//
#// Copyright (c) 1994 - 2015 Norbert Beckmann  <nb@informatik.uni-bremen.de>
#//
#// Licensed under the Apache License, Version 2.0 (see root directory)
#//
#ifndef __RSTUtil_h
#define __RSTUtil_h


/**:   universal Utility routines
       ==========================                                       **/


/**    Implementation:  Norbert Beckmann
              Version:  10.2
                 Date:  12/13                                           **/

/**    Level: intermediate.
                                                                        **/

#include "RSTDistQueryBase.h"
#include "RSTErrors.h"

/* declarations */

void     ClearVector(typword *ptr,
                     Rint wordsqty);

void     CopyRect(Rint numbOfDim,
                  const typinterval *from,
                  typinterval *to);

void     CopyPoint(Rint numbOfDim,
                   typcoord *from,
                   typcoord *to);

void     EvalCenter(Rint numbOfDim,
                    typinterval *rectangle,
                    typcoord *center);

Rfloat   PPDistance(Rint numbOfDim,
                    typcoord *point1,
                    typcoord *point2);
/* raw (i.e. quadratic) euklidian distance between two points */

Rfloat   PPDist(Rint numbOfDim,
                DistCalcFunc DistFunc,
                typcoord *point1,
                typcoord *point2);
/* raw DistFunc-dependend distance between two points */

Rfloat   PRectDist(RSTREE R,
                   DistCalcFunc DistFunc,
                   typcoord *point,
                   typinterval *rect);
/* raw DistFunc-dependend distance between a point and the nearest
   edge(/corner) of a rectangle */

Rfloat PRectMaxDist(RSTREE R,
                    DistCalcFunc DistFunc,
                    typcoord *point,
                    typinterval *rect);
/* raw DistFunc-dependend distance between a point and the farthest corner of
   a rectangle */

Rfloat PRectCntrDist(RSTREE R,
                     DistCalcFunc DistFunc,
                     typcoord *point,
                     typinterval *rect);
/* raw DistFunc-dependend distance between a point and the center of a
   rectangle */

void     EvalDirNodesMBB(RSTREE R,
                         refnode DIRnode,
                         typinterval *mbb);

void     EvalDataNodesMBB(RSTREE R,
                          refnode DATAnode,
                          typinterval *mbb);

boolean  RectsEql(Rint numbOfDim,
                  const typinterval *rect1,
                  const typinterval *rect2);

boolean  Covers(Rint numbOfDim,
                const typinterval *crect,
                const typinterval *rect);

boolean  Overlaps(Rint numbOfDim,
                  typinterval *rect1,
                  typinterval *rect2);

boolean  DirOvlps(RSTREE R,
                  const typinterval *rect1,
                  const typinterval *rect2);
/* Only difference referring to Overlaps: DirOvlps counts (dir)comparisons */

boolean  DataOvlps(RSTREE R,
                   const typinterval *rect1,
                   const typinterval *rect2);
/* Only difference referring to Overlaps: DataOvlps counts (data)comparisons */

void     GetOverlap(Rint numbOfDim,
                    typinterval *r1,
                    typinterval *r2,
                    Rfloat *spc);
         /* NOTE that GetOverlap may only be called after Overlaps yielded
            TRUE. */
/* Provides the spatial overlap */

Rfloat   OverlapEnl(RSTREE R,
                    typinterval *oldRect,
                    typinterval *enlRect,
                    typinterval *otherRect);
/* Returns overlap(enlRect,otherRect) - overlap(oldRect,otherRect) (spatial),
   and may be called without pre-query (in contrast to e.g. GetOverlap). */

void     GetOvlpEdge(Rint numbOfDim,
                     typinterval *r1,
                     typinterval *r2,
                     Rfloat *edge);
         /* NOTE that GetOvlpEdge may only be called after Overlaps yielded
            TRUE. */
/* Provides the "edgial" overlap */

Rfloat   OvlpEdgeEnl(RSTREE R,
                     typinterval *oldRect,
                     typinterval *enlRect,
                     typinterval *otherRect);
/* Returns overlap(enlRect,otherRect) - overlap(oldRect,otherRect) ("edgial"!),
   and may be called without pre-query (in contrast to e.g. GetOvlpEdge). */

void     GetOvlpRect(Rint numbOfDim,
                    typinterval *r1,
                    typinterval *r2,
                    typinterval *inter);
         /* NOTE that GetOvlpRect may only be called after Overlaps yielded
            TRUE. */
/* Provides the intersection rectangle of two overlapping rectangles */

boolean IsArea0(Rint numbOfDim, typinterval *r);
/* Returns TRUE if the area of r is 0 (because its extension is 0 in at least
   one dimension), otherwise FALSE. */

void     QSortRints(Rint begin,
                    Rint end,
                    Rint value[]);

void     QSortRpnints(Rlint begin,
                      Rlint end,
                      Rpnint value[]);
/* Needs type Rlint indices because Rpnint may be unsigned. */

void     QSortRfloats(Rint begin,
                      Rint end,
                      Rfloat value[]);

void     QSortIofRfloats(Rint begin,
                         Rint end,
                         ValueArray value,
                         Rint I[]);

void     QSortIofEnts(Rint begin,
                      Rint end,
                      Rint dim,
                      Side side,
                      void *entArr,
                      Rint entrylen,
                      Rint I[]);

void     InstallDirPath(RSTREE R,
                        Rint level,
                        typinterval *rect,
                        Rpnint pagenr);

/* minimum finders: */

Rfloat LeastRfloat(Rint begin, Rint end, Rfloat val[]);
/* Searches the array val from begin to end.
   Returns the value of the least element encountered. */

Rint IofLeastRfloat(Rint begin, Rint end, Rfloat val[], Rint I[]);
/* Searches the index array I from begin to end, examining values of the array
   val.
   Returns val's index of the least element of val between I's begin and end.
   In other words:
   Searches val[I[begin]] .. val[I[end]], returns I[i] (begin <= i <= end). */

Rint IofIofLeastRfloat(Rint begin, Rint end, Rfloat val[], Rint I[]);
/* Searches the index array I from begin to end, examining values of the array
   val.
   Returns I's index of the least element of val between I's begin and end.
   In other words:
   Searches val[I[begin]] .. val[I[end]], returns i (begin <= i <= end). */

/* maximum finders: */

Rfloat MaxRfloat(Rint begin, Rint end, Rfloat val[]);
/* Searches the array val from begin to end.
   Returns the value of the maximum element encountered. */

Rint IofMaxRfloat(Rint begin, Rint end, Rfloat val[], Rint I[]);
/* Searches the index array I from begin to end, examining values of the array
   val.
   Returns val's index of the greatest element of val between I's begin and
   end.
   In other words:
   Searches val[I[begin]] .. val[I[end]], returns I[i] (begin <= i <= end). */

Rint IofIofMaxRfloat(Rint begin, Rint end, Rfloat val[], Rint I[]);
/* Searches the index array I from begin to end, examining values of the array
   val.
   Returns I's index of the greatest element of val between I's begin and end.
   In other words:
   Searches val[I[begin]] .. val[I[end]], returns i (begin <= i <= end). */

/* error setting and printing: */

void setRSTerr(RSTREE     rst,
               char       *preMessage,
               typerrors  error);

void setRSTwarn(RSTREE     rst,
                char       *preMessage,
                typerrors  error);

/* other: */

void AlignInt(Rint *numb, Rint alignm);
/* Adjusts the address offset (an Rint) provided in numb, such that it
   conforms with the alignment provided in alignm. */


#endif /* __RSTUtil_h */
