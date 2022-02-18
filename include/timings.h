#ifndef TIMINGS_H
#define TIMINGS_H
/* get the current second
 *
 */
long get_time_base();


/* get microseconds since last basesecond
 *
 */
long get_time_mus(long basesecond);
#endif