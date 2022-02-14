#include <string.h>
#include <stdio.h>

#include "scatter_gather.h"

#define ARRAY_SIZE 300
#define SEGMENTS 100

void chartest() {

    char *init_ = "01234567890123456789";
    char *common;
    //char init_[ARRAY_SIZE];
    int segments = 5;
    char * proc_;
    int segment_no = scatter((void**)&common, init_, segments, (void**)&proc_, strlen(init_));

    if (segment_no == -1) {
        printf("Error during scatter.\n");
        return;
    }

    proc_[0] = 'x';
    proc_[1] = 'y';

    printf("proc_: %s of segment no %d\n", proc_, segment_no);

    __sync_synchronize();

    int ret = gather((void**)&common, segments, segment_no, strlen(init_), (void**)&proc_);

    printf("result: %s\n", common);

}

void fill_int(int *array, int len, int val) {
    for (int i = 0; i < len; i++) {
        array[i] = val;
    }
}

void inttest() {

    int init_[ARRAY_SIZE];
    fill_int(init_, ARRAY_SIZE, 2);
    int segments = SEGMENTS;
    int * proc_;
    void * common;
    int segment_no = scatter(&common, init_, segments, (void**)&proc_, ARRAY_SIZE*sizeof(int));

    if (segment_no == -1) {
        printf("Error during scatter.\n");
        return;
    }

    proc_[0] = 666;
    proc_[1] = 666;
    proc_[2] = 42;

    int ret = gather(&common, segments, segment_no, ARRAY_SIZE*sizeof(int), (void**)&proc_);

    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("%d\t", ((int*)common)[i]);
    }

    printf("\n");

}


int main() {

    chartest();
    printf("chartest ready\n");
    inttest();
    return 0;
}
