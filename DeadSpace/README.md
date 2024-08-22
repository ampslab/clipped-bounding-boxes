# README File for the DeadSpace Project #

The **DeadSpace** Project: implements dead space computation and elimination (based on skyline) algorithms for R-tree based index structures, i.e., within their nodes enclosed with a minimum bounding rectangle (MBR). 

# Usage #

*TODO!*

# Other notes #

## How to calculate dead speace area/volume in a tree node ##

** Current (approximate) approach **
At the moment we create a fixed resolution grid where each cell represents square/cubic area/volume in 2D/3D. Each cell is mapped to an integer that is set to 0 initialy. Then, for each rectangle within a node, we compute the overlapping cells and +1 to their mapped integers. This way, all zero-value cells indicate the dead (empty) space, while a non-zero cell is part of at least one rectangle: the count indicates how many rectangles a particular cell is overlapped with.

This is appoximate approach and its accuracy depends on the chosen grid's resolution (currently it depends on node MBR's shape but is up to 100 cells per dimension). It's difficult to compute the lower bound, as all cells could be only partially overlapping.

** New (accurate) approach **
We can partition a node based on its children coordinates as follows (assuming 2D). We split a node into four partitions by taking child's left-lower point as a center of the split: it divides the space into two at each dimension. The next split on child's upper right point divides futher the righter part and upper part each into two intervals (on each dimension). As such, after splitting node into grid based on one child (rectangle), we have 6 partitions. We do the same for all children in a node. This results in partitioning a node into two types of partitions: (i) either _fully covered_ or (ii) _completely empty_ partitions. Compute the areas of completely empty partitions gives us the precise dead space area in a node.
  
Note that this approach is more complicated, as it requires maintaining different size cells (compared to uniform grid above). Essentially, we need to build maintain a data structure known as _grid file_.

