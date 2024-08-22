#!/bin/bash

tools=/home/sidlausk/git/l-tree/DataGenerator/tools

if [ "$#" -eq 0 ]
then
  echo "Error: No args passed!" ;
  exit 2 ;
fi

inFile=$1

# convert to double
${tools}/asc2doub ${inFile} ${inFile}.bin

# visualize the binary file
${tools}/vidi ${inFile}.bin -124.406566 -114.133647  32.540257 42.009240 -m 0,0 -w960 -b



