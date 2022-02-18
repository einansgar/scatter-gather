#include <sys/time.h>
#include <stdlib.h>
#include "include/timings.h"

long get_time_base() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}

long get_time_mus(long basesecond) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long seconds = tv.tv_sec - basesecond;
    long microseconds = tv.tv_usec;
    return seconds * 1000000L + microseconds;
}