RT = ../src/rt -S
LUAS = $(wildcard *.lua)
PNGS = $(subst .lua,.png,$(LUAS))
LUA_PATH = $(shell pwd)/?.lua;$(shell pwd)/assets/?.lua;$(shell pwd)/include/?.lua

export LUA_PATH

all: $(PNGS)

clean:
	rm -rf *.png

%.png: %.lua
	$(RT) $<
