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


/* scatter: create a set of processes to compute tasks in parallel
 *  param common_data: where to put the shared data during parallel execution.
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
 *
 *  param common_data: shared memory, use the same pointer as in scatter
 *  param segments: amount of processes that have been created in scatter
 *  param segment_no: process identifier returned from scatter
 *  param length: size of the common memory in bytes
 *  param proc_data: private memory area of the current process
 *  return: 0 if everything alright, else the number of unfinished processes 
 */
int gather(void **common_data, int segments, int segment_no, const int length, void **proc_data);

#endif // SCATTER_GATHER_H