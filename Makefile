SUBDIRS = demos docs src

include common.mk

tests demos: src
docs: demos
