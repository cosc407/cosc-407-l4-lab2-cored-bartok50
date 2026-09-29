# Lab 2 results — sealed core

Name:  Jesse Staples
Student number:  32708851
Lab section:  4
Core:  D
Machine:  Codespace 4 core
Cores:  4

## Tools and sources

Tools and sources: Geeks For Geeks, ibm.com

> Mandatory, even if it says "none". **No AI in the lab, at all** — see the
> README. Missing declaration: zero until you supply one. False one: misconduct.

## S2 — the defect · 40 marks

Three or more runs of `./bar given`, including one thread:

```
mode=given threads=1 rounds=100 bad=0 firstbad=-1 checksum=ok correct=yes deadlock=no time=0.0002 cpu=0.0003

mode=given threads=4 rounds=100 bad=0 firstbad=-1 checksum=ok correct=yes deadlock=no time=0.1667 cpu=0.0060

mode=given threads=8 rounds=100 bad=0 firstbad=-1 checksum=ok correct=yes deadlock=no time=0.1981 cpu=0.0139
```

**S2.1** Name the mechanism: which claim in `given.c`'s header is false, and
what is actually happening? State the barrier's invariant and say which half of
it this code does not keep.

The claim that condition statements arent necessary,  

**S2.2** Prove it, in the form your `BRIEF.md` requires.

REPLACE THIS LINE

**S2.3** Minimality: what breaks if you do less, what it costs if you do more.

REPLACE THIS LINE

## S3 — the measurement · 30 marks

`./bar all <t> <rounds>` at 1, 2, 4 and 8 threads. Pasted, not retyped. If a
mode stops, `all` stops with it — run the modes one at a time and paste those.

```
mode=given threads=1 rounds=100 bad=0 firstbad=-1 checksum=ok correct=yes deadlock=no time=0.0003 cpu=0.0003
mode=fixed threads=1 rounds=100 bad=0 firstbad=-1 checksum=ok correct=yes deadlock=no time=0.0002 cpu=0.0002
mode=alt threads=1 rounds=100 bad=-1 firstbad=-1 checksum=nobarrier correct=no deadlock=no time=0.0000 cpu=0.0000
alt: create() returned NULL -- nothing was run


mode=given threads=2 rounds=100 bad=0 firstbad=-1 checksum=ok correct=yes deadlock=no time=0.1152 cpu=0.0024
mode=fixed threads=2 rounds=100 bad=0 firstbad=-1 checksum=ok correct=yes deadlock=no time=0.2176 cpu=0.0072
mode=alt threads=2 rounds=100 bad=-1 firstbad=-1 checksum=nobarrier correct=no deadlock=no time=0.0000 cpu=0.0000
alt: create() returned NULL -- nothing was run


mode=given threads=4 rounds=100 bad=0 firstbad=-1 checksum=ok correct=yes deadlock=no time=0.1782 cpu=0.0074
mode=fixed threads=4 rounds=100 bad=0 firstbad=-1 checksum=ok correct=yes deadlock=no time=0.6529 cpu=0.0255
mode=alt threads=4 rounds=100 bad=-1 firstbad=-1 checksum=nobarrier correct=no deadlock=no time=0.0000 cpu=0.0000
alt: create() returned NULL -- nothing was run


mode=given threads=8 rounds=100 bad=0 firstbad=-1 checksum=ok correct=yes deadlock=no time=0.2007 cpu=0.0140
mode=fixed threads=8 rounds=100 bad=0 firstbad=-1 checksum=ok correct=yes deadlock=no time=1.5196 cpu=0.0612
mode=alt threads=8 rounds=100 bad=-1 firstbad=-1 checksum=nobarrier correct=no deadlock=no time=0.0000 cpu=0.0000
alt: create() returned NULL -- nothing was run

```

| threads | given: correct? | given: time | given: cpu | fixed: time | fixed: cpu | alt: time | alt: cpu |
|---|---|---|---|---|---|---|---|
|mode=fixed threads=1 rounds=100 bad=0 firstbad=-1 checksum=ok correct=yes deadlock=no time=0.0003 cpu=0.0004
|mode=fixed threads=2 rounds=100 bad=0 firstbad=-1 checksum=ok correct=yes deadlock=no time=0.2187 cpu=0.0075
|mode=fixed threads=4 rounds=100 bad=0 firstbad=-1 checksum=ok correct=yes deadlock=no time=0.6512 cpu=0.0261
|mode=fixed threads=8 rounds=100 bad=0 firstbad=-1 checksum=ok correct=yes deadlock=no time=1.5251 cpu=0.0610

**S3.1** Reconcile with `PREDICTION.md`: quote what you predicted, say what
happened, account for the difference. If you were right, say what would have
made you wrong.

In my prediction I assumed that I would be able to make the program faster with condition variables but I ended up making it slower, if given more time I may have figured out how to make it faster.
In my prediction I seem to be wrong.

**S3.2** Which would you ship on this machine, **and what measurement would
change your mind?**

I would ship Given unless I found a way to make fixed faster

## S4 — explain-back · 15 marks

> Two or three sentences, your own words: someone who has not seen this code
> asks *what was wrong with it, and what did fixing it cost?*

The mutex locking and unlocking itself each millisecond costs time, I did not find a way to make it faster but my implementation of condition variables made it slower

## Anything you got stuck on

How to properly implement condition variables to this code if at all in order to make it faster
