include ../common.mk

PNGS = $(subst .lua,.png,$(DEMO_LUAS))

all: $(PNGS)

clean:
	rm -rf *.png

%.png: %.lua
	$(RT) $<
