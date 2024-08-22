# Step by step instructions how to prepare test data and run unit tests #

**NOTE**: rea02.RSF has been compressed to fit within github storage limits.  
  
At the moment, the only unit tests can be found in `apps/TestStaircaseDeadSpace.cc`.

A text file containing the test data must be in the following format (assuming 2D):
  `x1_low x1_high`
  `y1_low y1_high`
  `x2_low x2_high`
  `y2_low y2_high`
  `...`

In the following, we assume the PATH environment variable is set to point to
`l-tree/DataGenerator/tools/` and that we're in `cd l-tree/DeadSpace/tests`.

1. Convert the text file to the binary format understood by RSTree project:  
  `$ asc2doub test_data.txt test_data`

2. The binary file can be visualized using the RSTree tools too:
  `$ vidi test_data 0.0 8.5 0.0 8.5 -m w960 -b`
  
  *0.0 8.5 0.0 8.5* is the minimum bounding box (MBB) of the universe.
  
  Conviniently, the MBB of the data can be found:  
  `$ mbb 2 test_data`

3. Having verified that the `test_data` contains our intentions, we feed the 
  `ori_infile` file  to any of the RSTrees and get its ASCI dump, e.g., RRS:  
  `$ ../../L-tree/bin/trst < ori_infile > output.txt`

4. To visualize the constructed RR*-tree (remember to set `NUM_DIMS 2`), run:  
  `$ ../bin/visTree -v -i test_data.RSF.ASC -l 0 -a`

5. Having inspected that the expected tree has been created, we can start adding
   unit tests to ensure the correctness of DeadSpace algorithms. The intention 
   is to run the following command and have at least the major algorithms 
   tested:  
  `$ ./release/TestStaircaseDeadSpace`
  
##Generating queries##

Similar steps are needed to generate queries (from `test_data_queries.txt`):
  `$ asc2doub test_data_queries.txt test_data_queries && ../release/txt2vtk -v -i test_data_queries.txt -o vis/test_data_queries_vtk/`

## I'm feeling lucky ##

Useful commands (*when everything works!*).

Convert test data in text file and visualize:
  `$ asc2doub test_data.txt test_data && vidi test_data 0 8.5 0 8.5 -m w960 -b &`
  
Construct and dump Rtree (any variant) to text (ASCI) file, and generate vtk files:
  `$ ../../L-tree/bin/trst < ori_infile > output.txt && ../bin/visTree -v -i test_data.RSF.ASC -l 0 -a`
