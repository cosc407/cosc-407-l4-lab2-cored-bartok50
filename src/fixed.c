/* Lab 2, core D -- YOUR MINIMAL CORRECTION.
 *
 * wait_() must work every time it is called, at 1, 2, 4 and 8 threads, and at
 * more threads than this machine has cores.
 *
 * THE CONSTRAINT FROM YOUR BRIEF: keep the counter, keep the generation
 * number, keep the mutex, keep the shape of the round. Change only how a
 * waiting thread waits. The state given.c already has is exactly the state the
 * replacement needs, which is worth noticing and worth a sentence in S2.3.
 *
 * Two things about the replacement are not optional. Name both in S2.3 -- the
 * author of given.c avoided both of them by not using one at all, and two of
 * the other sections spent their period on one of the two.
 *
 * Copy anything you like out of given.c. Do not edit it.
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

const bar_ops_t bar_fixed = { "fixed", create, wait_, destroy };
