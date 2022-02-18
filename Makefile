DOC  := report.tex

RERUN := "(undefined references|Rerun to get (cross-references|the bars|point totals) right|Table widths have changed. Rerun LaTeX.|Linenumber reference failed)"
RERUNBIB := "No file.*\.bbl|Citation.*undefined"

CC ?= gcc
CFLAGS ?= -g -Wall -O2 --std=c99
CXX ?= c++
CXXFLAGS ?= -g -pthread -Wall -O2


#all: task1 run_t1 figs1 doc 
#all: task2 run_t2 run_t3 figs2 figs3 #doc
all: main run doc

exp1: exp1.c scatter_gather.o
	$(CC) $(CFLAGS) scatter_gather.o exp1.c -o exp1

main: main.c scatter_gather.o
	$(CC) $(CFLAGS) scatter_gather.o main.c -o main

scatter_gather.o: scatter_gather/scatter_gather.c include/scatter_gather.h
	$(CC) -c scatter_gather/scatter_gather.c


run: main
	./main

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


clean:
	rm *.o exp1 main random *.gz *.aux *.log *.out

distclean:
	rm -f *.o exp1 main random *.gz *.pdf 
	rm -rf figs
	rm -rf data

