CODE_DIR = src
UTIL_DIR=$(CODE_DIR)/util
RRST_DIR=$(CODE_DIR)/rrst
HRT_DIR=$(CODE_DIR)/hrt
RST_DIR=$(CODE_DIR)/rst
QRT_DIR=$(CODE_DIR)/qrt

# New: our L-tree based variants
LRRST_DIR=$(CODE_DIR)/lrrst
LQRT_DIR=$(CODE_DIR)/lqrt
LHRT_DIR=$(CODE_DIR)/lhrt
LRST_DIR=$(CODE_DIR)/lrst

m = 64

RM = rm -f

.PHONY: util rrst hrt rst qrt lrrst lqrt lhrt lrst

all: util rrst hrt rst qrt lrrst lqrt lhrt lrst

util:
	$(MAKE) -C $(UTIL_DIR)
	cp $(UTIL_DIR)/*.$(m).a lib/

clean_util:
	$(MAKE) -C $(UTIL_DIR) clean
	$(RM) lib/libUTIL.*
	$(RM) lib/libPltfHelp.*
	$(RM) $(UTIL_DIR)/*.$(m).a

rrst:
	$(MAKE) -C $(RRST_DIR)
	cp $(RRST_DIR)/*.$(m).a lib/
	mv $(RRST_DIR)/trrst bin/

clean_rrst:
	$(MAKE) -C $(RRST_DIR) clean
	$(RM) bin/trrst
	$(RM) lib/libRRST.*
	$(RM) $(RRST_DIR)/*.$(m).a

hrt:
	$(MAKE) -C $(HRT_DIR)
	cp $(HRT_DIR)/*.$(m).a lib/
	mv $(HRT_DIR)/thrt bin/

clean_hrt:
	$(MAKE) -C $(HRT_DIR) clean
	$(RM) bin/thrt
	$(RM) lib/libHRT.*
	$(RM) $(HRT_DIR)/*.$(m).a

rst:
	$(MAKE) -C $(RST_DIR)
	cp $(RST_DIR)/*.$(m).a lib/
	mv $(RST_DIR)/trst bin/

clean_rst:
	$(MAKE) -C $(RST_DIR) clean
	$(RM) bin/trst
	$(RM) lib/libRST.*
	$(RM) $(RST_DIR)/*.$(m).a

qrt:
	$(MAKE) -C $(QRT_DIR)
	cp $(QRT_DIR)/*.$(m).a lib/
	mv $(QRT_DIR)/tqrt bin/

clean_qrt:
	$(MAKE) -C $(QRT_DIR) clean
	$(RM) bin/tqrt
	$(RM) lib/libQRT.*
	$(RM) $(QRT_DIR)/*.$(m).a

lqrt:
	$(MAKE) -C $(LQRT_DIR)
	cp $(LQRT_DIR)/*.$(m).a lib/
	mv $(LQRT_DIR)/lqrt bin/

clean_lqrt:
	$(MAKE) -C $(LQRT_DIR) clean
	$(RM) bin/lqrt
	$(RM) lib/libLQRT.*
	$(RM) $(LQRT_DIR)/*.$(m).a

lhrt:
	$(MAKE) -C $(LHRT_DIR)
	cp $(LHRT_DIR)/*.$(m).a lib/
	mv $(LHRT_DIR)/lhrt bin/

clean_lhrt:
	$(MAKE) -C $(LHRT_DIR) clean
	$(RM) bin/lhrt
	$(RM) lib/libLHRT.*
	$(RM) $(LHRT_DIR)/*.$(m).a
	
lrrst:
	$(MAKE) -C $(LRRST_DIR)
	cp $(LRRST_DIR)/*.$(m).a lib/
	mv $(LRRST_DIR)/lrrst bin/

clean_lrrst:
	$(MAKE) -C $(LRRST_DIR) clean
	$(RM) bin/lrrst
	$(RM) lib/libLRRST.*
	$(RM) $(LRRST_DIR)/*.$(m).a

lrst:
	$(MAKE) -C $(LRST_DIR)
	cp $(LRST_DIR)/*.$(m).a lib/
	mv $(LRST_DIR)/lrst bin/

clean_lrst:
	$(MAKE) -C $(LRST_DIR) clean
	$(RM) bin/lrst
	$(RM) lib/libLRST.*
	$(RM) $(LRST_DIR)/*.$(m).a

clean: clean_hrt clean_rrst clean_rst clean_util clean_qrt \
	   clean_lqrt clean_lhrt clean_lrrst clean_lrst

