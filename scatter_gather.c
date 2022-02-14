#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "scatter_gather.h"


int scatter(void **common_data, const void *init_data, const int segments, void **proc_data, const int length) {

    int segment_size = length / segments;
    *common_data = malloc(length);
    void *_proc_data = malloc(segment_size);
    // create an area of shared memory and write it to common_data
    int protection = PROT_READ | PROT_WRITE;
    int visibility = MAP_SHARED | MAP_ANONYMOUS;
    *common_data = mmap(NULL, length, protection, visibility, -1, 0);
    // copy init_data into the shared memory area
    memcpy(*common_data, init_data, length);

    // create child processes
    for (int i = 0; i < segments-1; i++) {
       int pid = fork();
        if (pid == 0) {
            // assign work to child process
            // lock on
            memcpy(_proc_data, *common_data+i*(segment_size), segment_size);
            *proc_data = _proc_data;
            // lock off
            return i;
        } else if (pid == -1) {
            return -1; // error during spawn fork
        } 
    }

    // assign work to parent process
    // lock on
    memcpy(_proc_data, *common_data + (segments-1)*segment_size, segment_size);
    *proc_data = _proc_data;
    // lock off
    return segments - 1;
}

const void* gather(void **common_data, int segments, int segment_no, const int length, void **proc_data) {
    int segment_size = length / segments;
    if (segment_no < segments - 1) {
        // lock on
        printf("child\n");
        memcpy(*common_data + segment_no * segment_size, *proc_data, 
                segment_size);
        // lock off
        printf("child ready %d\n", segment_no);
        exit(0); // end the child's existence
    } else {
        // wait for all childs to exit
        int returnStatus; 
        waitpid(-1, &returnStatus, 0); 
        if (returnStatus == -1) {
            exit(0); // something failed at wait.
        }  

        memcpy(*common_data + (segments-1)*segment_size, *proc_data, 
                segment_size);
        return *common_data;
    }
}
