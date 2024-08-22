# Clipping Minimum Bounding Boxes in Spatial Indexing using (Skyline) Dominance Tests

This is a public source code release for the following paper:

> D Šidlauskas, S Chester, ET Zacharatou, A Ailamaki. (2018) “Improving Spatial Data Processing by Clipping Minimum Bounding Boxes.” ICDE: 425-436. https://doi.org/10.1109/ICDE.2018.00046.  
  
_Please note_: I am hosting the source code so that it is publicly available, but I am a minor author in the code base. We are not able to share the den* and axo* datasets.

### Paper Abstract
The majority of spatial processing techniques rely heavily on the idea of approximating each group of spatial objects by their minimum bounding box (MBB). As each MBB is compact to store (requiring only two multi-dimensional points) and intersection tests between MBBs are cheap to execute, these approximations are used predominantly to perform the (initial) filtering step of spatial data processing. However, fitting (groups of) spatial objects into a rough box often results in a very poor approximation of the underlying data. The resulting MBBs contain a lot of "dead space"---fragments of bounded area that contain no actual objects---that can significantly reduce the filtering efficacy.  
  
This paper introduces the general concept of a *clipped* bounding box (CBB) that addresses the principal disadvantages of MBBs, i.e., their poor approximation of spatial objects. Essentially, a CBB "clips away" dead space from the corners of an MBB by storing only a few auxiliary points. We present two approaches for computing the most promising such points (i.e., those that remove the most dead space) and thus two variants of the CBB. Turning to four popular R-trees implementations (a ubiquitous application of MBBs), we demonstrate how minor modifications to the query algorithm can exploit our CBB auxiliary points to avoid many unnecessary recursions into dead space. Extensive experiments show that clipped R-tree variants substantially reduce I/Os: e.g., by clipping the state-of-the-art revised R*-tree we can eliminate on average 18% of I/Os.  

### Why L-Tree?
Because the new MBB shape using one clip point resembles (capital) "L" letter.

### Subproject directories:

**L-tree:** contains the source code of the original RSTree bundle and our extensions for the L-tree.

**DataGenerator:** contains the original data generator (in `distNEW/`) used to produce the workloads in the RR*-tree paper. Also, it contains some useful tools (in `tools/`) for dealing with the generated files (e.g., their visualization).

**DeadSpace:** contains the implementation of dead space computation and elimination (based on skyline) algorithms.

_For more details, check the README files within each directory._
