/* Lab 2 -- YOUR OWN unit tests for bar_fixed and bar_alt.
 *
 * Unlike src/main.c, which drives the whole workload through ./bar, these
 * tests call create()/wait()/destroy() directly on a small, controlled
 * scenario -- no subprocess, no parsing printed output, and no timing noise.
 *
 * EVERY TEST HAS A TIMEOUT (`.timeout = BAR_TIMEOUT` below), the same
 * protection tests/test_mysem.c uses in Part A: if a barrier you wrote
 * deadlocks, Criterion reports "Timed out" instead of hanging your terminal
 * or this check. That line IS useful information -- it means some thread is
 * waiting for a release that will never come.
 *
 * One helper is given, below, and you do not need to change it:
 *
 *   run_reuse_check(ops, nthreads, rounds)  runs several threads through
 *       several rounds and returns how many times a thread saw a slot from
 *       the wrong round. 0 means the barrier held everyone back correctly,
 *       every round.
 *
 * One worked example is done for each of bar_fixed and bar_alt. Write at
 * least TWO MORE cases per function, covering:
 *   - a thread count of 1 (the trivial case: nothing to actually wait for)
 *   - more threads than this machine has cores (barrier.h's MAX_THREADS=128
 *     exists for exactly this -- a barrier that only works when every thread
 *     gets a real core is not a barrier)
 *
 * Build and run: make unit-test
 */
#include <criterion/criterion.h>

#include <pthread.h>
#include <stdlib.h>

#include "barrier.h"

#define BAR_TIMEOUT      5.0
#define BAR_TIMEOUT_SLOW 20.0  /* spin/poll barriers under heavy oversubscription can be genuinely, correctly slow -- not stuck */

/* Shared state for one run of the reusability check below. Each thread
 * writes the round number into its own slot, then (after both waits) every
 * thread's slot must say the SAME round -- see main.c's own comment on why
 * this needs two waits per round, not one. */
typedef struct {
    const bar_ops_t *ops;
    void             *bar;
    int               id;
    int               nthreads;
    long long         rounds;
    volatile int     *slot;
    int               bad;
} task_t;

static void *reuse_worker(void *p)
{
    task_t *t = (task_t *)p;
    for (long long r = 0; r < t->rounds; r++) {
        t->slot[t->id] = (int)r;
        t->ops->wait(t->bar);                 /* everyone has written */
        for (int j = 0; j < t->nthreads; j++) {
            if (t->slot[j] != (int)r) {
                t->bad++;
            }
        }
        t->ops->wait(t->bar);                 /* everyone has read, safe to write r+1 */
    }
    return NULL;
}

/* GIVEN. run_reuse_check(ops, nthreads, rounds) -> total bad sightings over
 * everyone. 0 means the barrier held every thread back on every round,
 * exactly as many times as it was reused. */
static int run_reuse_check(const bar_ops_t *ops, int nthreads, long long rounds)
{
    void         *bar  = ops->create(nthreads);
    pthread_t     tid[MAX_THREADS];
    task_t        task[MAX_THREADS];
    volatile int  slot[MAX_THREADS];

    cr_assert_not_null(bar, "create() returned NULL");

    for (int i = 0; i < nthreads; i++) {
        task[i].ops = ops;
        task[i].bar = bar;
        task[i].id = i;
        task[i].nthreads = nthreads;
        task[i].rounds = rounds;
        task[i].slot = slot;
        task[i].bad = 0;
        pthread_create(&tid[i], NULL, reuse_worker, &task[i]);
    }
    int total_bad = 0;
    for (int i = 0; i < nthreads; i++) {
        pthread_join(tid[i], NULL);
        total_bad += task[i].bad;
    }
    ops->destroy(bar);
    return total_bad;
}

/* ---------------------------------------------------------- bar_fixed --- */

/* WORKED EXAMPLE -- the pattern every case below follows: run several
 * threads through several rounds, assert nobody ever saw a stale slot. */
Test(fixed, reusable_across_several_rounds, .timeout = BAR_TIMEOUT)
{
    cr_assert_eq(run_reuse_check(&bar_fixed, 4, 20), 0,
                 "a reused bar_fixed let a thread through before everyone "
                 "had left the previous round");
}

/* TODO: single_thread -- nthreads=1. Nothing to actually wait for, but
 * create()/wait()/destroy() still have to work. */
Test(fixed, single_thread, .timeout = BAR_TIMEOUT)
{
    cr_assert_fail("TODO: write this case -- see the worked example above");
}

/* TODO: more_threads_than_cores -- pick an nthreads well above what this
 * machine actually has (nproc), still under MAX_THREADS. This is exactly
 * the case barrier.h's comment on MAX_THREADS=128 exists for. */
Test(fixed, more_threads_than_cores, .timeout = BAR_TIMEOUT_SLOW)
{
    cr_assert_fail("TODO: write this case -- see the worked example above");
}

/* ------------------------------------------------------------ bar_alt --- */

/* WORKED EXAMPLE -- same pattern as bar_fixed. This core's alt is supposed
 * to be a correct (if different) barrier -- see BRIEF.md and
 * tests/expect.env's ALT_CORRECT. */
Test(alt, reusable_across_several_rounds, .timeout = BAR_TIMEOUT)
{
    cr_assert_eq(run_reuse_check(&bar_alt, 4, 20), 0,
                 "a reused bar_alt let a thread through before everyone "
                 "had left the previous round");
}

/* TODO: single_thread -- nthreads=1. */
Test(alt, single_thread, .timeout = BAR_TIMEOUT)
{
    cr_assert_fail("TODO: write this case -- see the worked example above");
}

/* TODO: more_threads_than_cores -- same idea as bar_fixed's version above. */
Test(alt, more_threads_than_cores, .timeout = BAR_TIMEOUT_SLOW)
{
    cr_assert_fail("TODO: write this case -- see the worked example above");
}
