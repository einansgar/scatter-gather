#include <unistd.h>
//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <errno.h>

#include "../include/scatter_gather.h"

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
    int length; // length of shared memory in bytes
    int segments; // number of parallel processes
    int is_root; // indicate whether a process is the one that called scatter
    int children; // number of children the process needs to wait for to exit
    int used; // whether currently scattered
    //int curr;
} _manage_sg;
static _manage_sg _msg;


int scatter(void *init_data, const int segments, void **proc_data, const int length, const int size_datatype) {

    //printf("%d segments on %d length with datatype size %d\n", segments, length, size_datatype);
    if (segments < 1) {
        //printf("Segments must be positive.\n");
        errno = EINVAL;
        return -1;
    } else if (segments > length) {
        //printf("Cannot create more segments than length\n");
        errno = EINVAL;
        return -1;
    } else if ((length/segments) * segments != length) {
        //printf("Segments do not match length.\n");
        errno = EINVAL;
        return -1;
    } else if (_msg.used == 1) {
        // enforce that scatter cannot be called inside scattered processes
        //printf("Scatter called twice.\n");
        errno = EBUSY;
        return -1;
    }

    // assume that we do not have multithreading issues until now.
    _msg.used = 1; 
    // copy arguments into hidden memory
    _msg.segments = segments;
    _msg.length = length * size_datatype;
    // the size each segment should use
    int segment_size = size_datatype * length / segments;


    // create an area of shared memory
    _msg.com = malloc(_msg.length);
    int protection = PROT_READ | PROT_WRITE;
    int visibility = MAP_SHARED | MAP_ANONYMOUS;
    _msg.com = mmap(NULL, _msg.length, protection, visibility, -1, 0);
    // copy init_data into the shared memory area
    memcpy(_msg.com, init_data, _msg.length);

    // create child processes
    int area_start = 0; // measured in segment_size
    int area_size = segments; // measured in segment_size
    _msg.is_root = 1; // until now, no fork has happened
    _msg.children = 0; // until now, no children have been created

    // Now, create child processes recursively until the whole area has been
    // distributed among them.
    while(area_size > 1) {
        int pid = fork();
        if (pid == 0) {
            // Child. Gets the bigger half because it is often executed first
            area_size -= area_size/2;
            _msg.is_root = 0; // definitely not.
            _msg.children = 0; // not yet, so reset the counter.
        } else {
            // Parent. Gets the remaining memory.
            area_start += (area_size+1)/2;
            area_size /= 2;
            _msg.children ++;
        }
    }
    *proc_data = _msg.com + area_start*segment_size; // assign some memory area
    //_msg.curr = area_start;
    return _msg.children;
}

int gather(void **exit_data) {
    if (_msg.segments == 0) {
        errno = ENOTSUP;
        return -1;
    }
    int segment_size = _msg.length / _msg.segments; // in bytes
    //printf("Gather segment %d\n", _msg.curr);

    if (!_msg.children && !_msg.is_root) {
        //printf("segment %d finished as leaf.\n", _msg.curr);
        exit(0); // we can safely exit this one because it's a leaf.
    } else if (!_msg.is_root) {
        // just wait for the childs to finish and then exit.
        int status;
        while ((waitpid(0, &status, 0)) > 0) {
            _msg.children = _msg.children + status - 1;
        }
        if (_msg.children != 0) {
            //printf("Couldn't collect all childs/too many left. Left: %d\n", _msg.children);
            exit(_msg.children);
        }
        //printf("segment %d finished as parent.\n", _msg.curr);
        exit(0);
    } else {
        if (_msg.used != 1) {
            //printf("Called scather a second time, abort.\n");
            return -1;
        }
        int status;
        //int unfinished = _msg.children;
        while ((waitpid(0, &status, 0)) > 0) {
            //unfinished = unfinished + status - 1;
            _msg.children = _msg.children + status - 1;
        }
        //if (unfinished != 0) {
        if (_msg.children != 0) {
            //printf("Couldn't collect all childs/too many left. Left: %d\n", unfinished);
            //printf("Couldn't collect all childs/too many left. Left: %d\n", _msg.children);
            //return unfinished;
            errno = ECHILD;
            return _msg.children;
        }
        
        // Copy data into result pointer and delete shared memory area.
        *exit_data = malloc(_msg.length);
        memcpy(*exit_data, _msg.com, _msg.length);
        munmap(_msg.com, _msg.length);
        
        _msg.used = 0; // allow scatter
        return 0;
    }
}
