#!/bin/bash
# -----------------------------------------------------------------------------
# Script to for running Brinkhoff (improved version of computeMCR for VLDB'17).
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
#TREE="HR" ;
variant="sky" ;

echo "${TREE} [${variant}]" ;

# For mapping to command line options:
declare -A cl_var=( ["sky"]="-s" ["sta"]=" " )

# IPython dir for storing result files:
IPYTHON_DIR="/home/sidlausk/Dropbox/Docs/ipython-notebooks/Chamfered-R-Tree/stored_results/brinkhoff/" ;

DFAC=2 ;
max_dpoints=$( echo 2^$DIMS*$DFAC | bc ) ;
echo "DIMS=${DIMS}";
echo "max_dpoints=${max_dpoints}";

dumps_dir="${DIR}/../../L-tree/dumps/data/dumps4motivation/"
exe="${DIR}/../bin/computeMCRs" ;

for datafile in `/bin/ls -m1 ${dumps_dir}*${DATA}${DIMS}.RSF.ASC`; do
  if [[ ${datafile} != *"${DIMS}"* ]]; then
    continue ;
  fi
  if [[ ${datafile} != *"${TREE}"* ]]; then
    continue ;
  fi

# Few files that I prefer to skip:
   if [[ ${datafile} == *"axo"* ]]; then
     continue ;
   fi
#   if [[ ${datafile} == *"rea"* ]]; then
#     continue ;
#   fi
   if [[ ${datafile} == *"ger"* ]]; then
     continue ;
   fi
   if [[ ${datafile} == *"par"* ]]; then
     continue ;
   fi
   if [[ ${datafile} == *"tst"* ]]; then
     continue ;
   fi

  echo "Processing.. ${datafile:${#datafile}<40?0:-40}" ; # ${string:${#string}<3?0:-3}
  #echo "  ${exe} -i ${datafile} -v -m ${max_dpoints}" ;
  
  ${exe} -i ${datafile} -v -m ${max_dpoints} ${cl_var[$variant]} ;
  filename_ext="${datafile##*/}" ;
  filename="${filename_ext%.RSF.ASC}" ; 
  outfilename="${filename}_MCR-${variant}.csv" ;
  echo "OUTPUT: ${outfilename}" ;
  if [ ! -f ${outfilename} ]; then
    echo "ERROR: the above file not found!" ;
  fi
  mv "${outfilename}" "${IPYTHON_DIR}" ;
done
