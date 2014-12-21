SRCPNGS = $(wildcard ../demos/*.png)
PNGS = $(notdir $(SRCPNGS))
THUMBS = $(subst .png,_thumb.png,$(PNGS))

all: $(PNGS) $(THUMBS) implementation.pdf manual.pdf

clean:
	rm -rf *.png *.pdf *.log *.toc *.aux

$(PNGS):
	cp ../demos/$@ $@

%_thumb.png: %.png
	convert -thumbnail 200 $< $@

%.pdf: %.tex
	pdflatex $<
	pdflatex $<
