# Lab 2 — sealed core

**Read `lab2-core.md`, then `BRIEF.md`.** All 100 marks are here. Due at the end
of your lab period; no late submission.

**No AI in the lab. None** — not the C, not the prose, not "explain this error",
not the toolchain. Documentation, the textbooks, the notes and your TA are fine.
Declare what you used in `RESULTS.md` even if it is "none".

```sh
make
./bar given 1 2000        # modes: given, fixed, alt, all
make test                 # what the autograder runs
make verify               # just the "given.c is unmodified" check
```

**Nothing here hangs.** A broken barrier stops rather than lying, so every run
is watched: after `BAR_WATCHDOG` seconds (5 by default) the driver prints its
line with `deadlock=yes` and exits. That line is a measurement — paste it. If
you want to wait longer, `BAR_WATCHDOG=30 ./bar given 8 2000`.

| | |
|---|---|
| `BRIEF.md` | your core. Different in each section. |
| `PREDICTION.md` | **push by 0:20**, on its own commit, before you compile |
| `RESULTS.md` | most of the marks |
| `src/given.c` | **do not edit** — hashed by the autograder |
| `src/fixed.c`, `src/alt.c` | you write these |
| `src/main.c`, `include/` | given and complete |
| `tests/` | the autograder's checks; identical in all four cores |

**Keep `src/fixed.c`.** Lab 3 is a race hunt through four broken programs and
one of the four is your barrier from today. This is the only lab whose output
you are asked to bring to the next one.

```sh
git add PREDICTION.md && git commit -m "prediction" && git push   # 0:20
make test
git add -A && git commit -m "Lab 2 core" && git push              # by 1:20
```

You met ThreadSanitizer in class 5. It is Lab 3's instrument, it roughly
halves the speed of everything, and it says nothing at all about a barrier that
merely stops — so keep it out of the table this lab. Building a second copy by
hand to look is free and legitimate; `make` explains how.

## Unit testing and coverage

`tests/unit_test.c` calls `create()`/`wait()`/`destroy()` on `bar_fixed` and
`bar_alt` directly -- no `./bar`, no subprocess, no watchdog. This is
additive: it doesn't replace anything above, isn't wired into `make test`,
and doesn't affect your 100 marks unless/until it's actually added to
`lab2-rubric.md`.

```sh
make unit-test        # run YOUR OWN tests/unit_test.c against fixed.c/alt.c
make unit-coverage     # how much of fixed.c/alt.c YOUR OWN unit tests exercise
bash tests/run_tests.sh unit       # the count-and-pass check, if this becomes graded
bash tests/run_tests.sh unitcov    # the coverage-threshold check, if this becomes graded
bash tests/run_tests.sh score      # the running total, out of 68 script-checked marks
```

A few things worth knowing:

- `tests/unit_test.c` has one worked example per function already, as a
  model — the rest are `TODO` stubs that fail on purpose until you write
  them. You need at least 3 cases each for `bar_fixed` and `bar_alt`.
- Every test has a timeout (`.timeout = BAR_TIMEOUT`), the same protection
  `tests/test_mysem.c` used in Part A — a barrier that deadlocks is reported
  as `Timed out`, not a hang in your terminal.
- **No mutation testing here, on purpose** — unlike Lab 1, this lab does not
  ship a `make mutation` target.
