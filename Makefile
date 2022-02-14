DOC  := report.tex

RERUN := "(undefined references|Rerun to get (cross-references|the bars|point totals) right|Table widths have changed. Rerun LaTeX.|Linenumber reference failed)"
RERUNBIB := "No file.*\.bbl|Citation.*undefined"

CC ?= gcc
CFLAGS ?= -g -Wall -O2
CXX ?= c++
CXXFLAGS ?= -g -pthread -Wall -O2


#all: task1 run_t1 figs1 doc 
#all: task2 run_t2 run_t3 figs2 figs3 #doc
all: scatter_gather run doc

scatter_gather.cpp: 
	$(CC) $(CFLAGS) -o $@ $<

run: scatter_gather
	./scatter_gather

#figs1:
#	python3 ./plot1.py


doc: $(DOC:.tex=.pdf)

#run_scatter_gather: scatter_gather
#    ./scatter_gather

%.pdf: %.tex
	pdflatex $<
	@egrep -q $(RERUNBIB) $*.log && bibtex $* && pdflatex $<; true
	@egrep -q $(RERUN) $*.log && pdflatex $<; true
	@egrep -q $(RERUN) $*.log && pdflatex $<; true

latexmk:
	-latexmk -pvc -pdf $(DOC)

purge:
	-rm -f *.{aux,dvi,log,bbl,blg,brf,fls,toc,thm,out,fdb_latexmk}

clean: purge
	$(MAKE) -C figs $@
	-rm -f $(DOC:.tex=.pdf)

.PHONY: all figs purge clean
