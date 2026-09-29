/* COSC 407/507 Lab 2, core D -- the barrier you were handed.
 *
 * *** DO NOT EDIT. *** It is hashed by the autograder, and your report has to
 * compare against the code you were handed. Work in fixed.c and alt.c.
 *
 * ---------------------------------------------------------------------------
 * What the author believed, in their own words:
 *
 *   "Condition variables are easy to get wrong -- a signal instead of a
 *    broadcast, a predicate that is checked once instead of in a loop, a
 *    wake-up that arrives before the wait -- and every one of those bugs is a
 *    program that stops dead. So this barrier does not use one. A counter and
 *    a generation number under a mutex, and a thread that has to wait just
 *    drops the lock, sleeps for a millisecond, and looks again.
 *
 *    It cannot lose a wake-up, because there is no wake-up to lose. A sleeping
 *    thread costs no CPU at all. And a millisecond is nothing -- the threads
 *    are only ever a few microseconds apart, so a waiter will almost never go
 *    round the loop more than once."
 *
 * Exactly one of the claims in that paragraph is false. Every total this
 * program prints is right, so it is not that one.
 * ---------------------------------------------------------------------------
 *
 * BAR_POLL_NS is the poll interval, and it can be changed WITHOUT editing this
 * file -- which matters, because this file is hashed:
 *
 *     make clean && make EXTRA=-DBAR_POLL_NS=50000     (50 microseconds)
 *
 * Your brief asks you to sweep it. `make verify` will still pass afterwards,
 * because the bytes have not changed; only the command line has.
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "barrier.h"

#ifndef BAR_POLL_NS
#define BAR_POLL_NS (1000L * 1000L)      /* one millisecond */
#endif

typedef struct {
    pthread_mutex_t lock;
    int             n;        /* how many threads have to arrive  */
    int             count;    /* how many have arrived this round */
    unsigned long   gen;      /* which round this barrier is on   */
} bar_t;

static void *create(int nthreads)
{
    bar_t *b = malloc(sizeof *b);
    if (b == NULL) {
        return NULL;
    }
    if (pthread_mutex_init(&b->lock, NULL) != 0) {
        fprintf(stderr, "barrier init failed\n");
        free(b);
        return NULL;
    }
    b->n     = nthreads;
    b->count = 0;
    b->gen   = 0;
    return b;
}

static void wait_(void *p)
{
    bar_t *b = (bar_t *)p;
    struct timespec poll = { 0, BAR_POLL_NS };

    pthread_mutex_lock(&b->lock);

    unsigned long mine = b->gen;      /* the round I am waiting to leave */

    b->count++;
    if (b->count == b->n) {
        b->count = 0;                 /* re-arm for the next round */
        b->gen++;                     /* this round is over        */
        pthread_mutex_unlock(&b->lock);
        return;
    }

    /* Not the last one in, so wait -- by looking, sleeping, and looking again.
     * The lock is dropped before the sleep, which it has to be: holding it
     * would stop the thread we are waiting for from ever arriving. */
    while (b->gen == mine) {
        pthread_mutex_unlock(&b->lock);
        nanosleep(&poll, NULL);
        pthread_mutex_lock(&b->lock);
    }

    pthread_mutex_unlock(&b->lock);
}

static void destroy(void *p)
{
    bar_t *b = (bar_t *)p;
    pthread_mutex_destroy(&b->lock);
    free(b);
}

const bar_ops_t bar_given = { "given", create, wait_, destroy };
