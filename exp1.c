#include <stdio.h>
#include <stdlib.h>

#include "random.h"
#include "scatter_gather.h"

#define SIZE 10000


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

void run_test(int segments) {
    double *init_ = (double*) malloc(SIZE*sizeof(double));

    double *proc_mem;
    int segment_no = scatter(init_, segments, (void**)&proc_mem, sizeof(double)*SIZE);

    if (segment_no == -1) {
        printf("Error in double scatter\n");
        return;
    }

    //for (int i=0; i < SIZE/segments; i++) {
    //    proc_mem[i] = 1.0 / (i+1);
    //    //printf("want to fill proc_mem with %lf\n", 1.0/(i+1));
    //}

    for (int i = 0; i < 100; i++) {
        vector_fill(SIZE/segments, 1, proc_mem);
        vector_scale(SIZE/segments, segment_no, proc_mem);
    }

    double *exit_mem;

    int ret = gather((void**) &exit_mem);

    if (ret != 0) {
        printf("Error in gather.\n");
        return;
    }

    //for (int i = 0; i < SIZE; i++) {
    //    printf("%lf\t", exit_mem[i]);
    //}
    //printf("\n");
    free(exit_mem);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Please provide number of segments.\n");
        exit(0);
    } 

    int no_segments = atoi(argv[1]);

    if (no_segments < 1) {
        printf("Please provide segments > 0");
        exit(0);
    } else if ((SIZE / no_segments) * no_segments != SIZE) {
        printf("Warning: segments should be a factor of %d\n", SIZE);
    }

    run_test(no_segments);
    return 0;
}