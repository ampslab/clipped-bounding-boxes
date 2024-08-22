# make several tools

ifeq ($(OS),Windows_NT)
  dither = dither.exe
  marea = marea.exe
  mbb = mbb.exe
  proj = proj.exe
  cpeach = cpeach.exe
  recs = recs.exe
  rectpar = rectpar.exe
  fdpd = fdpd.exe
  vidi = vidi.exe
  asc2doub = asc2doub.exe
  doub2asc = doub2asc.exe
  randsegs = randsegs.exe
  gdNN = gdNN.exe
  cmpsegs = cmpsegs.exe
  
  ditherf = ditherf.exe
  mareaf = mareaf.exe
  mbbf = mbbf.exe
  projf = projf.exe
  vidif = vidif.exe
  vidiCE = vidiCE.exe
  vidiTxt = vidiTxt.exe
else
  dither = dither
  marea = marea
  mbb = mbb
  proj = proj
  cpeach = cpeach
  recs = recs
  rectpar = rectpar
  fdpd = fdpd
  vidi = vidi
  asc2doub = asc2doub
  doub2asc = doub2asc
  randsegs = randsegs
  gdNN = gdNN
  cmpsegs = cmpsegs
  
  ditherf = ditherf
  mareaf = mareaf
  mbbf = mbbf
  projf = projf
  vidif = vidif
  vidiCE = vidiCE
  vidiTxt = vidiTxt
endif

##### pointerSize #####
# demand m=<bitSize> argument:
# m = UNDEFINED
# set default for bitSize:
m = 64
##### compiler options: #####
# ### sun: ###
# # debugging:
# # CC = cc -m${m} -v -g
# ##optimized:
# # WARN:
# # CC = cc -fast -m${m} -v
# # WARN as above, but DO NOT WARN ABOUT VOID PTR IN ARITHMETIC:
# CC = cc -fast -m${m} -v -erroff=E_PTR_TO_VOID_IN_ARITHMETIC
### gnu: ###:
# debugging:
# CC = gcc -m${m} -Wall -g
##optimized:
# WARN:
# CC = gcc -O3 -m$(m) -Wall -Winline -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Werror-implicit-function-declaration
# WARN as above, but DO NOT MAKE PROPOSALS:
CC = gcc -O3 -m$(m) -Wall -Winline -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Werror-implicit-function-declaration -Wno-parentheses


dir =
programs = ${dir}$(dither) \
           ${dir}$(marea) \
           ${dir}$(mbb) \
           ${dir}$(proj) \
           ${dir}$(cpeach) \
           ${dir}$(recs) \
           ${dir}$(rectpar) \
           ${dir}$(fdpd) \
           ${dir}$(vidi) \
           ${dir}$(asc2doub) \
           ${dir}$(doub2asc) \
           ${dir}$(randsegs) \
           ${dir}$(gdNN) \
           ${dir}$(cmpsegs)
           
explicitly = ${dir}$(ditherf) \
             ${dir}$(mareaf) \
             ${dir}$(mbbf) \
             ${dir}$(projf) \
             ${dir}$(vidif) \
             ${dir}$(vidiCE) \
             $(dir)$(vidiTxt)

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
clean:
	rm -f *.o
	cd rrst&& $(MAKE) m=$(m) clean
rmprgs:
	rm -f $(programs)
	rm -f $(explicitly)
	rm -f *.o
	cd rrst&& $(MAKE) m=$(m) clean
recomp:
	rm -f $(programs)
	rm -f $(explicitly)
	rm -f *.o
	cd rrst&& $(MAKE) m=$(m) recomp


#### programs ####
### dither ###
${dir}$(dither): dither.c $(DEP_PltfHelp)
	${CC} -Dtypatom=double -o ${dir}$(dither) dither.c $(OPT_PltfHelp)

${dir}$(ditherf): dither.c $(DEP_PltfHelp)
	${CC} -Dtypatom=float -o ${dir}$(ditherf) dither.c $(OPT_PltfHelp)

### marea ###
${dir}$(marea): marea.c $(DEP_PltfHelp)
	${CC} -Dtypatom=double -o ${dir}$(marea) marea.c $(OPT_PltfHelp)

${dir}$(mareaf): marea.c $(DEP_PltfHelp)
	${CC} -Dtypatom=float -o ${dir}$(mareaf) marea.c $(OPT_PltfHelp)

### mbb ###
${dir}$(mbb): mbb.c $(DEP_PltfHelp)
	${CC} -Dtypatom=double -o ${dir}$(mbb) mbb.c $(OPT_PltfHelp)

${dir}$(mbbf): mbb.c $(DEP_PltfHelp)
	${CC} -Dtypatom=float -o ${dir}$(mbbf) mbb.c $(OPT_PltfHelp)

### proj ###
${dir}$(proj): proj.c $(DEP_PltfHelp)
	${CC} -Dtypatom=double -o ${dir}$(proj) proj.c $(OPT_PltfHelp)

${dir}$(projf): proj.c $(DEP_PltfHelp)
	${CC} -Dtypatom=float -o ${dir}$(projf) proj.c $(OPT_PltfHelp)

### cpeach ###
${dir}$(cpeach): cpeach.c $(DEP_UTIL)
	${CC} -o ${dir}$(cpeach) cpeach.c $(OPT_UTIL)

### recs ###
${dir}$(recs): recs.c $(DEP_PltfHelp)
	${CC} -o ${dir}$(recs) recs.c $(OPT_PltfHelp)

### rectpar ###
${dir}$(rectpar): rectpar.c $(DEP_RRST)
	${CC} -o ${dir}$(rectpar) rectpar.c $(OPT_RRST)

### fdpd ###
${dir}$(fdpd): fdpd.c $(DEP_PltfHelp)
	${CC} -o ${dir}$(fdpd) fdpd.c $(OPT_PltfHelp)

### vidi ###
${dir}$(vidi): vidi.c $(DEP_PltfHelp)
ifeq ($(OS),Windows_NT)
	@echo
	@echo '---------- Makefile: ------------------------------------------'
	@echo 'vidi - depending on X11, Xt, Xm - not being created on Windows!'
	@echo '---------------------------------------------------------------'
	@echo
else
	${CC} -Dtypcoord=double -o ${dir}$(vidi) vidi.c $(OPT_PltfHelp) -lXm -lXt -lX11
endif
#  ## vidif: vidi.c with -Dtypcoord=float version
${dir}$(vidif): vidi.c $(DEP_PltfHelp)
ifeq ($(OS),Windows_NT)
	@echo
	@echo '---------- Makefile: ------------------------------------------'
	@echo 'vidif - depending on X11, Xt, Xm - not being created on Windows!'
	@echo '---------------------------------------------------------------'
	@echo
else
	${CC} -Dtypcoord=float -o ${dir}$(vidif) vidi.c $(OPT_PltfHelp) -lXm -lXt -lX11
endif
#  ## vidiCE: (center, extension) version
${dir}$(vidiCE): vidiCE.c $(DEP_PltfHelp)
ifeq ($(OS),Windows_NT)
	@echo
	@echo '---------- Makefile: ------------------------------------------'
	@echo 'vidiCE - depending on X11, Xt, Xm - not being created on Windows!'
	@echo '---------------------------------------------------------------'
	@echo
else
	${CC} -Dtypcoord=double -o ${dir}$(vidiCE) vidiCE.c $(OPT_PltfHelp) -lXm -lXt -lX11
endif
#  ## vidiTxt: (text input) version
${dir}$(vidiTxt): vidiTxt.c $(DEP_PltfHelp)
ifeq ($(OS),Windows_NT)
	@echo
	@echo '---------- Makefile: ------------------------------------------'
	@echo 'vidiTxt - depending on X11, Xt, Xm - not being created on Windows!'
	@echo '---------------------------------------------------------------'
	@echo
else
	${CC} -Dtypcoord=double -o ${dir}$(vidiTxt) vidiTxt.c $(OPT_PltfHelp) -lXm -lXt -lX11
endif

### asc2doub ###
${dir}$(asc2doub): asc2doub.c $(DEP_PltfHelp)
	${CC} -o ${dir}$(asc2doub) asc2doub.c $(OPT_PltfHelp)

### doub2asc ###
${dir}$(doub2asc): doub2asc.c $(DEP_PltfHelp)
	${CC} -o ${dir}$(doub2asc) doub2asc.c $(OPT_PltfHelp)

### randsegs ###
${dir}$(randsegs): randsegs.c $(DEP_UTIL)
	${CC} -o ${dir}$(randsegs) randsegs.c $(OPT_UTIL)

### gdNN ###
${dir}$(gdNN): gdNN.c $(DEP_RRST)
	${CC} -o ${dir}$(gdNN) gdNN.c $(OPT_RRST)

### cmpsegs ###
${dir}$(cmpsegs): cmpsegs.c $(DEP_UTIL)
	$(CC) -o ${dir}$(cmpsegs) cmpsegs.c $(OPT_UTIL)

MultiDimIndex.$(m).o: MultiDimIndex.c \
                      MultiDimIndex.h \
                      $(DEP_PltfHelp)
	${CC} -c MultiDimIndex.c -o MultiDimIndex.$(m).o $(OPT_PltfHelp)

rrst/libRRST.${m}.a: FORCE
	cd rrst&& $(MAKE) m=${m} libRRST.${m}.a

util/libUTIL.${m}.a: FORCE
	cd util&& $(MAKE) m=${m} libUTIL.${m}.a

util/libPltfHelp.${m}.a: FORCE
	cd util&& $(MAKE) m=${m} libPltfHelp.${m}.a

FORCE:

