# Lab 2 — the sealed core

**100 marks · due at the end of your period · no AI, none, see `lab2-part-a-public.md` · 1 page**

**At 0:10 read only three things: this page, `BRIEF.md`, and `src/given.c`.**
That is about six minutes of the ten, which leaves four to think and write.
`README.md` and the two forms can wait until after 0:20 — they hold nothing
you need in order to predict.

## The three files

| | |
|---|---|
| `src/given.c` | somebody's **reusable barrier**, with **exactly one defect**. Its header says what they believed; one of those claims is false. **Do not edit it** — it is hashed, and your report compares against it. |
| `src/fixed.c` | **your minimal correction.** Keep this file: Lab 3 hunts in it. |
| `src/alt.c` | **the alternative `BRIEF.md` names.** Not necessarily better; in one core not even correct. |

`src/main.c` is given: it runs `rounds` phases across your barrier, and in each
phase every thread announces which round it is in and then checks that
everybody else agrees. `bad` counts the disagreements; `firstbad` is the
earliest round in which one appeared; `checksum` is the arithmetic, which can
be right while `bad` is not.

```
./bar given 8 2000        # also: fixed, alt, or all
mode=given threads=8 rounds=2000 bad=111726 firstbad=0 checksum=ok correct=no deadlock=no time=0.0061 cpu=0.0938
```

**There are two waits in every round**, one after the announcement and one
after the check. Both are necessary, you will be asked why, and the count
matters: a barrier that survives one use has already been used twice by the end
of round 0.

## What a barrier promises

Write this down before you write any code, because S2.1 asks for it: *when
`wait()` returns to any thread, every thread has arrived; **and** no thread can
be released from round r+1 before every thread has left round r.* Most broken
barriers keep the first half. The second half is the whole of Lab 2.

## First move: which kind of wrong is it?

Run it at one thread, then at two, then at eight. A barrier has **three** ways
to be wrong, not two, and telling them apart is the most expensive decision
available to you today:

| | what you see | how you diagnose it |
|---|---|---|
| **wrong** | `bad > 0`, `correct=no` | by argument: an interleaving, or a counting argument |
| **stopped** | `deadlock=yes` from the watchdog, and `cpu` near zero | by argument: who is asleep, and what are they waiting for |
| **slow** | `correct=yes` every time | only by a **timing table**, because every correctness test passes |

`deadlock=yes` is the watchdog giving up, not a diagnosis. **A program that has
stopped is still stopped at thirty seconds**; a slow one is not.
`BAR_WATCHDOG=30 ./bar given 8 2000` is how you tell, and `cpu`/`time` is how
you tell a thread that is asleep from a thread that is spinning.

## What to hand in

| | | marks |
|---|---|---|
| **S1** | `PREDICTION.md`, **pushed by 0:20 on its own commit**, before you compile. Marked on having predicted and on reconciling it in S3.1 — **not on being right.** | 15 |
| **S2** | The diagnosis in `RESULTS.md`: state the invariant, name the mechanism, prove it in the form your brief requires, and argue your fix is minimal — what breaks with less, what it costs with more. | 40 |
| **S3** | Twelve runs — all three modes at 1, 2, 4, 8 threads — pasted and tabulated, with `cpu` as well as `time`. Reconcile with S1. Then: which would you ship, **and what measurement would change your mind?** | 30 |
| **S4** | Explain-back, two or three sentences: what was wrong, and what did fixing it cost? | 15 |

Ask for more threads than you have cores. That is deliberate, and in one of the
four cores it is the whole answer.

## The clock

0:10 read · **0:20 push the prediction** · 0:50 S2 written while the evidence is
in front of you · 1:10 the twelve runs and S3 · 1:20 S4, `make test`, push.

**Still hunting at 0:50? Ask your TA.** It costs you nothing and a hint then is
worth far more than a blank S3. If you never find it, hand in what you ruled out
and how — a well-argued failed diagnosis earns real marks; a blank page does not.

`lab mark = automated mark × oral multiplier`, capped at 100. `make test` runs
exactly the machine-checked part, which is about 47 of the 100.
