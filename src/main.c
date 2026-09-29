/* Lab 2 sealed core -- the driver. GIVEN and complete. No marks in this file,
 * and nothing here to change; read the five paragraphs below and move on.
 *
 *   ./bar given 8 2000        the barrier you were handed
 *   ./bar fixed 8 2000        your correction
 *   ./bar alt   8 2000        the alternative in BRIEF.md
 *   ./bar all   8 2000        all three, in that order
 *
 * THE WORKLOAD. Every thread does the same amount of work in every round --
 * WORK_PER_THREAD elements of a shared read-only array, summed into an
 * accumulator it owns. The work per thread is FIXED, so adding threads adds
 * work: that is deliberate. We are measuring the barrier, not the arithmetic,
 * and this way the compute part of a round costs about the same at 1 thread
 * and at 8, so whatever grows in the timing column is synchronisation.
 *
 * THE INVARIANT, which is what makes this a test of a barrier rather than of
 * arithmetic. Each round, every thread writes the round number into its own
 * slot, waits, and then reads all the other slots. If the barrier is a
 * barrier, every slot it reads says r. A thread that has been let into round
 * r+1 early writes r+1 into its slot, and somebody sees it. `bad` counts those
 * sightings, over every thread and every round. `firstbad` is the lowest round
 * in which one was seen -- and it is the single most useful number this
 * program prints. A barrier that is not reusable is perfect in round 0.
 *
 * TWO waits per round, not one. The first says "everybody has written". The
 * second says "everybody has read, so it is safe to write the next round".
 * Take either one out and the check can fail against a perfectly good barrier.
 * You will be asked about this.
 *
 * THE CHECKSUM is the arithmetic: the whole array summed once per round. It
 * catches a thread that skipped its work or ran a round twice, which a wrong
 * `bad` count does not.
 *
 * THE WATCHDOG. A broken barrier does not always give a wrong answer; often it
 * just stops, and in Lab 2 that is a result and not a crash. So each run is
 * watched: if it has not finished after BAR_WATCHDOG seconds (30 in this core,
 * override with the environment variable), the driver prints its line with
 * deadlock=yes and exits. That line is a measurement -- paste it. One
 * consequence to know before it bites you: the whole process exits, so if
 * `all` deadlocks on `given` you never see `fixed` and `alt`. Run the modes one
 * at a time when that happens.
 *
 * `cpu` is process CPU time over all threads; `time` is wall clock. cpu/time
 * is roughly how many cores were busy. See include/timer.h.
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "timer.h"
#include "barrier.h"

#define WORK_PER_THREAD 2000     /* array elements each thread sums per round */
#define SLOT_PAD        64       /* bytes: one slot per cache line            */

/* One slot per thread, each on its own cache line. Padded ON PURPOSE: false
 * sharing between the slots was Lab 1's problem and would only add noise to
 * this one.
 *
 * `volatile` is not synchronisation -- Lab 1 said so and it is still true. It
 * is here for one narrow reason: it stops the compiler from keeping a slot it
 * has already read in a register across the call to wait(). The ordering
 * between one thread's write and another thread's read is the barrier's job,
 * which is exactly what is on trial. */
typedef union {
    volatile long long round;
    char pad[SLOT_PAD];
} slot_t;

typedef struct {
    int              id;
    int              nthreads;
    long long        rounds;
    const long long *values;      /* shared, read-only                      */
    long long        begin, end;  /* this thread's slice, [begin, end)      */
    slot_t          *slots;       /* shared: nthreads of them               */
    const bar_ops_t *ops;
    void            *bar;

    /* written by this thread only, read by main after the join */
    long long        acc;
    long long        bad;
    long long        firstbad;    /* -1 if none */
} task_t;

static void *worker(void *p)
{
    task_t *t = (task_t *)p;

    for (long long r = 0; r < t->rounds; r++) {
        /* this round's work: my own slice, into my own accumulator */
        long long s = 0;
        for (long long i = t->begin; i < t->end; i++) {
            s += t->values[i];
        }
        t->acc += s;

        t->slots[t->id].round = r;      /* announce where I am */

        t->ops->wait(t->bar);           /* everybody has announced */

        for (int j = 0; j < t->nthreads; j++) {
            if (t->slots[j].round != r) {
                t->bad++;
                if (t->firstbad < 0) {
                    t->firstbad = r;
                }
            }
        }

        t->ops->wait(t->bar);           /* everybody has read: safe to go on */
    }

    return NULL;
}

/* ------------------------------------------------------------ reporting ---
 * Both the normal path and the watchdog come through emit(), and exactly one
 * of them gets to print. */
static pthread_mutex_t emit_lock = PTHREAD_MUTEX_INITIALIZER;
static int             emitted;

static void emit(const char *mode, int threads, long long rounds,
                 long long bad, long long firstbad, const char *checksum,
                 int correct, int deadlock, double time_s, double cpu_s)
{
    pthread_mutex_lock(&emit_lock);
    if (!emitted) {
        emitted = 1;
        printf("mode=%s threads=%d rounds=%lld bad=%lld firstbad=%lld "
               "checksum=%s correct=%s deadlock=%s time=%.4f cpu=%.4f\n",
               mode, threads, rounds, bad, firstbad, checksum,
               correct ? "yes" : "no", deadlock ? "yes" : "no", time_s, cpu_s);
        fflush(stdout);
    }
    pthread_mutex_unlock(&emit_lock);
}

/* ------------------------------------------------------------- watchdog --- */
typedef struct {
    const char  *mode;
    int          threads;
    long long    rounds;
    double       t0, c0, limit;
    volatile int finished;
} watch_t;

static void *watchdog(void *p)
{
    watch_t *w = (watch_t *)p;

    while (!w->finished && now_seconds() - w->t0 < w->limit) {
        struct timespec slice = { 0, 50 * 1000 * 1000 };   /* 50 ms */
        nanosleep(&slice, NULL);
    }
    if (w->finished) {
        return NULL;
    }

    /* Still running, and out of time. Nothing can be said about bad or the
     * checksum, so they are reported as unknown. */
    emit(w->mode, w->threads, w->rounds, -1, -1, "unknown", 0, 1,
         now_seconds() - w->t0, cpu_seconds() - w->c0);
    fprintf(stderr, "%s: no progress after %.1f s -- giving up. This is a "
                    "result, not a crash: paste the line above.\n",
            w->mode, w->limit);
    fflush(stderr);
    _Exit(3);
    return NULL;                                           /* not reached */
}

static double watchdog_limit(void)
{
    const char *s = getenv("BAR_WATCHDOG");
    double v = (s != NULL) ? atof(s) : 0.0;
    /* 30, not the 5 the other cores use: this core's given barrier is SLOW by
     * design -- a few seconds a run, and far more if you raise the rounds -- so
     * a 5 s limit would report a program that is working as a program that has
     * stopped. The limit is here to catch a barrier that will never finish, not
     * to put a budget on one that is merely slow. */
    return (v > 0.0) ? v : 30.0;
}

/* ------------------------------------------------------------- one run ---- */
static void one_run(const bar_ops_t *ops, const long long *values,
                    int nthreads, long long rounds, long long expected)
{
    pthread_t tid[MAX_THREADS];
    task_t    task[MAX_THREADS];
    pthread_t wd;
    watch_t   w;

    slot_t *slots = calloc((size_t)nthreads, sizeof *slots);
    void   *bar   = ops->create(nthreads);

    emitted = 0;
    if (slots == NULL || bar == NULL) {
        /* An unfinished create() returns NULL, so this is the line you get
         * before you have written anything. It is still a line: every run of
         * this program reports something. */
        emit(ops->name, nthreads, rounds, -1, -1, "nobarrier", 0, 0, 0.0, 0.0);
        fprintf(stderr, "%s: create() returned NULL -- nothing was run\n",
                ops->name);
        free(slots);
        return;
    }

    w.mode = ops->name; w.threads = nthreads; w.rounds = rounds;
    w.t0 = now_seconds(); w.c0 = cpu_seconds();
    w.limit = watchdog_limit(); w.finished = 0;
    pthread_create(&wd, NULL, watchdog, &w);

    for (int i = 0; i < nthreads; i++) {
        task[i].id       = i;
        task[i].nthreads = nthreads;
        task[i].rounds   = rounds;
        task[i].values   = values;
        task[i].begin    = (long long)i * WORK_PER_THREAD;
        task[i].end      = (long long)(i + 1) * WORK_PER_THREAD;
        task[i].slots    = slots;
        task[i].ops      = ops;
        task[i].bar      = bar;
        task[i].acc      = 0;
        task[i].bad      = 0;
        task[i].firstbad = -1;

        if (pthread_create(&tid[i], NULL, worker, &task[i]) != 0) {
            fprintf(stderr, "pthread_create failed\n");
            exit(1);
        }
    }
    for (int i = 0; i < nthreads; i++) {
        pthread_join(tid[i], NULL);
    }

    double elapsed = now_seconds() - w.t0;
    double cpu     = cpu_seconds() - w.c0;

    w.finished = 1;
    pthread_join(wd, NULL);

    long long total = 0, bad = 0, firstbad = -1;
    for (int i = 0; i < nthreads; i++) {
        total += task[i].acc;
        bad   += task[i].bad;
        if (task[i].firstbad >= 0 && (firstbad < 0 || task[i].firstbad < firstbad)) {
            firstbad = task[i].firstbad;
        }
    }

    int ok_sum = (total == expected);
    emit(ops->name, nthreads, rounds, bad, firstbad, ok_sum ? "ok" : "BAD",
         (bad == 0 && ok_sum), 0, elapsed, cpu);

    ops->destroy(bar);
    free(slots);
}

int main(int argc, char **argv)
{
    if (argc < 4) {
        fprintf(stderr, "usage: %s <given|fixed|alt|all> <nthreads> <rounds>\n",
                argv[0]);
        return 1;
    }

    const char *mode = argv[1];
    int  nthreads    = (int)strtol(argv[2], NULL, 10);
    long long rounds = strtoll(argv[3], NULL, 10);

    if (nthreads < 1 || nthreads > MAX_THREADS || rounds < 1) {
        fprintf(stderr, "bad arguments: 1 <= nthreads <= %d, rounds >= 1\n",
                MAX_THREADS);
        return 1;
    }

    long long n = (long long)nthreads * WORK_PER_THREAD;
    long long *a = malloc((size_t)n * sizeof *a);
    if (a == NULL) {
        fprintf(stderr, "not enough memory for %lld elements\n", n);
        return 1;
    }
    for (long long i = 0; i < n; i++) {
        a[i] = i + 1;
    }

    /* Every thread sums its own slice once per round, so the whole array is
     * summed once per round. n(n+1)/2 is computed a different way from the
     * loops that do the work, so this is a real check. */
    long long expected = rounds * (n * (n + 1) / 2);

    int unknown = 0;
    if      (strcmp(mode, "given") == 0) one_run(&bar_given, a, nthreads, rounds, expected);
    else if (strcmp(mode, "fixed") == 0) one_run(&bar_fixed, a, nthreads, rounds, expected);
    else if (strcmp(mode, "alt")   == 0) one_run(&bar_alt,   a, nthreads, rounds, expected);
    else if (strcmp(mode, "all")   == 0) {
        one_run(&bar_given, a, nthreads, rounds, expected);
        one_run(&bar_fixed, a, nthreads, rounds, expected);
        one_run(&bar_alt,   a, nthreads, rounds, expected);
    } else {
        fprintf(stderr, "unknown mode '%s'\n", mode);
        unknown = 1;
    }

    free(a);
    return unknown;
}
