/* Lab 2, core D -- THE ALTERNATIVE named in BRIEF.md.
 *
 * The opposite extreme: no lock, no sleep, no kernel. An atomic counter, a
 * sense flag that flips every round so that nothing ever has to be reset, and
 * a loop that reads the flag until it changes. The standard centralised
 * sense-reversing barrier (Mellor-Crummey and Scott 1991; Pacheco 4.8 calls
 * the same trick a flag).
 *
 *   #include <stdatomic.h>, then atomic_int, atomic_fetch_add, atomic_load,
 *   atomic_store. The sense a thread is waiting for is per-thread, not shared:
 *   `static _Thread_local int my_sense;` is the tidy way to keep it.
 *
 * It is correct and it will be the fastest thing in your table. Measure it at
 * more threads than you have cores before you decide that settles anything.
 * Do not tune it: it is evidence, not a submission.
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "barrier.h"

/* TODO: the barrier's state. What has to be shared between the threads, and
 *       what does each thread have to remember for itself? */

static void *create(int nthreads)
{
    /* TODO: allocate it, initialise everything, and return it. Anything a
     *       thread might lock or wait on has to be ready BEFORE the first
     *       thread can reach it. */
    (void)nthreads;
    return NULL;
}

static void wait_(void *p)
{
    /* TODO: the barrier. Write the invariant you are keeping in a comment
     *       above it, in one line, before you write the code -- your report
     *       and your oral both ask you to state it. */
    (void)p;
}

static void destroy(void *p)
{
    /* TODO: release what create() took. Every thread has been joined by the
     *       time this is called. */
    (void)p;
}

const bar_ops_t bar_alt = { "alt", create, wait_, destroy };
