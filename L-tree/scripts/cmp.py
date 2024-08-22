#!/usr/bin/env python

#
# To test using test data execute the following:
#  $ ./scripts/cmp.py -f1 test_data/rea02.RS_stair-dead.txt -f2 test_data/rea02.RSF.LTR
#  $ ./scripts/cmp.py -f1 test_data/axo03.RS_stair-dead.txt -f2 test_data/axo03.RSF.LTR
#

import argparse
import sys
import os

parser = argparse.ArgumentParser( description='Description of your program' )
parser.add_argument( '-f1','--shorter-file', help='File containing the output from DeadSpace', required=True )
parser.add_argument( '-f2','--longer-file', help='File containing the output from L-Tree', required=True )

args = parser.parse_args()
# print( args )

f1_size = os.path.getsize( args.shorter_file )
f2_size = os.path.getsize( args.longer_file )

if f1_size > f2_size:
    print( "ERROR: f1 (" + str(f1_size) + " B) > f2(" + str(f2_size) + " B)" )
    exit()

f1 = open( args.shorter_file )
f2 = open( args.longer_file )
 
line1 = next(f1)
line2 = next(f2)
 
errors = 0
corrects = 0
num_lines = 1 
while line1 and line2:
       
    line1 = line1.replace('\n', '')
    line2 = line2.replace('\n', '')
    
#     print( "Line1: " + line1 )
#     print( "Line2: " + line2 )
    if line2.startswith( line1 ):
        corrects = corrects + 1
    else:
        errors = errors + 1
        print( "Error " + str(errors) + ":" )
        print( " `-> line1: " + line1 )
        print( " `-> line2: " + line2 )
 
    line1 = next(f1, '')
    line2 = next(f2, '')
    num_lines = num_lines + 1
 
if errors == 0:
    print( "SUCCESS! (num_lines read: " + str(num_lines) + ")" )
else:
    print( "FAILURE (" + str(errors) + " in " + str(num_lines) + ")" )
   
f1.close()
f2.close()
