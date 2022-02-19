#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "include/scatter_gather.h"
#include "include/timings.h"

#define SIZE 100000
#define RUNCNT 2

void vector_fill(int n, double val, double x[]) {
    for (int i = 0; i < n; i++) {
        x[i] = val;
    }
}

void vector_scale(int n, double scale, double x[]) {
    for (int i = 0; i < n; i++) {
        x[i] *= scale;
    }
}

void vector_add(int n, double val, double x[]) {
    for (int i = 0; i < n; i++) {
        x[i] = val + x[i];
    }
}

void run_test(int segments, int size) {
    double *init_ = (double*) malloc(size*sizeof(double));

    double *proc_mem;

    int segment_no = scatter(init_, segments, (void**)&proc_mem, size, sizeof(double));


    if (segment_no == -1) {
        printf("Error in double scatter\n");
        return;
    }

    for (int i = 0; i < 10; i++) {
        vector_fill(size/segments, 1, proc_mem);
        vector_scale(size/segments, segment_no, proc_mem);
        vector_add(size/segments, 0.5, proc_mem);
        vector_scale(size/segments, -0.1, proc_mem);
    }

    double *exit_mem;

    int ret = gather((void**) &exit_mem);

    if (ret != 0) {
        printf("Error in gather.\n");
        return;
    }

    free(exit_mem);
    free(init_);
}

int main(int argc, char *argv[]) {


    int size, runcnt;
    int *runs;
    char * filename = "dump.txt";
    size = SIZE;
    runcnt = RUNCNT;
    runs = (int*)malloc(sizeof(int)*runcnt);
    for (int i = 0; i < runcnt; i++) {
        runs[i] = 1;
    }
    if (argc < 2) {
        printf("using default values.\n");
    } else if (argc == 2) {
        printf("Help on %s:\nusage\n\n"
                "%s s [SIZE] f [OUTFILE] r [RUNS WITH DIFFERENT SEGMENTS]\n"
                "example: %s s 10000 f dump.txt r 1 2 4 5 8 10 16 20\n"
                "r must be the last argument.\n", argv[0], argv[0], argv[0]);
        exit(0);
    } else {
        int rem = argc - 1;
        int curr = 1;
        while (rem > 0) {
            if (rem < 2) {
                printf("Invalid command, type %s h for help\n", argv[0]);
                exit(0);
            } else if (!strcmp(argv[curr], "s")) {
                size = atoi(argv[curr+1]);
                curr += 2;
                rem -= 2;
            } else if (!strcmp(argv[curr], "f")) {
                filename = argv[curr+1];
                curr += 2;
                rem -= 2;
            } else if (!strcmp(argv[curr], "r")) {
                runcnt = rem - 1;
                runs = (int*) realloc(runs, sizeof(int)*runcnt);
                for (int i = 0; i < runcnt; i++) {
                    runs[i] = atoi(argv[curr+i+1]);
                }
                rem = 0;
            } else {
                printf("Invalid command, type %s h for help\n", argv[0]);
                exit(0);
            }

        }
    }

    //printf("Called with size=%d file=%s #runs=%d\n", size, filename, runcnt);


    

    for (int i = 0; i < runcnt; i++) {

        if (runs[i] == 0 || (size / runs[i]) * runs[i] != size) {
            printf("skipped run %d - invalid parameters.\n", i);
            continue;
        } else {
            printf("called %dth time  with %d\n", i, runs[i]);
        }
        long base_time = get_time_base();
        
        long start = get_time_mus(base_time);

        run_test(runs[i], size);

        long stop = get_time_mus(base_time);

        FILE * f = fopen(filename, "a");
        fprintf(f, "%d %ld\n", runs[i], stop-start);
        fclose(f);
        //printf("%d %ld\n", runs[i], stop-start);
    }

    

    return 0;
}