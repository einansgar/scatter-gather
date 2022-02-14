#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "scatter_gather.h"

// this one should be defined by sys/mman.h but vs code does not find it
#ifndef MAP_ANONYMOUS
# ifdef __MAP_ANONYMOUS
#  define MAP_ANONYMOUS __MAP_ANONYMOUS
# else
#  define MAP_ANONYMOUS 0x20
# endif
#endif

// make this inaccessible to the user to increase security.
typedef struct {
    void * com; // shared memory
    int segment; // segment number of the process
    int length; // length of shared memory in bytes
    int segments; // number of parallel processes
    int used; // whether currently scattered
} _manage_sg;
static _manage_sg _msg;

int scatter(void *init_data, const int segments, void **proc_data, const int length) {
    if ((length/segments) * segments != length) {
        printf("Segments do not match length.\n");
    }

    // enforce that scatter cannot be called inside scattered processes
    if (_msg.used == 1) {
        printf("Scatter called twice.\n");
        return -1;
    } else {
        _msg.used = 1;
    }

    int segment_size = length / segments;

    _msg.segments = segments;
    _msg.length = length;

    _msg.com = malloc(length);
    // create an area of shared memory and write it to common_data
    int protection = PROT_READ | PROT_WRITE;
    int visibility = MAP_SHARED | MAP_ANONYMOUS;
    _msg.com = mmap(NULL, length, protection, visibility, -1, 0);
    // copy init_data into the shared memory area
    memcpy(_msg.com, init_data, length);

    // create child processes
    for (int i = 0; i < segments-1; i++) {
       int pid = fork();
        if (pid == 0) {
            // assign work to child process
            *proc_data = _msg.com+i*(segment_size);
            _msg.segment = i;
            return i;
        } else if (pid == -1) {
            return -1; // error during spawn fork
        } 
    }
    // assign work to parent process
    *proc_data = _msg.com + (segments-1)*segment_size;
    _msg.segment = segments - 1;
    return segments - 1;
}

int gather(void **exit_data) {
    int segment_size = _msg.length / _msg.segment;

    if (_msg.segment < _msg.segments - 1) {
        //printf("child ready %d\n", getpid());
        exit(0); // end the child's existence
    } else {
        // wait for all childs to exit

        int outstanding = _msg.segments-1;
        int status;
        int child;

        while ((child=waitpid(-1, &status, 0)) > 0) {
            //printf(" %d finished\n", child);
            outstanding--;
        }
        if (outstanding != 0) {
            printf("Couldn't collect all childs/too many. Left: %d\n", outstanding);
            return outstanding;
        }
        
        *exit_data = malloc(_msg.length);
        memcpy(*exit_data, _msg.com, _msg.length);
        munmap(_msg.com, _msg.length);
        _msg.used = 0;

        return 0;
    }
}
