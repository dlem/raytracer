include ../common.mk

PNGS = $(subst .lua,.png,$(notdir $(DEMO_LUAS)))

.PHONY: output results reference

all: results output

clean:
	rm -rf results *.png *.time

output: $(PNGS)

results: output
	rm -rf results
	mkdir -p results
	for x in reference/*.png ; do compare $$x `basename $$x` results/diff_`basename $$x`; convert $$x `basename $$x` results/diff_`basename $$x` +append results/`basename $$x` ; done
	for x in reference/*.`hostname`.time ; do out=results/`hostname`.diff ; echo `basename $$x` >> $$out ; echo diff $$x `basename $$x` >> $$out ; done
	

reference: output
	rm -rf reference
	mkdir -p reference
	cp *.png reference
	cp *.time reference

%.png: $(ROOT)/demos/%.lua
	@echo $(HOSTNAME)
	/usr/bin/time -v -o $@.$(shell hostname).time $(RT) --minres $<
