#!/bin/bash
# -----------------------------------------------------------------------------
# The script can be used to test LRRST or LQRT correcness of using rea02 or
# axo03 workloads, e.g.:
#   $ ./scripts/testRangeCountQueries.sh rea02
#
# NOTE: that the testing LRRST or LQRT is hardcoded below.
#
# The script creates a temporary file (`tmp_infile`) that is used for the 
# actual executable to be tested (lrrst or lqrt). Therefore, the temporary
# file can be used in the following command to check for memory leaks with
# valgrind:
#   $ valgrind --leak-check=full ./bin/lrrst < test_data/tmp_infile > tmp_out
#
# NOTE: valgrind with axo03 can take a very looong time..

function exec() {

  datafile=$1 ;
  inFile=$2 ;
  ltree=$3 ;
  
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
  echo "-" >> ${inFile} ;
  echo "X" >> ${inFile} ;
  echo "0" >> ${inFile} ; # Maximum #deadly-points per node (0 -> 1 per corner)
  echo "${ltree}" >> ${inFile} ; # 0 -> Skyline; 1 -> Staircase
  echo "o" >> ${inFile} ;
  echo "-" >> ${inFile} ;
  qcodes="query0 query2 query3" ;
  for qc in $qcodes; do 
    echo "-" >> ${inFile};
    echo "o" >> ${inFile};
    echo "dumps/${qc}/${datafile:(-5)}" >> ${inFile};
    echo "0 0" >> ${inFile};
    echo "o" >> ${inFile};
    echo "0" >> ${inFile};
  done
  echo "." >> ${inFile} ; # Close the created tree (free memory)
  echo "q" >> ${inFile} ;
  echo "" >> ${inFile} ;
  
  ${BIN}${exe}  < ${inFile} > "${datafile}_output"
  
  # clean up (leave only ASCI dump):
  rm ${datafile}.RSF ;
  rm ${datafile}.RSF.D* ;

  echo "DONE: $datafile"
}

## MAIN PART ###

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )" ;
BIN="${DIR}/../bin/" ;

data_rea02="${DIR}/../test_data/rea02" ;
data_axo03="${DIR}/../test_data/axo03" ;
datafile="${data_rea02}" ;
exe="lrrst" ;
variant=1 ;

while [[ $# > 0 ]]
do
  key="$1"
  
  case $key in
  -d|--data-code)
    datafile="${DIR}/../test_data/$2" ;
    shift # past argument
    ;;
  -e|--executable)
    exe="$2"
    shift # past argument
    ;;
  -v|--variant)
    variant="$2"
    shift # past argument
    ;;
  *)
    # unknown option
    echo "ERROR: unknown option: $key"
    echo "Usage: testRangeQueries -d rea02/axo03 -e lrrst|lqrt"
    exit -1
    ;;
  esac
  shift # past argument or value
done

echo "Datafile: ${datafile}" ;
echo "Executable: $exe" ;
if [ ${variant} == 1 ]
then
  echo "Variant: Staircase" ;
else
  echo "Variant: Skyline" ;
fi

STARTTIME=$(date +%s) ;

infile="${DIR}/../test_data/tmp_infile" ;

exec $datafile $infile $variant;

correct=$(echo " 226935 1898604 5970038") ;
if [ "${datafile:(-5)}" == "axo03" ];
then 
  correct=$(echo " 144447 899580 903146") ;
fi
if [ "${datafile:(-5)}" == "rea03" ];
then 
  correct=$(echo " 1212412 11957960 37821960") ;
fi
echo "COR: $correct"

OUT=$(grep "rectangles found:" ${datafile}_output | cut -c19-) ;
echo "ACT: " $OUT

A=$(echo "$OUT" | xargs) ;
C=$(echo "$correct" | xargs) ;
if [ "$C" == "$A" ];
then
  echo "SUCCESS!" ;
else
  echo " ********************" ;
  echo " *** TEST FAILED! ***" ;
  echo " ********************" ;
fi

ENDTIME=$(date +%s) ;
elapsed=$(($ENDTIME - $STARTTIME)) ;
hours=$(($elapsed / 3600)) ;
mins=$(($elapsed % 3600 / 60)) ;
secs=$(($elapsed % 3600 % 60)) ;
echo "DONE at $(date '+%H:%M:%S'), duration: $(($hours))h:$(($mins))m:$(($secs))s" ;
