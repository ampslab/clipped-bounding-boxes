# Makefile for distribution producers, NOT for the distributions itself!
# Distributions are made by "createall".
# Make of programs possible.
# BUT CAUTION: other Makefiles depend on programs made here.
# As a consequence createall may later rebuilt the complete test environment
# which may take many hours!!

## necessary command line parameters:
# dim
#
##### pointerSize #####
# demand m=<bitSize> argument:
# m = UNDEFINED
# set default for bitSize
m = 64
# NOTE that these (and possible flags (e.g. -n) are retained in $(MAKEFLAGS).
# The parameters of MAKEFLAGS are AUTOMATICALLY exported to any command or
# shell invoked in a makefile. Thereby assigned parameters may be re-assigned
# in the command-line of the invokation and in the makefile itself:
# --> MAKEFLAGS re-assigned by makefile re-assigned by command-line!

#sunC:
# debugging:
# Cflags = -v -g
# profiling:
# Cflags = -fast -v -xpg
# optimized:
# Cflags = -fast -v
# CC = cc
#gnuC:
# debugging:
# Cflags = -Wall -g
# profiling:
# Cflags = -O3 -m$(m) -Wall -Winline -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Werror-implicit-function-declaration -fno-strict-aliasing -Wno-parentheses -Wno-uninitialized -pg
# optimized:
Cflags = -O3 -m$(m) -Wall -Winline -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Werror-implicit-function-declaration -fno-strict-aliasing -Wno-parentheses -Wno-uninitialized
CC = gcc

comp = $(CC) $(Cflags) -DNumbOfDim=$(dim) -o
compDimFree = $(CC) $(Cflags) -o

CDs = CDabs$(dim) CDbit$(dim) CDdia$(dim) \
      CDpar$(dim) CDped$(dim) CDpha$(dim) \
      CDuni$(dim)
MDs = MDapr$(dim) MDrpr$(dim) MDmpr$(dim) \
      MDsuc$(dim)
conv2D = 2Dar2dr 2Ddr2ar
convNDpoints = ap2dr nap2dr
programs = $(CDs) $(MDs) $(conv2D) $(convNDpoints) \
           xchbytes ckrctcns rstplain$(dim)

# depends on RRST:
DEP_RRST = rrst/libRRST.$(m).a \
           util/libUTIL.$(m).a
# options for RRST (+math):
OPT_RRST = -I rrst -I rtcom -I util -L/usr/lib -Lrrst -lRRST.$(m) -Lutil -lUTIL.$(m) -lm
# depends on utilities:
DEP_UTIL = util/libUTIL.$(m).a
# options for utilities (+math):
OPT_UTIL = -I util -L/usr/lib -Lutil -lUTIL.$(m) -lm
# depends on PltfHelp
DEP_PltfHelp = util/libPltfHelp.$(m).a
# options for PltfHelp (+math)
OPT_PltfHelp = -I util -L/usr/lib -Lutil -lPltfHelp.$(m) -lm


all: $(programs)
CD: $(CDs)
MD: $(MDs)
clean:
	rm -f *.o
	cd rrst&& $(MAKE) m=$(m) clean
rmprgs:
	rm -f $(programs) *.exe
	rm -f *.o
	cd rrst&& $(MAKE) m=$(m) clean
recomp:
	rm -f $(programs) *.exe
	rm -f *.o
	cd rrst&& $(MAKE) m=$(m) recomp


CDabs$(dim): CDabs.c $(DEP_PltfHelp)
	$(comp) CDabs$(dim) CDabs.c $(OPT_PltfHelp)

CDbit$(dim): CDbit.c $(DEP_PltfHelp)
	$(comp) CDbit$(dim) CDbit.c $(OPT_PltfHelp)

CDdia$(dim): CDdia.c $(DEP_PltfHelp)
	$(comp) CDdia$(dim) CDdia.c $(OPT_PltfHelp)

CDpar$(dim): CDpar.c $(DEP_PltfHelp)
	$(comp) CDpar$(dim) CDpar.c $(OPT_PltfHelp)

CDped$(dim): CDped.c $(DEP_PltfHelp)
	$(comp) CDped$(dim) CDped.c $(OPT_PltfHelp)

CDpha$(dim): CDpha.c $(DEP_PltfHelp)
	$(comp) CDpha$(dim) CDpha.c $(OPT_PltfHelp)

CDuni$(dim): CDuni.c $(DEP_PltfHelp)
	$(comp) CDuni$(dim) CDuni.c $(OPT_PltfHelp)

MDapr$(dim): MDapr.c $(DEP_RRST)
	$(comp) MDapr$(dim) MDapr.c $(OPT_RRST)

MDrpr$(dim): MDrpr.c $(DEP_RRST)
	$(comp) MDrpr$(dim) MDrpr.c $(OPT_RRST)

MDmpr$(dim): MDmpr.c $(DEP_RRST)
	$(comp) MDmpr$(dim) MDmpr.c $(OPT_RRST)

MDsuc$(dim): MDsuc.c $(DEP_RRST)
	$(comp) MDsuc$(dim) MDsuc.c $(OPT_RRST)

2Dar2dr: 2Dar2dr.c $(DEP_PltfHelp)
	$(compDimFree) 2Dar2dr 2Dar2dr.c $(OPT_PltfHelp)

2Ddr2ar: 2Ddr2ar.c $(DEP_PltfHelp)
	$(compDimFree) 2Ddr2ar 2Ddr2ar.c $(OPT_PltfHelp)

ap2dr: ap2dr.c $(DEP_PltfHelp)
	$(compDimFree) ap2dr ap2dr.c $(OPT_PltfHelp)

nap2dr: nap2dr.c $(DEP_PltfHelp)
	$(compDimFree) nap2dr nap2dr.c $(OPT_PltfHelp)

xchbytes: xchbytes.c $(DEP_PltfHelp)
	$(compDimFree) xchbytes xchbytes.c $(OPT_PltfHelp)

ckrctcns: ckrctcns.c $(DEP_PltfHelp)
	$(compDimFree) ckrctcns ckrctcns.c $(OPT_PltfHelp)

rstplain$(dim): rstplain.c $(DEP_RRST)
	$(comp) rstplain$(dim) rstplain.c $(OPT_RRST)

rrst/libRRST.${m}.a: FORCE
	cd rrst&& $(MAKE) m=${m} libRRST.${m}.a

util/libUTIL.${m}.a: FORCE
	cd util&& $(MAKE) m=${m} libUTIL.${m}.a

util/libPltfHelp.${m}.a: FORCE
	cd util&& $(MAKE) m=${m} libPltfHelp.${m}.a

FORCE:

