# Sealed core **D**

Read `lab2-core.md` first. Other sections were given a different core.

**Every total this program prints is right.** Stop checking the answer; there is
nothing wrong with it. Nothing in this core is a race, and no test of
correctness will ever fail.

**Use `rounds = 100` everywhere in your report** — `given.c`'s header says why
one round is expensive, and a hundred of them is already several seconds.

```sh
make
./bar given 1 100
./bar given 8 100          # be patient, and watch the clock, not the answer
```

**`given` is slow, and it is not stuck.** Telling those two apart is part of
S2.1 and you cannot do it from the clock alone: a program that has stopped is
still stopped at thirty seconds, and a slow one is not. The watchdog here is set
to thirty for that reason, so if you ever do see `deadlock=yes` you have found
something real — raise the limit (`BAR_WATCHDOG=120 ./bar given 8 2000`) and say
which of the two it turned out to be. `cpu` next to `time` is the other half of
the answer: a thread that is asleep and a thread that is spinning look identical
on the clock and nothing alike in that ratio.

**The alternative** (`src/alt.c`): the opposite extreme — **no lock, no sleep,
no kernel.** An atomic counter, a sense flag that flips every round so nothing
has to be reset, and a loop that reads the flag until it changes. (Mellor-Crummey
and Scott 1991; Pacheco §4.8 calls the same trick a flag.) It is correct, and it
will be the fastest thing you measure today. Measure it at more threads than you
have cores before you decide you believe that.

**Your four questions, for this core**

- **S2.1 mechanism.** Three of the author's claims about a sleeping thread are
  true. Say which one is false and put a number on it: what does one round of
  this program actually cost, what should it cost, and where did the difference
  come from? `cpu` and `time` together tell you which — use both.
- **S2.2 proof.** A measurement, in two parts. (1) `given`'s time per round at
  1, 2, 4, 8 threads: it does not grow with threads, and saying why not is half
  the marks. (2) The sweep: `given.c`'s poll interval is a compile-time constant
  you can change **without editing the file**,
  `make clean && make EXTRA=-DBAR_POLL_NS=50000`. Sweep 1 ms, 200 µs, 50 µs,
  10 µs and report `time` **and `cpu`** at each. Say where the time stops
  falling and what has taken over by then. Two practical notes: **raise the
  rounds** for the sweep until `time` is at least a tenth of a second, because
  `cpu` is only measured to about a sixtieth of a second and a shorter run
  reports `cpu=0.0000` whatever it did; and *watch `cpu` in the last rows*,
  because below some interval your platform stops sleeping at all, and that is
  a finding rather than an error.
- **S2.3 minimality.** Keep the counter, the generation number and the mutex;
  change only how a waiting thread waits. Say how many lines that was, and then
  both directions: what is the least that could work, and name two things about
  a condition variable that are **not** optional — the author of `given.c`
  avoided both of them by not using one at all.
- **S3.2 ship it.** Your `alt` is likely the fastest thing in your table and
  your `fixed` is second. Say which ships, why the fastest one might not, and
  **what measurement would change your mind.** No second half, half the marks.

**One question you may get in the oral.** *This barrier is correct, uses almost
no CPU, and is three hundred times slower than your fix. Where did the time go,
and who was holding it?*
