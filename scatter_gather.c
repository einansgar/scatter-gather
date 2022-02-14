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
            memcpy(_proc_data, *common_data+i*(segment_size), segment_size);
            *proc_data = _proc_data;
            return i;
        } else if (pid == -1) {
            return -1; // error during spawn fork
        } 
    }

    // assign work to parent process
    memcpy(_proc_data, *common_data + (segments-1)*segment_size, segment_size);
    *proc_data = _proc_data;
    return segments - 1;
}

int gather(void **common_data, int segments, int segment_no, const int length, void **proc_data) {
    int segment_size = length / segments;
    if (segment_no < segments - 1) {
        memcpy(*common_data + segment_no * segment_size, *proc_data, 
                segment_size);
        // printf("child ready %d\n", getpid());
        exit(0); // end the child's existence
    } else {
        // wait for all childs to exit

        int outstanding = segments-1;
        int status;
        int child;

        while ((child=waitpid(-1, &status, 0)) > 0) {
            // printf(" %d finished\n", child);
            outstanding--;
        }
        if (outstanding != 0) {
            printf("Couldn't collect all childs/too many. Left: %d\n", outstanding);
        }
        memcpy(*common_data + (segments-1)*segment_size, *proc_data, 
                segment_size);
        return outstanding;
    }
}
