#!/bin/bash
# -----------------------------------------------------------------------------
#
# Script for running scalability experiments on server machine (diascld22).
#
# Scalability: 1B objects from par02 and par03 distros occupying ~ 30GB and
#              ~50GB of raw disk storage, respectively.
# 
# Just to run lrrst (both sky and sta) on both par02 and par03 requires a
# whopping 9h:17m:44s!
# Running trrst took ~3h.
# -----------------------------------------------------------------------------

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )" ;
BIN="${DIR}/../bin/" ;

# IMPORTANT HARDCODED VALUES:
dump_dir="${DIR}/../dumps/par_maxstep2^30/" ;
#dump_dir="${DIR}/../dumps/sigmod/" ;

if [ "$#" -ne 1 ]
then
  echo "Error: Output dir for results must be given!" ;
  exit 2 ;
fi

echo "WORKING DIR: $DIR" ;
RESDIR="$1" ;

#exe="trrst" ; # Revised R*-tree
#tree_name="RRS" ;
#exe="trst" ; # R*-tree
#tree_name="RS" ;
#exe="tqrt" ; # R-tree
#tree_name="QR" ;
#exe="thrt" ; # R-tree
#tree_name="HR" ;

# All methods:
execs="lrrst trrst thrt trst tqrt lqrt lhrt lrst" ;
#execs="lhrt lqrt lrst" ; # this took 6h to finish on dfiles="par02"

# Original trees:
#execs="trrst thrt trst tqrt" ;
#variants="ori" ;

# Stairline/Skyline clipped variants:
#execs="lrrst lqrt lhrt lrst" ;
#variants="sky sta" ;

execs="thrt" ;
variants="ori" ;

dfiles="par02 par03" ;
qdirs="QueriesR0 QueriesR1 QueriesR2" ;

STARTTIME=$(date +%s) ;

for exe in $execs; do
  for v in $variants; do
    # Create test directory and enter it:
    test_dir="${RESDIR}/${exe}_${v}"
    mkdir -p ${test_dir} ;
    prev_dir="$(pwd)" ;
    cd ${test_dir} ;

    for df in $dfiles; do
      echo "RUNNING ${exe}_${v}_${df}" ;

      # Link executable, data, and query files in the test directory:
      ln -sf "${BIN}${exe}" ${exe} ;
      ln -s "${dump_dir}Insertion/${df}" ${df} ;
      for qd in ${qdirs}; do ln -s "${dump_dir}${qd}/${df}" "${qd}_${df}" ; done

      # Generate the RSTree input file for this data:
      infile="${exe}_${v}_${df}" ;
      
      ingen="../../scripts/genRSTinputfile.sh" ; # RST input file generator
      echo "Generating RSTree input files.." ;
      $ingen -e $exe -d $df -q "$qdirs" -v $v ; 
      
      if [ ! -f ${infile} ]; then
        echo "ERROR: failed to generate RST input file!"
        exit 2;
      fi
      echo "Running: ./${exe} < ${infile} > ${infile}_output" ;
      ./${exe} < ${infile} > "${infile}_output" ;
  
#      mv $infile ${RESDIR}/ ;
#      mv ${infile}_output ${RESDIR}/ ;
      
      # clean-up
      # rm ${data_dir}/${df}.RSF* ; 
      echo "" ;
    done

    cd "$prev_dir" ;
  done
done

ENDTIME=$(date +%s) ;
elapsed=$(($ENDTIME - $STARTTIME)) ;
hours=$(($elapsed / 3600)) ;
mins=$(($elapsed % 3600 / 60)) ;
secs=$(($elapsed % 3600 % 60)) ;
echo "DONE at $(date '+%H:%M:%S'), duration: $(($hours))h:$(($mins))m:$(($secs))s" ;
