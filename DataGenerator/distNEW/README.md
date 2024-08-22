## How to generate new queries based HBP datasets ##

Copy HBP datasets (e.g., axo03, den03, and neu03) into `RealDistOrig`. Then run:
`./createall R 2 3 9`

Note that I commented out generation of artificial and real datasets (see `grep "DS:" createall`).

*Important:* make sure to comply with below [Quick-start]

Also, you might want to check `MakeHBP`.

## Quick-start ##

* Add to PATH the following directories:
`export PATH=../tools:$PATH`
`export PATH=./:$PATH`

* Make sure in current directory (`distNEW/`) the following folders are removed:
`rm -r Insertion QueriesP QueriesR0 QueriesR1 QueriesR2 QueriesR3`

* For artificial/synthetic datasets, run:
`./createall 2 3 9`

* To generate also real datasets (rea02, rea03, rea05, rea16, rea22, rea26):
`./createall R 2 3 9`

* To activate the original dataset generation, run:
`for i in abs bit dia par ped pha uni; do cp CD${i}.c.ORIG CD${i}.c; done`

* To clean up:
`./rmProgs`

## Generating 1'000'000'000 (billion!) objects ##
To generate 1B objects of parcel (`par02` and `par03`) you need:

1. Use the modified source file CDpar.c.1B:
`  for i in par; do`
`    cp CD${i}.c.1B CD${i}.c`
`  done`

2. Uncomment the lines in `createall` marked with *DS-1B:*.
3. Run: `createall 2 3`

Note: I get errors of *NEGATIVE EXTENSION*, though. Check `CDpar.c` for details.