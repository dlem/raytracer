# Donated code; modified by dlemmond.

SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
DEPENDS = $(SOURCES:.cpp=.d)
LDFLAGS = $(shell pkg-config --libs lua5.1) -llua5.1 -lpng -lpthread
CPPFLAGS = $(shell pkg-config --cflags lua5.1) -std=c++0x
CXXFLAGS = $(CPPFLAGS) -Wall -Wextra -Wno-sign-compare -Wno-reorder -Wno-unused-parameter -O1 -Wno-parentheses -g
CXX = g++
MAIN = rt

all: $(MAIN) manual.pdf

depend: $(DEPENDS)

clean:
	rm -f *.o *.d *.log *.aux $(MAIN) manual.pdf

$(MAIN): $(OBJECTS)
	@echo Creating $@...
	@$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	@echo Compiling $<...
	@$(CXX) -o $@ -c $(CXXFLAGS) $<

%.d: %.cpp
	@echo Building $@...
	@set -e; $(CC) -M $(CPPFLAGS) $< \
                  | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
                [ -s $@ ] || rm -f $@

%.pdf : %.tex
	pdflatex $< -o $@

include $(DEPENDS)
