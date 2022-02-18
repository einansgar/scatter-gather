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
    int is_leaf;
    int is_parent;
    int children;
} _manage_sg;
static _manage_sg _msg;


int scatter(void *init_data, const int segments, void **proc_data, const int length, const int size_datatype) {
    #if TIMING
    long base = get_time_base();
    long t0 = get_time_mus(base);
    __sync_synchronize();
    #endif

    printf("%d segments on %d length with datatype size %d\n", segments, length, size_datatype);
    if (segments < 1) {
        printf("Segments must be positive.\n");
        return -1;
    } else if (segments > length) {
        printf("Cannot create more segments than length\n");
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

    int segment_size = size_datatype * length / segments;

    _msg.segments = segments;
    _msg.length = length * size_datatype;

    #if TIMING
    __sync_synchronize();
    long t1 = get_time_mus(base);
    __sync_synchronize();
    #endif


    _msg.com = malloc(_msg.length);

    #if TIMING
    __sync_synchronize();
    long t2 = get_time_mus(base);
    __sync_synchronize();
    #endif
    // create an area of shared memory and write it to common_data
    int protection = PROT_READ | PROT_WRITE;
    int visibility = MAP_SHARED | MAP_ANONYMOUS;
    _msg.com = mmap(NULL, _msg.length, protection, visibility, -1, 0);

    #if TIMING
    __sync_synchronize();
    long t3 = get_time_mus(base);
    __sync_synchronize();
    #endif
    // copy init_data into the shared memory area
    memcpy(_msg.com, init_data, _msg.length);

    #if TIMING
    __sync_synchronize();
    long t4 = get_time_mus(base);
    __sync_synchronize();
    #endif

    // create child processes
    int area_start = 0;
    int area_size = segments;
    _msg.is_leaf = 1;
    _msg.is_parent = 1;
    _msg.children = 0;

    while(area_size > 1) {
        int pid = fork();
        //printf("current mem area: %d with size: %d\n", area_start, area_size);
        if (pid == 0) {
            // child.
            //printf("Created another child at layer %d\n", _msg.children);
            //*proc_data = _msg.com + area_start;
            // _msg.segment
            area_size -= area_size/2;
            _msg.is_parent = 0;
            _msg.children = 0;
        } else {
            // parent.
            //printf("Still parent at layer %d\n", _msg.children);

            area_start += (area_size+1)/2;
            area_size /= 2;
            //*proc_data = _msg.com + area_start;
            
            _msg.is_leaf = 0;
            _msg.children ++;
            // _msg.segment
        }
    }
    *proc_data = _msg.com + area_start*segment_size;
    _msg.segment = 0;
    //printf("Segment created. for mem area %d\n", area_start);
    return _msg.children;
}

int gather(void **exit_data) {
    #if TIMING
    long base = get_time_base();
    long t0 = get_time_mus(base);
    __sync_synchronize();
    #endif
    int segment_size = _msg.length / _msg.segments;

    if (_msg.is_leaf && !_msg.is_parent) {
        //printf("Leaf exits with 0.\n");
        exit(0); // we can safely exit this one.
    } else if (!_msg.is_parent) {
        //printf("Not overall parent.\n");
        // just wait for the childs to finish and then exit.
        int status;
        int unfinished;
        while ((waitpid(0, &status, 0)) > 0) {
            //printf(" %d finished\n", child);
            _msg.children--;
            //_msg.children += unfinished;
        }
        if (_msg.children != 0) {
            printf("Couldn't collect all childs/too many left. Left: %d\n", _msg.children);
            exit(0);
        }
        //printf("Leaf exits with 0.\n");
        exit(0);
    } else {
        //printf("hold on, the parent's here. Needs to wait for %d childs\n", _msg.children);
        int status;
        int unfinished = _msg.children;
        while ((waitpid(0, &status, 0)) > 0) {
            //printf(" %d finished\n", child);

            //printf("unfinished = %d\n", unfinished);
            unfinished --;
            //_msg.children += unfinished;
        }
        if (unfinished != 0) {
            printf("Couldn't collect all childs/too many left. Left: %d\n", unfinished);
            return unfinished;
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
    printf("stranded?\n");
    return -1;
}
