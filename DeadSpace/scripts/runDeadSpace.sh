#!/bin/bash
# -----------------------------------------------------------------------------

if [ "$#" -ne 2 ]
then
  echo "Error: enter #dimensions using two digits!" ;
  echo "Example: runDeadSpace.sh stair|sky 03"
  exit 2 ;
fi

DIMS=$2 ;
#DATA="den" ;
DATA="" ;

data_dir="../../L-tree/dumps" # -i rea02.RSF.ASC
DeadSpace_exe="../release/staircaseDeadSpace" ;

if [ "$1" == "sky" ]; then
  DeadSpace_exe="../release/skylineDeadSpace" ;
fi

echo "Running $DeadSpace_exe" ;
echo "data opt least-dead most-dead average avg1 avg2 avg3 avg4 avg5 avg6 avg7 avg8"

for datafile in `/bin/ls -m1 ${data_dir}/*${DATA}${DIMS}.RSF.ASC`; do
  if [[ ${datafile} != *"${DIMS}"* ]]; then
    continue ;
  fi

  OUTPUT=$( ${DeadSpace_exe} -i ${datafile} -l 0) ;
  filename_ext="${datafile##*/}"
  filename="${filename_ext%.RSF.ASC}"
  
  echo "${filename} ${OUTPUT}"

done

