#!/bin/bash
# -----------------------------------------------------------------------------

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )" ;
BIN="${DIR}/../bin/" ;
dump_dir="${DIR}/../dumps/" ;
data_dir="${dump_dir}data/hbp" ;

exe="trrst" ; # Revised R*-tree
tree_name="RRS" ;

#exe="trst" ; # R*-tree
#tree_name="RS" ;

#exe="tqrt" ; # R-tree
#tree_name="QR" ;

#exe="thrt" ; # R-tree
#tree_name="HR" ;


STARTTIME=$(date +%s) ;

for datafile in `/bin/ls -m1 ${data_dir}/`; do
  if [ "${datafile:(-2)}" != "02" ] && [ "${datafile:(-2)}" != "03" ]; then
    continue ;
  fi
  
  echo "${datafile:0:3}" ;

  if [ "${datafile:0:3}" != "50a" ]; then
    continue ;
  fi
  
#  if [ "${datafile:0:3}" == "axo" ] || [ "${datafile:0:3}" == "den" ]; then
#    continue ;
#  fi
  
  echo $datafile ;
  
  inFile="${dump_dir}inFile_${tree_name}_${datafile}" ;
  
  echo "c" > ${inFile} ;
  echo "${data_dir}/${datafile}" >> ${inFile} ;
  echo "4096" >> ${inFile} ;
  echo "4096" >> ${inFile} ;
  echo ${datafile:(-2)} >> ${inFile} ; # number of dimensions!
  echo "4" >> ${inFile} ;
  echo "n" >> ${inFile} ;
  echo "i" >> ${inFile} ;
  echo "0 0" >> ${inFile} ;
  echo "o" >> ${inFile} ;
  echo "400000" >> ${inFile} ;
  echo "I" >> ${inFile} ;
  echo "A" >> ${inFile} ;
  echo "f" >> ${inFile} ;
  echo "q" >> ${inFile} ;
  echo "" >> ${inFile} ;
  
  ${BIN}${exe}  < ${inFile} > "${dump_dir}outFile_${tree_name}_${datafile}"
  
  echo "Didn't move: ${data_dir}/${datafile}.RSF.ASC !"
#  mv ${data_dir}/${datafile}.RSF.ASC "${dump_dir}${tree_name}_${datafile}.RSF.ASC" ;
#  rm ${data_dir}/${datafile}.RSF* ; 
#  rm ${inFile}
done

ENDTIME=$(date +%s) ;
elapsed=$(($ENDTIME - $STARTTIME)) ;
hours=$(($elapsed / 3600)) ;
mins=$(($elapsed % 3600 / 60)) ;
secs=$(($elapsed % 3600 % 60)) ;
echo "DONE at $(date '+%H:%M:%S'), duration: $(($hours))h:$(($mins))m:$(($secs))s" ;



