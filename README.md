# scatter-gather

## About

Scatter Gather functionality using processes.

## Contents

The functions are implemented in `include/scatter_gather.h` and in `scatter_gather.c`. The other files represent examples and a report document.

## How to build and run

`make` will build, execute examples and generate the report. Multiple calls to `make` will append more execution data to the files in `data` which will later be used to generate graphics for the report. If you want to generate clean output, use `make distclean` before the call to `make`. You might want to modify the Makefile depending on your own application (e. g. remove all the report and exp1, exp2, main stuff).

## Warning

Execution of the programs might lead to high CPU utilization. However, you can exit the program at any time using `Ctrl+C`.
