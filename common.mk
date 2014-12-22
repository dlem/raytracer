COMMON_MK_PATH = $(abspath $(lastword $(MAKEFILE_LIST)))
ROOT = $(patsubst %/,%,$(dir $(COMMON_MK_PATH)))
RT = $(ROOT)/src/rt -S
DEMO_LUAS = $(wildcard $(ROOT)/demos/*.lua)

# Tells LUA where to look for includes.
export LUA_PATH=$(ROOT)/demos/?.lua;$(ROOT)/demos/assets/?.lua;$(ROOT)/demos/include/?.lua
export ASSET_PATH=$(ROOT)/demos/assets

# Automatically handle making/cleaning subdirectories if the caller defines
# SUBDIRS.
CLEAN_SUBDIRS = $(addprefix clean_,$(SUBDIRS))

$(SUBDIRS): 
	cd $@ && $(MAKE) 

$(CLEAN_SUBDIRS):
	cd $(subst clean_,,$@) && $(MAKE) clean	

.PHONY: all clean $(SUBDIRS) $(CLEAN_SUBDIRS)

all: $(SUBDIRS)

clean: $(CLEAN_SUBDIRS)
