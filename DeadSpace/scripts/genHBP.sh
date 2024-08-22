#!/bin/bash
# -----------------------------------------------------------------------------

# commands I used to generate HBP data in RStar format

cd /home/sidlausk/Workspace/eHBP/SpatialIndexTrunk ;

# Generate dataset (in original binary form) together with visualization files
# This is v2.0:
#   extracts all axons and dendrites from testcircuit.
#   suits better queries inside semidense area    
./scripts/genRealData.sh -a mbr -t dendrite -n 1288251 -o ~/git/l-tree/DeadSpace/
./scripts/genRealData.sh -a mbr -t axon -n 2570016 -o ~/git/l-tree/DeadSpace/
./scripts/genRealData.sh -a mbr -t segment -n 3858267 -o ~/git/l-tree/DeadSpace/ # the above two in one

# convert them to text
./bin/ConvertToText -i ~/git/l-tree/DeadSpace/testcircuit-axon-mbr-2570016.bin \
                    -o ~/git/l-tree/DeadSpace/testcircuit-axon-mbr-2570016.bin.txt
./bin/ConvertToText -i ~/git/l-tree/DeadSpace/testcircuit-dendrite-mbr-1288251.bin \
                    -o ~/git/l-tree/DeadSpace/testcircuit-dendrite-mbr-1288251.bin.txt
./bin/ConvertToText -i ~/git/l-tree/DeadSpace/testcircuit-segment-mbr-3858267.bin \
                    -o ~/git/l-tree/DeadSpace/testcircuit-segment-mbr-3858267.bin.txt

# Convert to two-column format that is understood by RStar tools
# NOTE: DeadSpace should be compiled with "NUM_DIMS 3"
cd /home/sidlausk/git/l-tree/DeadSpace
./bin/txt2RS -i testcircuit-axon-mbr-2570016.bin.txt -o testcircuit-axon-mbr-2570016.bin.rs -v
./bin/txt2RS -i testcircuit-dendrite-mbr-1288251.bin.txt -o testcircuit-dendrite-mbr-1288251.bin.rs -v
./bin/txt2RS -i testcircuit-segment-mbr-3858267.bin.txt -o testcircuit-segment-mbr-3858267.bin.rs -v
# many 100K datasets are here:
dir="/media/sidlausk/Data2/ai_data/ai-workload-v2/"
./release/txt2RS -i ${dir}100K-axon-mbr-50000000.txt -o ${dir}100K-axon-mbr-50000000.txt.rs -v

# finally convert to the supported format using RSTree tools:
export PATH=/home/sidlausk/git/l-tree/DataGenerator/tools:$PATH # RSTree toolkit
asc2doub ~/git/l-tree/DeadSpace/testcircuit-axon-mbr-2570016.bin.rs /home/sidlausk/git/l-tree/DeadSpace/axo03
asc2doub ~/git/l-tree/DeadSpace/testcircuit-dendrite-mbr-1288251.bin.rs /home/sidlausk/git/l-tree/DeadSpace/den03
asc2doub ~/git/l-tree/DeadSpace/testcircuit-segment-mbr-3858267.bin.rs /home/sidlausk/git/l-tree/DeadSpace/neu03

# can verify MBB (need to compare the output manually):
mbb 3 /home/sidlausk/git/l-tree/DeadSpace/axo03
mbb 3 /home/sidlausk/git/l-tree/DeadSpace/den03
mbb 3 /home/sidlausk/git/l-tree/DeadSpace/neu03

# move result files to the destination dir:
cd ~/git/l-tree/DeadSpace/ ;
mv axo03 /home/sidlausk/git/l-tree/L-tree/dumps/data/ ;
mv den03 /home/sidlausk/git/l-tree/L-tree/dumps/data/ ;
mv neu03 /home/sidlausk/git/l-tree/L-tree/dumps/data/ ;

## ------------------------- GENERATE QUERIES: ------------------------------ ##
# Also new v2.0: for the demidense area in testcircuit
#
# Initial the three query sizes (the three selectivities) were as 
# follows (-v option):
#     low: -v 0.000001
#  medium: -v 0.00001
#    high: -v 0.0001
#
# UPDATE: v4.0 does not use these queries! All query profiles are generated
#         using the RStar generator. 

## These are 4 query profiles (sizes) for neu03 dataset: ##
cd ~/Workspace/eHBP/SpatialIndexTrunk
./scripts/genQueries4STITCH.sh -n 100000 -d testcircuit -r semidense -v 0.0000001 &&\
./scripts/genQueries4STITCH.sh -n 100000 -d testcircuit -r semidense -v 0.000001 &&\
./scripts/genQueries4STITCH.sh -n 10000 -d testcircuit -r semidense -v 0.00002 &&\
./scripts/genQueries4STITCH.sh -n 1000 -d testcircuit -r semidense -v 0.0002
mv -v testcircuit_1000* ~/git/l-tree/DeadSpace/
cd ~/git/l-tree/DeadSpace/
# remove the first '0' in each file (side effect of current generator):
tail -n +2 testcircuit_100000_square_semidense_queries_0.0000001.txt > testcircuit_100000_square_semidense_queries_0.0000001.rs &&\
tail -n +2 testcircuit_100000_square_semidense_queries_0.000001.txt > testcircuit_100000_square_semidense_queries_0.000001.rs &&\
tail -n +2 testcircuit_10000_square_semidense_queries_0.00002.txt > testcircuit_10000_square_semidense_queries_0.00002.rs &&\
tail -n +2 testcircuit_1000_square_semidense_queries_0.0002.txt > testcircuit_1000_square_semidense_queries_0.0002.rs
# convert to two-column text file:
./bin/txt2RS -i testcircuit_100000_square_semidense_queries_0.0000001.rs -o query0.rs &&\
./bin/txt2RS -i testcircuit_100000_square_semidense_queries_0.000001.rs -o query1.rs &&\
./bin/txt2RS -i testcircuit_10000_square_semidense_queries_0.00002.rs -o query2.rs &&\
./bin/txt2RS -i testcircuit_1000_square_semidense_queries_0.0002.rs -o query3.rs
# finally, convert to the binary files using RStar tools:
asc2doub query0.rs ../L-tree/dumps/query0/neu03 &&\
asc2doub query1.rs ../L-tree/dumps/query1/neu03 &&\
asc2doub query2.rs ../L-tree/dumps/query2/neu03 &&\
asc2doub query3.rs ../L-tree/dumps/query3/neu03


## These are 4 query profiles (sizes) for den03 dataset: ##
cd ~/Workspace/eHBP/SpatialIndexTrunk
./scripts/genQueries4STITCH.sh -n 100000 -d testcircuit -r semidense -v 0.0000004 &&\
./scripts/genQueries4STITCH.sh -n 100000 -d testcircuit -r semidense -v 0.000002 &&\
./scripts/genQueries4STITCH.sh -n 10000 -d testcircuit -r semidense -v 0.00006 &&\
./scripts/genQueries4STITCH.sh -n 1000 -d testcircuit -r semidense -v 0.0005
mv -v testcircuit_1000* ~/git/l-tree/DeadSpace/
cd ~/git/l-tree/DeadSpace/
# remove the first '0' in each file (side effect of current generator):
tail -n +2 testcircuit_100000_square_semidense_queries_0.0000004.txt > testcircuit_100000_square_semidense_queries_0.0000004.rs &&\
tail -n +2 testcircuit_100000_square_semidense_queries_0.000002.txt > testcircuit_100000_square_semidense_queries_0.000002.rs &&\
tail -n +2 testcircuit_10000_square_semidense_queries_0.00006.txt > testcircuit_10000_square_semidense_queries_0.00006.rs &&\
tail -n +2 testcircuit_1000_square_semidense_queries_0.0005.txt > testcircuit_1000_square_semidense_queries_0.0005.rs
# convert to two-column text file:
./bin/txt2RS -i testcircuit_100000_square_semidense_queries_0.0000004.rs -o query0.rs &&\
./bin/txt2RS -i testcircuit_100000_square_semidense_queries_0.000002.rs -o query1.rs &&\
./bin/txt2RS -i testcircuit_10000_square_semidense_queries_0.00006.rs -o query2.rs &&\
./bin/txt2RS -i testcircuit_1000_square_semidense_queries_0.0005.rs -o query3.rs
# finally, convert to the binary files using RStar tools:
asc2doub query0.rs ../L-tree/dumps/query0/den03 &&\
asc2doub query1.rs ../L-tree/dumps/query1/den03 &&\
asc2doub query2.rs ../L-tree/dumps/query2/den03 &&\
asc2doub query3.rs ../L-tree/dumps/query3/den03


## These are 4 query profiles (sizes) for axo03 dataset: ##
cd ~/Workspace/eHBP/SpatialIndexTrunk
./scripts/genQueries4STITCH.sh -n 100000 -d testcircuit -r semidense -v 0.0000002 &&\
./scripts/genQueries4STITCH.sh -n 100000 -d testcircuit -r semidense -v 0.000001 &&\
./scripts/genQueries4STITCH.sh -n 10000 -d testcircuit -r semidense -v 0.00003 &&\
./scripts/genQueries4STITCH.sh -n 1000 -d testcircuit -r semidense -v 0.0003
mv -v testcircuit_1000* ~/git/l-tree/DeadSpace/
cd ~/git/l-tree/DeadSpace/
# remove the first '0' in each file (side effect of current generator):
tail -n +2 testcircuit_100000_square_semidense_queries_0.0000002.txt > testcircuit_100000_square_semidense_queries_0.0000002.rs &&\
tail -n +2 testcircuit_100000_square_semidense_queries_0.000001.txt > testcircuit_100000_square_semidense_queries_0.000001.rs &&\
tail -n +2 testcircuit_10000_square_semidense_queries_0.00003.txt > testcircuit_10000_square_semidense_queries_0.00003.rs &&\
tail -n +2 testcircuit_1000_square_semidense_queries_0.0003.txt > testcircuit_1000_square_semidense_queries_0.0003.rs
# convert to two-column text file:
./bin/txt2RS -i testcircuit_100000_square_semidense_queries_0.0000002.rs -o query0.rs &&\
./bin/txt2RS -i testcircuit_100000_square_semidense_queries_0.000001.rs -o query1.rs &&\
./bin/txt2RS -i testcircuit_10000_square_semidense_queries_0.00003.rs -o query2.rs &&\
./bin/txt2RS -i testcircuit_1000_square_semidense_queries_0.0003.rs -o query3.rs
# finally, convert to the binary files using RStar tools:
asc2doub query0.rs ../L-tree/dumps/query0/axo03 &&\
asc2doub query1.rs ../L-tree/dumps/query1/axo03 &&\
asc2doub query2.rs ../L-tree/dumps/query2/axo03 &&\
asc2doub query3.rs ../L-tree/dumps/query3/axo03

## ----------------------- END OF GENERATE QUERIES -------------------------- ##
## -------------------------------------------------------------------------- ##

# Below steps must be done for each set (of size 3) of queries:
mv testcircuit_1000* ~/git/l-tree/DeadSpace/

# Convert to text (before manually removed '0' on the first line):
cd ~/git/l-tree/DeadSpace/
#./release/txt2RS -i testcircuit_100000_square_semidense_queries_0.000001.txt -o query0.rs
./bin/txt2RS -i testcircuit_100000_square_semidense_queries_0.0000005.txt -o query0.rs
#./release/txt2RS -i testcircuit_10000_square_semidense_queries_0.00001.txt -o query2.rs
./bin/txt2RS -i testcircuit_10000_square_semidense_queries_0.00003.txt -o query2.rs
#./release/txt2RS -i testcircuit_1000_square_semidense_queries_0.0001.txt -o query3.rs
./bin/txt2RS -i testcircuit_1000_square_semidense_queries_0.0003.txt -o query3.rs

# and convert to the binary files using RStar tools:
asc2doub query0.rs ../L-tree/dumps/query0/neu03
asc2doub query2.rs ../L-tree/dumps/query2/neu03
asc2doub query3.rs ../L-tree/dumps/query3/neu03
