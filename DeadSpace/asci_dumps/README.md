## How to visualize queries stored in binary (RStar) format ##

First convert binary file to text (ASCI) file:
  `$ doub2asc 2 ../L-tree/dumps/query2/axo03 asci_dumps/QR2_axo03.rs`

Then, convert the text file to vtk-file:
  `$ ./bin/txt2vtk -i asci_dumps/QR2_axo03.rs -o asci_dumps/QR2_axo03 -a`

The `-a` option puts all query rectangles in one vtk file. In case I want to
visualize only few queries and in seperate vtk files. E.g., the first 3 queries:
  `$ head asci_dumps/QR2_axo03.rs -n 9 > asci_dumps/QR2_axo03[0-2].rs`
  `$ ./bin/txt2vtk -i asci_dumps/QR2_axo03[0-2].rs -o asci_dumps/QR2_axo03`
