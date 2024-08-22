#!/bin/bash
# -----------------------------------------------------------------------------
#
# runScaQueries (run Scalability Queries)
# Script for running scalability experiments: querying already built R-trees.
#
# Let's see how my desktop machine can handle this.
# 
# Just to run lrrst (both sky and sta) on both par02 and par03 requires a
# whopping 9h:43m:31s!
# Running trrst took ~3h.
# -----------------------------------------------------------------------------

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )" ;
BIN="${DIR}/../bin/" ;

# IMPORTANT HARDCODED VALUES:
dump_dir="${DIR}/../dumps/" ;
wdir="${dump_dir}1B/" ;

# For mapping to the correct directory:
declare -A RTreeNames=( ["lrrst"]="RRST" ["trrst"]="RRST" ["lhrt"]="HRT" ["thrt"]="HRT")
# Dictionary to get original executable:
declare -A OriExe=( ["lrrst"]="trrst" ["lhrt"]="thrt" )

if [ "$#" -ne 1 ]
then
  echo "Error: Output dir for results must be given!" ;
  echo " " ;
  echo "CL used to run for VLDB'17 experiments:" ;
  echo "       sudo ./scripts/runScaQueries.sh dumps/1B/" ;
  echo " " ;
  echo "Note 'sudo' is required for dropping OS caches!!!" ;
  exit 2 ;
fi

wdir="$1" ;
echo "WORKING DIR: $wdir" ;

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

execs="lhrt lrrst" ;
variants="sta" ;

dfiles="par03" ;
qdirs="QueriesR0 QueriesR1 QueriesR2" ;

STARTTIME=$(date +%s) ;

for exe in $execs; do
  test_dir="${wdir}${RTreeNames[$exe]}" ;
  echo "test_dir: $test_dir" ;
  
  prev_dir="$(pwd)" ;
  cd ${test_dir} ;
  pwd ;
  
  for v in $variants; do
    prev_exe=$exe ;
    if [ "$v" = "ori" ]; then
      exe="${OriExe[$exe]}"  
    fi
    
    for df in $dfiles; do
      echo "QUERYING ${exe}_${v}_${df}" ;

      # Link executable, data, and query files in the test directory:
      ln -sf "${BIN}${exe}" ${exe} ;
      
      for qd in ${qdirs}; do
        qfile="${qd}_${df}" ;
        if [ ! -f ${qfile} ]; then
          echo "ERROR: missing query file (\"${qfile}\")! "
          exit 2;
        fi
      done

      # Generate the RSTree input file for this data:
      infile="${exe}_${v}_${df}" ;
      
      ingen="${DIR}/../scripts/genRSTinputfileQuerying.sh" ; # RST input file generator
      echo "Generating RSTree input files.." ;
      $ingen -e $exe -d $df -q "$qdirs" -v $v ;
      if [ ! -f ${infile} ]; then
        echo "ERROR: failed to generate RST input file!"
        exit 2;
      fi
      echo "Running: ./${exe} < ${infile} > ${infile}_Qoutput" ;
      ./${exe} < ${infile} > "${infile}_Qoutput" ;
      
      echo "" ;
    done
    exe=$prev_exe ;
  done
  cd "$prev_dir" ;
done

ENDTIME=$(date +%s) ;
elapsed=$(($ENDTIME - $STARTTIME)) ;
hours=$(($elapsed / 3600)) ;
mins=$(($elapsed % 3600 / 60)) ;
secs=$(($elapsed % 3600 % 60)) ;
echo "DONE at $(date '+%H:%M:%S'), duration: $(($hours))h:$(($mins))m:$(($secs))s" ;
