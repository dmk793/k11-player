
TEX_FILES += dport.tex
TEX_FILES += os.tex
TEX_FILES += lpc1788con.tex

DVI_FILES += $(foreach obj,$(TEX_FILES),$(patsubst %tex,%dvi, $(obj)))
PDF_FILES += $(foreach obj,$(TEX_FILES),$(patsubst %tex,%pdf, $(obj)))

.PHONY: all pdf clean
all: $(DVI_FILES)

pdf: $(PDF_FILES)

$(DVI_FILES):
$(PDF_FILES):

%.dvi: %.tex
	latex $<
	latex $<

%.pdf: %.tex
	pdflatex $<
	pdflatex $<

clean:
	rm -f *.dvi *.pdf *.aux *.log *.out *.toc

