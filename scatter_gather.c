#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

// this one should be defined by sys/mman.h but vs code does not find it
#ifndef MAP_ANONYMOUS
# ifdef __MAP_ANONYMOUS
#  define MAP_ANONYMOUS __MAP_ANONYMOUS
# else
#  define MAP_ANONYMOUS 0x20
# endif
#endif

#define ARRAY_SIZE 300
#define SEGMENTS 100

// internal use type to be able to save main variables in static memory
typedef struct {
    void **com;
    void **proc;
} _sg_t;

static _sg_t _sgt;

// initialize for scatter/gather. Must be called in main() before scatter/gather
//    can be used.
#define initsg()     void * com; void * proc; _sgt.com = &com; _sgt.proc = &proc;

/* create_shared_memory:
 *  create an area of shared memory for processes
 *  param size: bytes to allocate
 *  return void* pointer to shared memory area
 *  see also https://stackoverflow.com/questions/5656530/how-to-use-shared-memory-with-linux-in-c
 */
void* _create_shared_memory(size_t size) {
  // Our memory buffer will be readable and writable:
  int protection = PROT_READ | PROT_WRITE;

  // The buffer will be shared (meaning other processes can access it), but
  // anonymous (meaning third-party processes cannot obtain an address for it),
  // so only this process and its children will be able to use it:
  int visibility = MAP_SHARED | MAP_ANONYMOUS;

  // The remaining parameters to `mmap()` are not important for this use case,
  // but the manpage for `mmap` explains their purpose.
  return mmap(NULL, size, protection, visibility, -1, 0);
}


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
int scatter(const void *init_data, const int segments, void **proc_data, const int length) {
    void **common_data = _sgt.com;
    void *_proc_data = *_sgt.proc;
    // create an area of shared memory and write it to common_data
    *common_data = _create_shared_memory(length);
    // copy init_data into the shared memory area
    memcpy(*common_data, init_data, length);

    int segment_size = length / segments;

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


/* gather: combine the processes to one single process.
 *
 */
void* gather(int segments, int segment_no, void **collected, const int length) {
    void **common_data = _sgt.com;
    void *proc_data = *_sgt.proc;
    int segment_size = length / segments;
    if (segment_no < segments - 1) {
        // lock on
        memcpy(*common_data + segment_no * segment_size, proc_data, 
                segment_size);
        // lock off
        exit(0); // end the child's existence
    } else {
        // wait for all childs to exit
        int returnStatus; 
        waitpid(-1, &returnStatus, 0); 
        if (returnStatus == -1) {
            exit(0); // something failed at wait.
        }  

        memcpy(*common_data + (segments-1)*segment_size, proc_data, 
                segment_size);
        return *common_data;
    }
}

void chartest() {

    char *init_ = "1234567891234560";
    //char init_[ARRAY_SIZE];
    int segments = 4;
    char * proc_;
    int segment_no = scatter(init_, segments, (void**)&proc_, ARRAY_SIZE);

    if (segment_no == -1) {
        printf("Error during scatter.\n");
        return;
    }

    proc_[0] = '_';
    proc_[1] = ' ';

    __sync_synchronize();

    char * collect = gather(segments, segment_no, (void**)&collect, ARRAY_SIZE);

    printf("result: %s\n", collect);

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
    int segment_no = scatter(init_, segments, (void**)&proc_, ARRAY_SIZE*sizeof(int));

    if (segment_no == -1) {
        printf("Error during scatter.\n");
        return;
    }

    proc_[0] = 666;
    proc_[1] = 666;
    proc_[2] = 42;

    int * collect = gather(segments, segment_no, (void**)&collect, ARRAY_SIZE*sizeof(int));

    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("%d\t", collect[i]);
    }

    printf("\n");

}


int main() {
    initsg();

    
    chartest();
    inttest();
    return 0;
}
