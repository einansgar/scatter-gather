DOC  := report.tex

RERUN := "(undefined references|Rerun to get (cross-references|the bars|point totals) right|Table widths have changed. Rerun LaTeX.|Linenumber reference failed)"
RERUNBIB := "No file.*\.bbl|Citation.*undefined"

CC ?= gcc
CFLAGS ?= -g -Wall -O2 --std=c99
CXX ?= c++
CXXFLAGS ?= -g -pthread -Wall -O2

# size can be divided by any number until 17. 
SIZE ?= 504000


#all: task1 run_t1 figs1 doc 
#all: task2 run_t2 run_t3 figs2 figs3 #doc
all: main run doc

exp2: exp2.c scatter_gather.o timings.o
	$(CC) $(CFLAGS) scatter_gather.o timings.o exp2.c -o exp2

exp1: exp1.c scatter_gather.o timings.o
	$(CC) $(CFLAGS) scatter_gather.o timings.o exp1.c -o exp1

main: main.c scatter_gather.o
	$(CC) $(CFLAGS) scatter_gather.o main.c -o main

random: random.c
	$(CC) $(CFLAGS) random.c -o random

scatter_gather.o: scatter_gather/scatter_gather.c timings.o include/scatter_gather.h
	$(CC) -c scatter_gather/scatter_gather.c

timings.o: timings.c include/timings.h
	$(CC) -c timings.c

run: main
	./main

#figs1:
#	python3 ./plot1.py

run_exp1: exp1
	for number in 1 2 3 4 5 6 7 8 9 0 11 12 13 14 15 16 17 18 19 20 ; do \
		./exp1 s $(SIZE) f data/dump1.txt r 1 2 3 4 5 6 7 8 9 10 12 14 16 18 20 30 40 ; \
	done

run_exp2: exp2
	for number in 1 2 3 4 5 6 7 8 9 0 11 12 13 14 15 16 17 18 19 20 ; do \
		./exp2 s $(SIZE) f data/dump2.txt r 1 2 3 4 5 6 7 8 9 10 12 14 16 18 20 30 40 ; \
	done

figs: run_exp1 run_exp2
	python3 scripts/plot_dump.py `pwd`/data/dump1.txt `pwd`/figs/dump1.pdf
	python3 scripts/plot_dump.py `pwd`/data/dump2.txt `pwd`/figs/dump2.pdf



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

