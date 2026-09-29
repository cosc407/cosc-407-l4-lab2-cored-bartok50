# COSC 407/507 Lab 2 - the sealed core
#
#   make            build ./bar
#   make test       run the same checks the autograder runs
#   make verify     check that src/given.c is still byte-for-byte as issued
#   make clean      remove the binary
#
# -pthread is not optional: it sets the compile-time flags AND links the
# threading library. Using -lpthread alone works on some systems and silently
# misbehaves on others.
#
# No sanitizer flags here. ThreadSanitizer is Lab 3's instrument and it changes
# the timings this lab asks you to interpret, so keep it out of the table. If
# you want to look, build a copy by hand:
#
#   gcc -std=gnu11 -O2 -g -fsanitize=thread -Iinclude -o bar-tsan \
#       src/main.c src/given.c src/fixed.c src/alt.c

CC      := gcc
CFLAGS  := -std=gnu11 -O2 -Wall -Wextra -Iinclude
LDFLAGS := -pthread

# Anything you want to add to the compile, without editing anything:
#
#   make clean && make EXTRA=-DBAR_POLL_NS=50000
#
# One of the four cores asks you to sweep a compile-time constant, and its
# given.c is hashed -- so the sweep has to happen on the command line. In the
# other three you will not need this.
EXTRA   :=
CFLAGS  += $(EXTRA)

# The wildcard is for the one core whose brief needs a counting semaphore; in
# the others there is no such file and the list is just the four.
OBJSRC  := src/main.c src/given.c src/fixed.c src/alt.c $(wildcard src/mysem_ref.c)

.PHONY: all test verify clean

all: bar

bar: $(OBJSRC) include/barrier.h include/timer.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJSRC)

test: all
	bash tests/run_tests.sh all

verify:
	bash tests/run_tests.sh given

clean:
	rm -f bar bar.exe

# --- Unit tests (T1/T2 of the rubric) -----------------------------------
# tests/unit_test.c calls create()/wait()/destroy() on bar_fixed/bar_alt
# directly -- no subprocess, no ./bar, no watchdog. Needs libcriterion-dev,
# already in the course devcontainer.
UNITOBJ := src/fixed.c src/alt.c $(wildcard src/mysem_ref.c)
UNIT_COV_CFLAGS := -std=gnu11 -O0 -g -Wall -Wextra -Iinclude --coverage

.PHONY: unit-test unit-coverage

unit-test: tests/unit_test.c $(UNITOBJ) include/barrier.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o unit_test tests/unit_test.c $(UNITOBJ) -lcriterion
	./unit_test --verbose
	rm -f unit_test

unit-coverage: tests/unit_test.c $(UNITOBJ) include/barrier.h
	$(CC) $(UNIT_COV_CFLAGS) $(LDFLAGS) -o unit_test_cov tests/unit_test.c $(UNITOBJ) -lcriterion
	./unit_test_cov
	gcovr --root . --filter 'src/fixed\.c' --filter 'src/alt\.c' -s
	rm -f unit_test_cov *.gcda *.gcno
