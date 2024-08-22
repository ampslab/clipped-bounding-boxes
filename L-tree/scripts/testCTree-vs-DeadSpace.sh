#!/bin/bash
# -----------------------------------------------------------------------------
# Command line for valgrind check:
# valgrind --leak-check=full ./bin/lrrst < test_data/tmp_infile > tmp_out
#

function exec() {

  datafile=$1 ;
  inFile=$2 ;
  
  echo "c" > ${inFile} ;
  echo "${datafile}" >> ${inFile} ;
  echo "4096" >> ${inFile} ;
  echo "4096" >> ${inFile} ;
  echo ${datafile:(-2)} >> ${inFile} ; # extract #dimensions
  echo "4" >> ${inFile} ;
  echo "n" >> ${inFile} ;
  echo "i" >> ${inFile} ;
  echo "0 0" >> ${inFile} ;
  echo "o" >> ${inFile} ;
  echo "0" >> ${inFile} ;
  echo "I" >> ${inFile} ;
  echo "A" >> ${inFile} ;
  echo "f" >> ${inFile} ; # dump to file
  echo "X" >> ${inFile} ;
  echo "0" >> ${inFile} ;
  echo "1" >> ${inFile} ; # 0 -> Skyline; 1 -> Staircase
  echo "n" >> ${inFile} ;
  echo "f" >> ${inFile} ;
  echo "-" >> ${inFile} ;
  echo "." >> ${inFile} ; # Close the created tree (free memory)
  echo "q" >> ${inFile} ;
  echo "" >> ${inFile} ;
  
  echo "${BIN}${exe}  < ${inFile} > ${datafile}_output" ;
  ${BIN}${exe}  < ${inFile} > "${datafile}_output" ;
}

if [ "$#" -eq 0 ]
then
  echo "Using default datafile: test_data/test_data" ;
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )" ;
BIN="${DIR}/../bin/" ;

if [ "$#" -eq 1 ]
then
  echo "Using input datafile: $1" ;
fi

exe="lrrst" ; # L'ized Revised R*-tree
tree_name="LRRS" ;

STARTTIME=$(date +%s) ;

data_rea02="${DIR}/../test_data/rea02" ;
data_axo03="${DIR}/../test_data/axo03" ;

datafile="$1" ;

infile="${DIR}/tmp_infile" ;

# Perform C-Tree computations
exec $datafile $infile ;
# clean up (leave only ASCI and MCR dumps):
rm -v -f ${datafile}.RSF ;
rm -v -f ${datafile}.RSF.D* ;
echo "DONE: L-tree run finsihed!" ;

# sort and store the sorted file in scripts/
CT_mcr_dump="${datafile}.RRS.MCR" ;
CT_bn=$(basename $CT_mcr_dump) ;
echo "sort -n -k1 -t: ${CT_mcr_dump} > ${DIR}/sorted_${CT_bn}" ;
sort -n -k1 -t: "${CT_mcr_dump}" > "${DIR}/sorted_${CT_bn}";

# Perform DeadSpace Computations
echo "" ;
echo "Running computeMCRs" ;
exeMCR="${DIR}/../../DeadSpace/bin/computeMCRs -o";
${exeMCR} -i "${datafile}.RSF.ASC" ;
echo "DONE: computeMCRs finished!" ;

DS_mcr_dump="${datafile}.RS_MCR-sta-dspace.txt" ;
DS_bn=$(basename $DS_mcr_dump) ;
echo "sort -n -k1 -t: ${DS_mcr_dump} > ${DIR}/sorted_${DS_bn}" ;
sort -n -k1 -t: "${DS_mcr_dump}" > "${DIR}/sorted_${DS_bn}";

# OUTPUT COMPARISON:
DS_num_lines=( $(wc -l ${DIR}/sorted_${DS_bn}) ) ; # extra brackets to make it array (of words)
CT_num_lines=( $(wc -l ${DIR}/sorted_${CT_bn}) ) ; # extra brackets to make it array (of words)
DS_num_lines=${DS_num_lines[0]} ;
CT_num_lines=${CT_num_lines[0]} ;

common_lines=0 ;

if [ ${DS_num_lines} -gt ${CT_num_lines} ]
then
  printf "%12s %12s %12s\n" "DS_OUT_LINES" "COMMON_LINES" "CT_OUT_LINES" ;
  common_lines=$(grep -Fxf ${DIR}/sorted_${DS_bn} ${DIR}/sorted_${CT_bn} | wc -l) ;
else
  printf "%12s %12s %12s\n" "CT_OUT_LINES" "COMMON_LINES" "DS_OUT_LINES" ;
  common_lines=$(grep -Fxf ${DIR}/sorted_${CT_bn} ${DIR}/sorted_${DS_bn} | wc -l) ;
fi

printf "%12s %12s %12s\n" "$DS_num_lines" "${common_lines[0]}" "$CT_num_lines" ;

echo "" ;
ENDTIME=$(date +%s) ;
elapsed=$(($ENDTIME - $STARTTIME)) ;
hours=$(($elapsed / 3600)) ;
mins=$(($elapsed % 3600 / 60)) ;
secs=$(($elapsed % 3600 % 60)) ;
echo "ALL DONE at $(date '+%H:%M:%S'), duration: $(($hours))h:$(($mins))m:$(($secs))s" ;
