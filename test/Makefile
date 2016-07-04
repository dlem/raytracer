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
	for x in reference/*.png ; do compare $$x `basename $$x` diff_`basename $$x`; convert $$x `basename $$x` diff_`basename $$x` +append results/`basename $$x` ; done ; true
	for x in reference/*.`hostname`.time ; do out=results/`hostname`.diff ; echo `basename $$x` >> $$out ; diff $$x `basename $$x` >> $$out ; done ; true

reference: output
	rm -rf reference
	mkdir -p reference
	cp *.png reference
	cp *.time reference

%.png: $(ROOT)/demos/%.lua
	/usr/bin/time -v -o $@.$(shell hostname).time $(RT) --reproducible --threads 1 --minres $<
