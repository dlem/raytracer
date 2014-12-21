SUBDIRS = demos docs src
CLEAN_SUBDIRS = $(addprefix clean_,$(SUBDIRS))

.PHONY: all clean $(SUBDIRS) $(CLEAN_SUBDIRS)

all: $(SUBDIRS)

clean: $(CLEAN_SUBDIRS)

demos: src
docs: demos

$(SUBDIRS): 
	cd $@ && $(MAKE) 

$(CLEAN_SUBDIRS):
	cd $(subst clean_,,$@) && $(MAKE) clean	
