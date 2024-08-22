#!/bin/bash
# -----------------------------------------------------------------------------
# Running computeMCR on dumps for motivation figure
#

if [ "$#" -ne 1 ]
then
  echo "Error: enter #dimensions using two digits!" ;
  exit 2 ;
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )" ;

DIMS=$1 ;
DATA="" ;
TREE="RRS" ;

# First results (used in motivation Figure (a) and (b)) stored here:
# IPYTHON_DIR="/home/sidlausk/Dropbox/Docs/ipython-notebooks/Chamfered-R-Tree/stored_results/motivation_Figures_AB/" ;

# Calculating truly pruned deadspace:
IPYTHON_DIR="/home/sidlausk/Dropbox/Docs/ipython-notebooks/Chamfered-R-Tree/stored_results/truly-pruned/" ;

# Calculating accumulated (with extra clip-point) truly pruned deadspace:
#IPYTHON_DIR="/home/sidlausk/Dropbox/Docs/ipython-notebooks/Chamfered-R-Tree/stored_results/accumulated-truly-pruned_sky/" ;

echo "DIMS=${DIMS}";
max_dpoints=16 ; # or 8
echo "max_dpoints=${max_dpoints}";

data_dir="${DIR}/../../L-tree/dumps/data/dumps4motivation/"
exe="${DIR}/../bin/computeMCRs" ;

for datafile in `/bin/ls -m1 ${data_dir}*${DATA}${DIMS}.RSF.ASC`; do
  if [[ ${datafile} != *"${DIMS}"* ]]; then
    continue ;
  fi

# Few files that I prefer to skip:
   if [[ ${datafile} == *"axo"* ]]; then
     continue ;
   fi
   if [[ ${datafile} == *"rea"* ]]; then
     continue ;
   fi
   if [[ ${datafile} == *"ger"* ]]; then
     continue ;
   fi
   if [[ ${datafile} == *"par"* ]]; then
     continue ;
   fi

  echo "Processing.. ${datafile:${#datafile}<40?0:-40}" ; # ${string:${#string}<3?0:-3}
  
  ${exe} -i ${datafile} -v -m ${max_dpoints} ; # To compute skyline-based: -s
  filename_ext="${datafile##*/}" ;             # Change output file too then
  filename="${filename_ext%.RSF.ASC}" ;        # 
  outfilename="${filename}_MCR-sta.csv" ;      # or sky
  echo "OUTPUT: ${outfilename}" ;
  if [ ! -f ${outfilename} ]; then
    echo "ERROR: the above file not found!"
  fi
  mv "${outfilename}" "${IPYTHON_DIR}" ;
done
