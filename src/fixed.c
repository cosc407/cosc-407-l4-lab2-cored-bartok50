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
#include <time.h>

#include "barrier.h"

/* TODO: the barrier's state. What has to be shared between the threads, and
 *       what does each thread have to remember for itself? */


#ifndef BAR_POLL_NS
#define BAR_POLL_NS (1000L * 1000L)      /* one millisecond */
#endif

typedef struct {
    pthread_cond_t cv;
    pthread_mutex_t lock;
    int             n;        /* how many threads have to arrive  */
    int             count;    /* how many have arrived this round */
    unsigned long   gen;      /* which round this barrier is on   */
} bar_t;


static void *create(int nthreads)
{
    /* TODO: allocate it, initialise everything, and return it. Anything a
     *       thread might lock or wait on has to be ready BEFORE the first
     *       thread can reach it. */

        bar_t *b = malloc(sizeof *b);
    if (b == NULL) {
        return NULL;
    }
    if (pthread_mutex_init(&b->lock, NULL) != 0) {
        fprintf(stderr, "barrier init failed\n");
        free(b);
        return NULL;
    }

    if(pthread_cond_init(&b->cv, NULL) != 0){
         fprintf(stderr, "Condition init failed\n");
        free(b);
        return NULL;
    }

    b->n     = nthreads;
    b->count = 0;
    b->gen   = 0;
    return b;

    (void)nthreads;
    return NULL;
}

static void wait_(void *p)
{
    /* TODO: the barrier. Write the invariant you are keeping in a comment
     *       above it, in one line, before you write the code -- your report
     *       and your oral both ask you to state it. */

    //Kept most of the if statement the same as well as how count and gen work

    bar_t *b = (bar_t *)p;
    struct timespec poll = { 0, BAR_POLL_NS };

    pthread_mutex_lock(&b->lock);

    b->count++;
    if (b->count == b->n) {
        b->count = 0;                 /* re-arm for the next round */
        b->gen++;              
        pthread_cond_broadcast(&b->cv);       /* this round is over        */
        pthread_mutex_unlock(&b->lock);
        return;
    }
    
    nanosleep(&poll, NULL);
    pthread_cond_wait(&b->cv, &b->lock);


    pthread_mutex_unlock(&b->lock);
            


}

static void destroy(void *p)
{
    /* TODO: release what create() took. Every thread has been joined by the
     *       time this is called. */
    bar_t *b = (bar_t *)p;
    pthread_mutex_destroy(&b->lock);
    pthread_cond_destroy(&b->cv);
    free(b);
}

const bar_ops_t bar_fixed = { "fixed", create, wait_, destroy };
