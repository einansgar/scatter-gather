#ifndef SCATTER_GATHER_H
#define SCATTER_GATHER_H


// this one should be defined by sys/mman.h but vs code does not find it
#ifndef MAP_ANONYMOUS
# ifdef __MAP_ANONYMOUS
#  define MAP_ANONYMOUS __MAP_ANONYMOUS
# else
#  define MAP_ANONYMOUS 0x20
# endif
#endif


// internal use type to be able to save main variables in static memory
typedef struct {
    void **com;
    void **proc;
} _sg_t;

static _sg_t _sgt;

// initialize for scatter/gather. Must be called in main() before scatter/gather
//    can be used.
#ifndef initsg
# define initsg()     void * com; void * proc; _sgt.com = &com; _sgt.proc = &proc;
#endif // initsg


/* scatter: create a set of processes to compute tasks in parallel
 *  param init_data: data to initialize the processes. Will be shared equally.
 *      init_data must have a constant length.
 *      init_data must have the length indicated in param length.
 *      cannot use a pointer on other pointers because then the values do not 
 *        get copied.
 *  param segments: number of processes to create.
 *  param proc_data: where to put the data for each process.
 *      length(proc_data) = length / segments.
 *  param length: length of init_data. Must be a multiple of segments. 
 *  return -1 in case of error, otherwise a process identifier. 
 *      segments-1 is the parent process.
 */
int scatter(void **common_data, const void *init_data, const int segments, void **proc_data, const int length);


/* gather: combine the processes to one single process.
 *  exit the child processes, exit the parent in case of any failure
 *  param segments: amount of processes that have been created in scatter
 *  param segment_no: process identifier returned from scatter
 *  param length: size of the common memory in bytes
 *  return: common memory  
 */
const void* gather(void **common_data, int segments, int segment_no, const int length, void **proc_data);

#endif // SCATTER_GATHER_H