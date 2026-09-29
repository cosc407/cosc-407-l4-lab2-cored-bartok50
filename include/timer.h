/* COSC 407/507 -- wall-clock timer used by every lab this term.
 *
 * now_seconds() returns a monotonic wall-clock reading in seconds. Monotonic
 * matters: the wall clock can jump backwards when the system clock is adjusted,
 * and a negative interval in a timing table is a bug you will be asked about.
 *
 * Always time an interval as (now_seconds() - t0), never an absolute value.
 *
 * ---------------------------------------------------------------------------
 * NEW IN LAB 2: cpu_seconds(), the CPU time this PROCESS has consumed, added
 * over all of its threads. Wall-clock time answers "how long did I wait"; CPU
 * time answers "how much machine did I burn getting there". In Lab 1 the two
 * were the same story. In Lab 2 they are not, and the ratio
 *
 *      cpu / wall
 *
 * is the most useful number on the page: it is roughly how many cores were
 * busy, on average, for the whole run. Eight threads that sit blocked on a
 * condition variable cost almost no CPU; eight threads that spin waiting for
 * each other cost eight cores' worth. Both can print the same wall time.
 * ---------------------------------------------------------------------------
 */
#ifndef COSC407_TIMER_H
#define COSC407_TIMER_H

#include <time.h>

static inline double now_seconds(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + 1e-9 * (double)ts.tv_nsec;
}

/* Process CPU time, all threads added together, in seconds.
 *
 * CLOCK_PROCESS_CPUTIME_ID is the right clock and it exists on Linux, in the
 * lab container, and in MinGW-w64's pthreads. The fallback is clock(), which
 * really does measure process CPU time on POSIX -- but on Windows measures
 * wall-clock time instead, so a machine that lands in the fallback will report
 * cpu == time for everything. If you ever see that, say so in your report
 * rather than concluding that nothing spins.
 */
static inline double cpu_seconds(void)
{
#ifdef CLOCK_PROCESS_CPUTIME_ID
    struct timespec ts;
    if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts) == 0) {
        return (double)ts.tv_sec + 1e-9 * (double)ts.tv_nsec;
    }
#endif
    return (double)clock() / (double)CLOCKS_PER_SEC;
}

#endif /* COSC407_TIMER_H */
