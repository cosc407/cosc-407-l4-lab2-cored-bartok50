/* Lab 2 sealed core -- the barrier interface. GIVEN; nothing to change here.
 *
 * Three implementations of one thing: a barrier that n threads can pass
 * through, over and over again, for as many rounds as the driver asks for.
 *
 *   bar_given   src/given.c   the code you were handed. DO NOT EDIT: hashed.
 *   bar_fixed   src/fixed.c   your minimal correction.
 *   bar_alt     src/alt.c     the alternative named in BRIEF.md.
 *
 * Each of the three fills in one bar_ops_t: three function pointers and a
 * name. src/main.c takes one of them and runs exactly the same phased workload
 * through it, so the three columns in your table differ only in the barrier.
 * The bar_ops_t line is already written at the bottom of every file, including
 * the two you have to finish -- you fill in the functions above it.
 *
 *   create(nthreads)  allocate and initialise a barrier for exactly that many
 *                     threads, and return it. NULL means failure. Called once,
 *                     by the driver, before any thread exists.
 *   wait(b)           called by each of the nthreads threads, once per phase.
 *                     Returns only when all nthreads of them have arrived.
 *                     THIS ONE HAS TO WORK EVERY TIME IT IS CALLED, not just
 *                     the first time.
 *   destroy(b)        release everything create() took. Called once, after
 *                     every thread has been joined.
 *
 * ---------------------------------------------------------------------------
 * What a barrier has to promise, in one line, because your report will have to
 * state it: when wait() returns to any thread, every thread has arrived; and
 * no thread can be released from round r+1 before every thread has left
 * round r.
 *
 * The second half of that sentence is the whole of Lab 2. A barrier that keeps
 * only the first half is correct exactly once.
 * ---------------------------------------------------------------------------
 */
#ifndef COSC407_BARRIER_H
#define COSC407_BARRIER_H

/* 128, not Lab 1's 64: one of the four cores asks you to run several times
 * more threads than the machine has cores, and on a 20-core box 64 is not
 * several times. Nothing here allocates 128 of anything until you ask for it. */
#define MAX_THREADS 128

typedef struct {
    const char *name;
    void *(*create)(int nthreads);
    void  (*wait)(void *b);
    void  (*destroy)(void *b);
} bar_ops_t;

extern const bar_ops_t bar_given;
extern const bar_ops_t bar_fixed;
extern const bar_ops_t bar_alt;

#endif /* COSC407_BARRIER_H */
