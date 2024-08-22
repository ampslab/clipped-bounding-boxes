# Usefull notes for a quick-start with DeadSpace #

## Compile ##

Assuming, you are in `DeadSpace` directory.

```
$ mkdir bin && cd bin/
$ cmake ../
$ make
```

After the above commands, the `bin/` folder contains all executables.

*For debuging:*
```
$ mkdir debug && cd debug
$ cmake -DDEBUG_MODE=ON ../
$ make
```

Similarly, `debug/` now contains all executable (compiled with `-O0` and `-g` flags).

**Important:** All the functionality supports only 2D or 3D data. The number of dimensions
is hard-coded in `include/SpatialObjects.h:18` (`NUM_DIMS`) and thus have to be recompiled
each time to match the data dimensionality.

## Run ##

The executable `bin/cbb` is responsible for computing clipped bounding box on a
given dump of R-tree. Simply running it without any arguments will show the synopsis.

Some toy test data can be found in `tests/`.
- `tests/test_data.txt`: 7 two-dimensional rectangles in text format.
- `tests/test_data`:	 the same 7 rectangles in binary format.
- `tests/test_data.RSF.ASC`: a two level RR*-tree dump after indexing the above 7 
			     rectangles (max node capacity is 4).
- `tests/test_data.RSF.ASC.eps`: a visualization of the above RR*-tree (and data).

So, executing `./bin/cbb -i tests/test_data.RSF.ASC -v` should output something like this:

```
Nodes processed: 0k
   #nodes  avg_size  avg_#DPs   avg_mbb(%)   avg_mbc(%)    avg_ch(%)  avg_rmbb(%)   avg_4-c(%)   avg_5-c(%)   avg_cbb(%)   cbb-k=1   cbb-k=2   cbb-k=3   cbb-k=4
        3       3.0       2.3       44.810       65.002       32.261       44.264       37.674       34.638       10.871   22.8767   14.8955   12.7283   10.8707
DONE!
'-> #lines: 32
'-> #visualization files saved: 0
'-> #nodes parsed: 3
'-> #clipped: 3 (100.00%)
'-> #zero-volume CBBs: 0
DIR node [min--max] ids: [0--0]
DAT node [min--max] ids: [1--2]
All stats about each node are written to test_test_data_CBB-sta.csv
```

I guess for more details you will look at `apps/cbb.cc`.
