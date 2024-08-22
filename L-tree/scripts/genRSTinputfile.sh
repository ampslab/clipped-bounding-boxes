#!/bin/bash
# -----------------------------------------------------------------------------
# The script generate (script) input file for the (automatic) execution of any
# of RSTree variants. E.g.:
#   $ ./genRSTinputfile.sh lrrst axo03 "query0 query1 query2 query3" sta
#

function printFile() {
 
  infilename="${1}" ;
  
  echo "c" > ${infilename} ;
  echo "${ddir}${dfile}" >> ${infilename} ;
  echo $PAGESIZE >> ${infilename} ;
  echo $PAGESIZE >> ${infilename} ;
  echo ${dims} >> ${infilename} ;
  echo "4" >> ${infilename} ;
  echo "n" >> ${infilename} ;
  echo "i" >> ${infilename} ;
  echo "0 0" >> ${infilename} ;
  echo "o" >> ${infilename} ;
  echo "0" >> ${infilename} ;
  echo "I" >> ${infilename} ;
  if [ "$variant" = "ori" ]; then
    for qc in $qdirs; do 
      echo "r" >> ${infilename} ;
      echo "${qc}_${dfile:(-5)}" >> ${infilename} ;
      echo "0 0" >> ${infilename} ;
      echo "o" >> ${infilename} ;
      echo "0" >> ${infilename} ;
      echo "-" >> ${infilename} ;
    done  
      
    echo "."  >> ${infilename} ;
    echo "-"  >> ${infilename} ;
    echo "q"  >> ${infilename} ;
    
  else
    
    echo "-" >> ${infilename} ;
    echo "X" >> ${infilename} ;
    echo "${dpoints}" >> ${infilename} ; # Maximum #deadly-points per node (0 -> 1 per corner)
    if [ "$variant" = "sta" ]; then     # 0 -> Skyline; 1 -> Staircase
      echo "1" >> ${infilename} ;
    else
      echo "0" >> ${infilename} ;
    fi
    echo "o" >> ${infilename} ;
    echo "-" >> ${infilename} ;
    for qc in $qdirs; do 
      echo "-" >> ${infilename};
      echo "o" >> ${infilename};
      echo "${qc}_${dfile:(-5)}" >> ${infilename};
      echo "0 0" >> ${infilename};
      echo "o" >> ${infilename};
      echo "0" >> ${infilename};
    done
    echo "." >> ${infilename} ; # Close the created tree (free memory)
    echo "q" >> ${infilename} ;
  fi
  
  echo "GENERATED: ${infilename}" ;
}

# MAIN PART #

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )" ;
BIN="${DIR}/../bin/" ;

# DEFAULT VALUES
exe="lrrst" ;
dfile="axo03" ;
ddir="" ; # DATA DIRECTORY
PAGESIZE=4096 ;
DFAC=2 ; # #dpoints = corners x 2

while [[ $# > 0 ]]
do
  key="$1"
  
  case $key in
  -D|--data-dir)
    ddir="$2/" ;
    shift # past argument
    ;;
  -d|--datafile)
    dfile="$2"
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
  -q|--query-dirs)
    qdirs="$2"
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

dims=$( echo ${dfile:(-1)} ) ; # extract #dimensions
dpoints=$( echo 2^$dims*$DFAC | bc )

echo "exe: $exe"
echo "dfile: $dfile"
echo "qdirs: $qdirs"
echo "variant: $variant"
echo "dims: $dims"
echo "#dpoints: ${dpoints}"
echo "pagesize: $PAGESIZE bytes"

printFile "${exe}_${variant}_${dfile}" ;
