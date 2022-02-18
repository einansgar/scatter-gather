#ifndef SCATTER_GATHER_H
#define SCATTER_GATHER_H

/* scatter: create a set of processes to compute tasks in parallel
 *  param init_data: data to initialize the processes. Will be shared equally.
 *      init_data must have a constant length.
 *      init_data must have the length indicated in param length.
 *      cannot use a pointer on other pointers because then the values do not 
 *        get copied.
 *  param segments: number of processes to create.
 *  param proc_data: where to put the data for each process.
 *      length(proc_data) = length / segments.
 *  param length: length of init_data array. Must be a multiple of segments. 
 *  param size_datatype: size of the datatype to use.
 *  Note: You cannot call scatter between you called scatter and gather.
 *  return -1 in case of error, otherwise a process identifier. 
 *      segments-1 is the parent process.
 */
int scatter(void *init_data, const int segments, void **proc_data, const int length, const int size_datatype);

/* gather: combine the processes to one single process.
 *  exit the child processes, exit the parent in case of any failure
 *
 *  param exit_data: where to write the resulting data.
 *      Allocates memory dynamically here. Use free().
 *  Note: You cannot access proc_data after this call.
 *  return: 0 if everything alright, else the number of unfinished processes 
 */
int gather(void **exit_data);

#endif // SCATTER_GATHER_H