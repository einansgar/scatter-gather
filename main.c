#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/scatter_gather.h"

#define ARRAY_SIZE 100
#define SEGMENTS 50

void doubletest() {
    double *init_ = (double*) malloc(ARRAY_SIZE*sizeof(double));

    double *proc_mem;
    int segment_no = scatter(init_, SEGMENTS, (void**)&proc_mem, ARRAY_SIZE, sizeof(double));

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
    int segment_no = scatter(init_, segments, (void**)&proc_, strlen(init_), sizeof(char));

    if (segment_no == -1) {
        printf("Error during scatter.\n");
        return;
    }
    char * proc2;
    int ret2 = scatter(proc_, 2, (void**)&proc2, 2, sizeof(char));

    if (ret2 == -1) {
        //printf("Error in scatter.\n");
        //exit(0);
    }
    proc_[0] = 'x';
    proc_[1] = 'y';
    
    char * exit2;
    //printf("call gather for 2\n");
    gather((void**)&exit2);



    char * exit_;

    //printf("gather #2\n");

    int ret = gather((void**)&exit_);

    printf("result: %s ;) and return value %d\n", exit_, ret);
    printf("result first: %s\n", exit2);

    free(exit2);

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
    int segment_no = scatter(init_, segments, (void**)&proc_, ARRAY_SIZE, sizeof(int));

    if (segment_no == -1) {
        printf("Error during scatter.\n");
        return;
    }

    proc_[0] = 666;
    proc_[1] = 666;
    proc_[2] = 42;


    int ret = gather((void**)&exit_);
    if (ret == 0) {
        free(exit_);
    }
}

int main() {
    doubletest();
    chartest();
    printf("chartest ready\n");
    inttest();
    printf("inttest ready.\n");

    return 0;
}
