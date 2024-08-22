#!/bin/bash

# Feed a ACSI dump of RR*-tree, compute L-tree related stuff and ouput the following files:
# - dump_sky.txt: skyline points for each tree node
# - dump_stair.txt: staircase points for each tree node
# - dump_dead.txt: chosen deadly points for each tree node
#
# The output files have one line per tree node.
#

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )" ;

dtype="skyline" ;
suffix="sky" ;

if [ "$#" -eq 1 ]
then
  dtype="$1";
  
  if [ "$dtype" != "skyline" ]; then
    suffix="stair" ;
  fi
fi

exe="$DIR/../release/${dtype}DeadSpace" ;

# DON'T FORGET TO COMPILE FOR CORRECT DIMS!
# and choose a corresponding input file:
#infile="$DIR/../../L-tree/test_data/rea02"
infile="$DIR/../../L-tree/test_data/axo03"

CMD="$exe -i ${infile}.RSF.ASC -o -l 1" ;
echo "$CMD" ;

$CMD ;

# Latest test required this output sorted:
echo "Sorting: sort -n -t: -k1 ${infile}.RS_${suffix}-dead.txt"
sort -n -t: -k1 ${infile}.RS_${suffix}-dead.txt -o ${infile}.RS_${suffix}-dead.txt

