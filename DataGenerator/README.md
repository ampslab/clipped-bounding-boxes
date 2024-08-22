# My Notes on DataGenerator #

## Table of Contents ##

  * [Introduction](#introduction)
  * [Generating Workloads for SpatialIndexing](#generating-workloads-for-spatialindexing)

---

# Introduction #

For general install, usage, and other instructions, check the original readme files: `ReadMe.txt`, `tools/README`, `distNEW/README`, `distNEW/README.Version`, `distNEW/README.readData.c.txt`.

# Generating Workloads for SpatialIndexing #

So far I was able to generate only 1B spatial objects for *par02* and *par03* distributions.

1. Follow the steps in `distNEW/README.md` to generate 1B spatial objects.
   
   The generated binary `par03` for 1B spatial objects requires ~52 GB.

   The bellow output of `recs` for each generated file give an overview too.

```bash
$ recs 48 Insertion/par03
              length                      name             #records   remainder
-------------------- ------------------------- --------------------  ----------
         51539607552           Insertion/par03           1073741824           0
              515376           QueriesR0/par03                10737           0
              162960           QueriesR1/par03                 3395           0
               51504           QueriesR2/par03                 1073           0
               16272           QueriesR3/par03                  339           0
```

2. Convert the generated RStar binary (data and query) files to the binary format used in SpatialIndexing:

  `$ cd ~/Workspace/eHBP/SpatialIndexTrunk`
  `$ ./bin/rs2bin -i Insertion/par03 -o Insertion/par03.bin`

and queries:
  `$ for i in 0 1 2 3 ; do ./bin/rs2bin -i /media/sidlausk/Data2/1B/QueriesR${i}/par03 -o /media/sidlausk/Data1/research-data/SIW/RStar-1B/QueriesR${i}/par03.bin ; done`

Note: **rs2bin supports only 3D at the moment!**

3. *(Optional)* the converted binary files can be visualized as follows:

  `$ ./bin/VisDataFiles -d Insertion/par03.bin --vtk -c blue -p 0.01`

The above command scans entire data file and visualizes only 0.01 % of it.

Since #queries is much smaller, all of them can be visualized:
  `$ QR=0; for i in abs bit dia par ped pha rea uni ; do ./bin/VisDataFiles -i /media/sidlausk/Data1/research-data/SIW/RStar-original/QueriesR${QR}/${i}03.bin --vtk -c red ; done`

This line was used to create visualization showing the order/sequence of generated data 
  `$ ./bin/VisDataFiles -i /media/sidlausk/Data1/research-data/SIW/RStar-1B/Insertion/par03.bin --vtk -c blue -s 10000 --chunks-of 100 -p 0.01`

That is, it uniformly samples 10k spatial objects (using combination of `-s` and `-p` options) and generates visualization after each chunk of 100 objects.

### Convert the Original RStar 3D workload ###

```bash
cd ~/Workspace/eHBP/SpatialIndexTrunk

for i in abs bit dia par ped pha rea uni ; do ./bin/rs2bin -i /media/sidlausk/Data1/research-data/rrstar-data/dumps/data/${i}03 -o /media/sidlausk/Data1/research-data/SIW/RStar-original/Insertion/${i}03.bin ; done

QR=0; for i in abs bit dia par ped pha rea uni ; do ./bin/rs2bin -i /media/sidlausk/Data1/research-data/rrstar-data/dumps/query${QR}/${i}03 -o /media/sidlausk/Data1/research-data/SIW/RStar-original/QueriesR${QR}/${i}03.bin ; done

QR=1; for i in abs bit dia par ped pha rea uni ; do ./bin/rs2bin -i /media/sidlausk/Data1/research-data/rrstar-data/dumps/query${QR}/${i}03 -o /media/sidlausk/Data1/research-data/SIW/RStar-original/QueriesR${QR}/${i}03.bin ; done

QR=2; for i in abs bit dia par ped pha rea uni ; do ./bin/rs2bin -i /media/sidlausk/Data1/research-data/rrstar-data/dumps/query${QR}/${i}03 -o /media/sidlausk/Data1/research-data/SIW/RStar-original/QueriesR${QR}/${i}03.bin ; done

QR=3; for i in abs bit dia par ped pha rea uni ; do ./bin/rs2bin -i /media/sidlausk/Data1/research-data/rrstar-data/dumps/query${QR}/${i}03 -o /media/sidlausk/Data1/research-data/SIW/RStar-original/QueriesR${QR}/${i}03.bin ; done

for i in abs bit dia par ped pha rea uni ; do ./bin/VisDataFiles -i /media/sidlausk/Data1/research-data/SIW/RStar-original/Insertion/${i}03.bin --vtk -p 10 -c blue ; done

QR=0; for i in abs bit dia par ped pha rea uni ; do ./bin/VisDataFiles -i /media/sidlausk/Data1/research-data/SIW/RStar-original/QueriesR${QR}/${i}03.bin --vtk -c red ; done

QR=1; for i in abs bit dia par ped pha rea uni ; do ./bin/VisDataFiles -i /media/sidlausk/Data1/research-data/SIW/RStar-original/QueriesR${QR}/${i}03.bin --vtk -c red ; done

QR=2; for i in abs bit dia par ped pha rea uni ; do ./bin/VisDataFiles -i /media/sidlausk/Data1/research-data/SIW/RStar-original/QueriesR${QR}/${i}03.bin --vtk -c red ; done

QR=3; for i in abs bit dia par ped pha rea uni ; do ./bin/VisDataFiles -i /media/sidlausk/Data1/research-data/SIW/RStar-original/QueriesR${QR}/${i}03.bin --vtk -c red ; done
```



