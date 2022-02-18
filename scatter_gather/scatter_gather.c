#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "../include/scatter_gather.h"
#include "../include/timings.h"

#define TIMING 0

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
    #if TIMING
    long base = get_time_base();
    long t0 = get_time_mus(base);
    __sync_synchronize();
    #endif
    if (segments < 1) {
        printf("Segments must be positive.\n");
        return -1;
    }
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

    #if TIMING
    __sync_synchronize();
    long t1 = get_time_mus(base);
    __sync_synchronize();
    #endif


    _msg.com = malloc(length);

    #if TIMING
    __sync_synchronize();
    long t2 = get_time_mus(base);
    __sync_synchronize();
    #endif
    // create an area of shared memory and write it to common_data
    int protection = PROT_READ | PROT_WRITE;
    int visibility = MAP_SHARED | MAP_ANONYMOUS;
    _msg.com = mmap(NULL, length, protection, visibility, -1, 0);

    #if TIMING
    __sync_synchronize();
    long t3 = get_time_mus(base);
    __sync_synchronize();
    #endif
    // copy init_data into the shared memory area
    memcpy(_msg.com, init_data, length);

    #if TIMING
    __sync_synchronize();
    long t4 = get_time_mus(base);
    __sync_synchronize();
    #endif

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
    #if TIMING
    __sync_synchronize();
    long t5 = get_time_mus(base);
    __sync_synchronize();
    #endif
    *proc_data = _msg.com + (segments-1)*segment_size;
    _msg.segment = segments - 1;
    #if TIMING
    __sync_synchronize();
    long t6 = get_time_mus(base);
    __sync_synchronize();
    printf("t1: %ld, t2: %ld, t3: %ld, t4: %ld, t5: %ld, t6: %ld\n", t1-t0, t2-t1, t3-t2, t4-t3, t5-t4, t6-t5);
    #endif
    return segments - 1;
}

int gather(void **exit_data) {
    #if TIMING
    long base = get_time_base();
    long t0 = get_time_mus(base);
    __sync_synchronize();
    #endif
    int segment_size = _msg.length / _msg.segments;

    if (_msg.segment < _msg.segments - 1) {
        //printf("child ready %d\n", getpid());
        exit(0); // end the child's existence
    } else {
        // wait for all childs to exit
        #if TIMING
        __sync_synchronize();
        long t1 = get_time_mus(base);
        __sync_synchronize();
        #endif

        int outstanding = _msg.segments-1;
        int status;
        int child;

        #if TIMING
        __sync_synchronize();
        long t2 = get_time_mus(base);
        __sync_synchronize();
        #endif

        while ((child=waitpid(-1, &status, 0)) > 0) {
            //printf(" %d finished\n", child);
            outstanding--;
        }
        if (outstanding != 0) {
            printf("Couldn't collect all childs/too many. Left: %d\n", outstanding);
            return outstanding;
        }

        #if TIMING
        __sync_synchronize();
        long t3 = get_time_mus(base);
        __sync_synchronize();
        #endif
        
        *exit_data = malloc(_msg.length);

        #if TIMING
        __sync_synchronize();
        long t4 = get_time_mus(base);
        __sync_synchronize();
        #endif
        memcpy(*exit_data, _msg.com, _msg.length);
        #if TIMING
        __sync_synchronize();
        long t5 = get_time_mus(base);
        __sync_synchronize();
        #endif
        munmap(_msg.com, _msg.length);
        _msg.used = 0;
        #if TIMING
        __sync_synchronize();
        long t6 = get_time_mus(base);
        __sync_synchronize();
        printf("t1: %ld, t2: %ld, t3: %ld, t4: %ld, t5: %ld, t6: %ld\n", t1-t0, t2-t1, t3-t2, t4-t3, t5-t4, t6-t5);
        #endif

        return 0;
    }
}