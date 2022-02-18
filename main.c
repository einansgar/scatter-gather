#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/scatter_gather.h"

#define ARRAY_SIZE 60
#define SEGMENTS 20

void doubletest() {
    double *init_ = (double*) malloc(ARRAY_SIZE*sizeof(double));

    double *proc_mem;
    int segment_no = scatter(init_, SEGMENTS, (void**)&proc_mem, sizeof(double)*ARRAY_SIZE);

    if (segment_no == -1) {
        printf("Error in double scatter\n");
        return;
    }

    for (int i=0; i < ARRAY_SIZE/SEGMENTS; i++) {
        proc_mem[i] = 1.0 / (i+1);
        //printf("want to fill proc_mem with %lf\n", 1.0/(i+1));
    }
    double *exit_mem;

    int ret = gather((void**) &exit_mem);

    if (ret != 0) {
        printf("Error in gather.\n");
        return;
    }

    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("%lf\t", exit_mem[i]);
    }
    printf("\n");
    free(exit_mem);
}

void chartest() {

    char *init_ = "01234567890123456789";
    //char init_[ARRAY_SIZE];
    int segments = 5;
    char * proc_;
    int segment_no = scatter(init_, segments, (void**)&proc_, strlen(init_));

    if (segment_no == -1) {
        printf("Error during scatter.\n");
        return;
    }
    char * proc2;
    int ret2 = scatter(proc_, 2, (void**)&proc2, 2);

    printf("ret2 = %d\n", ret2);

    proc_[0] = 'x';
    proc_[1] = 'y';
    //proc2[0] = 'h';
    //proc2[1] = 'a';

    char * exit2;
    printf("call gather for 2\n");
    gather((void**)&exit2);



    char * exit_;

    printf("gather #2\n");

    //printf("proc_: %s of segment no %d\n", proc_, segment_no);

    //__sync_synchronize();

    int ret = gather((void**)&exit_);

    printf("result: %s\n", exit_);
    printf("result first: %s\n", exit2);


    //char * exit2;
    //int ret2 = gather((void**)&exit2);
    free(exit_);

}

void fill_int(int *array, int len, int val) {
    for (int i = 0; i < len; i++) {
        array[i] = val;
    }
}

void inttest() {

    int *init_ = (int*) malloc(sizeof(int)*ARRAY_SIZE);
    fill_int(init_, ARRAY_SIZE, 2);
    int segments = SEGMENTS;
    int * proc_;
    int * exit_;
    int segment_no = scatter(init_, segments, (void**)&proc_, ARRAY_SIZE*sizeof(int));

    if (segment_no == -1) {
        printf("Error during scatter.\n");
        return;
    }

    proc_[0] = 666;
    proc_[1] = 666;
    proc_[2] = 42;


    int ret = gather((void**)&exit_);

    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("%d\t", exit_[i]);
    }

    printf("\n");
    free(exit_);

}


int main() {
    doubletest();
    //chartest();
    //printf("chartest ready\n");
    //inttest();
    return 0;
}
