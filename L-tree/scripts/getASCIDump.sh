#!/bin/bash
# -----------------------------------------------------------------------------

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )" ;
BIN="${DIR}/../bin/" ;
dump_dir="${DIR}/../dumps/" ;
data_dir="${dump_dir}data/" ;
asci_dir="${DIR}/../../DeadSpace/asci_dumps/" ;

# default values:
exe="trrst" ; # Revised R*-tree
tree_name="RRS" ;
datafile="${data_dir}rea02" ;
pagesize=4096 ;
simulate=0 ;

#exe="trst" ; # R*-tree
#tree_name="RS" ;

#exe="tqrt" ; # R-tree
#tree_name="QR" ;

#exe="thrt" ; # R-tree
#tree_name="HR" ;

while [[ $# > 0 ]]
do
  key="$1"

  case $key in
    -e|--exe)
    exe="$2"
    if [ "${exe}" == "trrst" ] || [ "${exe}" == "rrs" ]; then
      exe="trrst";
      tree_name="RRS";
    fi
    if [ "${exe}" == "trst" ] || [ "${exe}" == "rs" ]; then
      exe="trst";
      tree_name="RS";
    fi
    if [ "${exe}" == "tqrt" ] || [ "${exe}" == "qr" ]; then
      exe="tqrt";
      tree_name="QR";
    fi
    if [ "${exe}" == "thrt" ] || [ "${exe}" == "hr" ]; then
      exe="thrt";
      tree_name="HR";
    fi
    shift # past argument
    ;;
    -d|--datafile-name)
    datafile="${data_dir}$2"
    shift # past argument
    ;;
    -f|--full-datafile-path)
    datafile="$2"
    shift # past argument
    ;;
    -p|--pagesize)
    pagesize="$2"
    shift # past argument
    ;;
    -s|--simulate)
    simulate="1"
    ;;
    *)
    # unknown option
    echo "ERROR: unknown option: $key"
    exit -1
    ;;
  esac
  shift # past argument or value
done

echo " ## Generating ${tree_name}-tree ASCI dump ## "
echo "     exe: bin/${exe}"
echo "datafile: ${datafile}"
echo "    dims: ${datafile:(-2)}"
echo "pagesize: ${pagesize}"
echo "simulate: ${simulate}"

STARTTIME=$(date +%s) ;

inFile="${dump_dir}tmp_infile" ;
outFile="${dump_dir}tmp_outfile" ;
echo "c" > ${inFile} ;
echo "${datafile}" >> ${inFile} ;
echo "${pagesize}" >> ${inFile} ;
echo "${pagesize}" >> ${inFile} ;
echo ${datafile:(-2)} >> ${inFile} ; # number of dimensions!
echo "4" >> ${inFile} ;
echo "n" >> ${inFile} ;
echo "i" >> ${inFile} ;
echo "0 0" >> ${inFile} ;
echo "o" >> ${inFile} ;
echo "0" >> ${inFile} ;
echo "I" >> ${inFile} ;
echo "A" >> ${inFile} ;
echo "f" >> ${inFile} ;
echo "q" >> ${inFile} ;
echo "" >> ${inFile} ;

if [ $simulate == 1 ] ; then
  echo "SIMULATE: ${BIN}${exe}  < ${inFile} > ${outFile} " ;
else
  echo "RUNNING"
  ${BIN}${exe}  < ${inFile} > "${outFile}"
  # echo "Didn't move: ${datafile}.RSF.ASC !"
  mv ${datafile}.RSF.ASC "${asci_dir}${tree_name}_${datafile:(-5)}.RSF.ASC" ;
  echo "ASCI dump moved to: ${asci_dir}${tree_name}_${datafile:(-5)}.RSF.ASC"
  rm ${datafile}.RSF* ; 
  rm ${inFile}
  rm ${outFile}
fi

ENDTIME=$(date +%s) ;
elapsed=$(($ENDTIME - $STARTTIME)) ;
hours=$(($elapsed / 3600)) ;
mins=$(($elapsed % 3600 / 60)) ;
secs=$(($elapsed % 3600 % 60)) ;
echo "DONE at $(date '+%H:%M:%S'), duration: $(($hours))h:$(($mins))m:$(($secs))s" ;
echo ""
