#!/usr/bin/env bash
# COSC 407/507 Lab 2, sealed core -- the checks the autograder runs.
#
#   bash tests/run_tests.sh all       everything (default)
#   bash tests/run_tests.sh fixed     one group at a time
#
# Groups: given (src/given.c unmodified), fixed, alt, speed, report.
#
# This script is IDENTICAL in all four cores. What differs is tests/expect.env,
# which says what your core's given barrier and alternative are supposed to do.
# Green here is not full marks: most of this lab is the reasoning in RESULTS.md
# and the oral test, and no script can see either.
#
# NOTHING HERE HANGS. A broken barrier stops rather than lying, so every run is
# watched by the driver itself (BAR_WATCHDOG seconds, see src/main.c) and comes
# back with deadlock=yes instead of never coming back. If a check below is
# slower than you expected, that is a watchdog firing and it is a result.

set -u

WHICH="${1:-all}"
FAILED=0

ROUNDS=2000            # phases per run; a core whose brief needs fewer says so
MARGIN=1.3             # a timing claim has to win by this much to count
WATCHDOG=5             # seconds before the driver gives up on one run
CORE_CHECK=none
ALT_LABEL="the alternative in BRIEF.md"
ALT_CORRECT=yes

# expect.env is sourced AFTER those defaults, so a core can override any of
# them -- the poll-based core needs fewer rounds and a longer watchdog.
# shellcheck disable=SC1091
. tests/expect.env

export BAR_WATCHDOG="$WATCHDOG"

BIN=""

pass() { printf '  PASS  %s\n' "$1"; }
fail() { printf '  FAIL  %s\n' "$1"; FAILED=1; }
info() { printf '  ....  %s\n' "$1"; }

# Sets BIN, or reports a failure and returns 1. Deliberately NOT written as
# BIN="$(need_bin)": a command substitution runs in a subshell, so a FAILED=1
# set inside it is thrown away and a missing binary would report a silent pass.
need_bin() {
    if [ -x ./bar ];       then BIN=./bar
    elif [ -x ./bar.exe ]; then BIN=./bar.exe
    else
        fail "bar was not built -- run 'make' and fix the compile errors first"
        return 1
    fi
    return 0
}

# field <line> <name>  ->  the value of name= in that output line
field() { printf '%s\n' "$1" | sed -n "s/.*[[:space:]]$2=\([^[:space:]]*\).*/\1/p"; }

cores() {
    local c
    c="$(nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)"
    case "$c" in ''|*[!0-9]*) c=4 ;; esac
    echo "$c"
}

# More threads than cores, which is where a barrier that spins comes apart.
# Capped at 128 (MAX_THREADS) and floored at 4 so a single-core container still
# oversubscribes.
oversubscribed() {
    local t=$(( $(cores) * 2 ))
    [ "$t" -lt 4 ]   && t=4
    [ "$t" -gt 128 ] && t=128
    echo "$t"
}

# best_of <repeats> <mode> <threads> <field>  ->  the smallest value of that
# field over that many runs. One run of a fast thing tells you nothing -- that
# is Lab 0's whole point, and it applies to the autograder too.
best_of() {
    local i out v bestv=""
    for i in $(seq "$1"); do
        out="$("$BIN" "$2" "$3" "$ROUNDS" 2>/dev/null)"
        v="$(field "$out" "$4")"
        [ -z "$v" ] && continue
        if [ -z "$bestv" ] || awk -v a="$v" -v b="$bestv" 'BEGIN{exit !(a < b)}'; then
            bestv="$v"
        fi
    done
    printf '%s' "$bestv"
}

# ------------------------------------------------------- given unmodified ---
test_given() {
    echo "src/given.c -- must be byte for byte as issued"
    local want have
    if [ ! -f tests/given.sha256 ]; then
        fail "tests/given.sha256 is missing from your repository"
        return
    fi
    want="$(cut -d' ' -f1 tests/given.sha256)"
    have="$(sha256sum src/given.c | cut -d' ' -f1)"
    if [ "$want" = "$have" ]; then
        pass "src/given.c is unmodified"
    else
        fail "src/given.c has been edited. Restore it: 'git checkout src/given.c'."
        info "your report has to compare against the code you were handed, so this"
        info "check is worth its own marks and it is not negotiable"
    fi
}

# ---------------------------------------------------------------- fixed -----
test_fixed() {
    echo "src/fixed.c -- your minimal correction"
    local t out over
    need_bin || return

    for t in 1 2 4 8; do
        out="$("$BIN" fixed "$t" "$ROUNDS" 2>/dev/null)"
        if printf '%s\n' "$out" | grep -q 'correct=yes'; then
            pass "./bar fixed $t $ROUNDS is exactly correct"
        else
            fail "./bar fixed $t $ROUNDS must be exactly correct; got: $out"
        fi
    done

    # A barrier is not correct until it is correct the second time, and the
    # tenth, and with the threads landing in a different order.
    local bad=0 i
    for i in 1 2 3 4 5; do
        out="$("$BIN" fixed 8 "$ROUNDS" 2>/dev/null)"
        printf '%s\n' "$out" | grep -q 'correct=yes' || bad=1
    done
    if [ "$bad" -eq 0 ]; then
        pass "./bar fixed 8 $ROUNDS is still correct after five more runs"
    else
        fail "./bar fixed 8 $ROUNDS is not correct every time -- a barrier that works sometimes is not a barrier"
    fi

    # ... and with more threads than there are cores to run them on, which is
    # the case a barrier is most likely to get wrong and the one your machine
    # will not show you by accident.
    over="$(oversubscribed)"
    out="$("$BIN" fixed "$over" "$ROUNDS" 2>/dev/null)"
    if printf '%s\n' "$out" | grep -q 'correct=yes'; then
        pass "./bar fixed $over $ROUNDS is correct with $over threads on $(cores) cores"
    else
        fail "./bar fixed $over $ROUNDS must be correct with more threads than cores; got: $out"
    fi
}

# ------------------------------------------------------------------ alt -----
test_alt() {
    echo "src/alt.c -- $ALT_LABEL"
    local out
    need_bin || return

    out="$("$BIN" alt 4 "$ROUNDS" 2>/dev/null)"
    if [ "$ALT_CORRECT" = yes ]; then
        if printf '%s\n' "$out" | grep -q 'correct=yes'; then
            pass "./bar alt 4 $ROUNDS is correct, as your brief says it should be"
        else
            fail "./bar alt 4 $ROUNDS should be correct; got: $out"
        fi
    else
        if printf '%s\n' "$out" | grep -q 'correct=no'; then
            pass "./bar alt 4 $ROUNDS still does not work -- which is the point your brief makes"
            info "$out"
        else
            fail "./bar alt 4 $ROUNDS came out correct. Re-read your brief: this alternative"
            fail "is supposed to stay broken. Got: $out"
        fi
    fi
}

# ----------------------------------------------------------- core claim -----
# The one claim about your core that a script can check. Which one it is
# depends on the core; tests/expect.env picks it and says why.
test_speed() {
    echo "the measurement your brief asks for"
    local out out1 out2 out4 ta tb ratio a b over
    need_bin || return

    case "$CORE_CHECK" in
    none)
        info "no single claim is auto-checked for this core; the numbers are:"
        # One mode at a time rather than `all`: a watchdog firing on one of them
        # ends the process, and running them separately keeps the other two.
        for a in given fixed alt; do
            out="$("$BIN" "$a" 4 "$ROUNDS" 2>/dev/null)"
            printf '        %s\n' "$out"
        done
        pass "timings recorded (the marks for the measurement are in RESULTS.md)"
        ;;

    given_correct_at_one_wrong_at_four)
        out1="$("$BIN" given 1 "$ROUNDS" 2>/dev/null)"
        out4="$("$BIN" given 4 "$ROUNDS" 2>/dev/null)"
        if printf '%s\n' "$out1" | grep -q 'correct=yes'; then
            pass "./bar given 1 $ROUNDS is correct -- one thread is always in step with itself"
        else
            fail "./bar given 1 $ROUNDS should be correct; got: $out1"
        fi
        if printf '%s\n' "$out4" | grep -q 'correct=no' &&
           printf '%s\n' "$out4" | grep -q 'deadlock=no'; then
            pass "./bar given 4 $ROUNDS gets the wrong answer without stopping (bad=$(field "$out4" bad), firstbad=$(field "$out4" firstbad))"
        else
            fail "./bar given 4 $ROUNDS should come back WRONG rather than deadlocked; got: $out4"
        fi
        ;;

    given_correct_at_two_deadlocks_at_four)
        out2="$("$BIN" given 2 "$ROUNDS" 2>/dev/null)"
        out4="$("$BIN" given 4 "$ROUNDS" 2>/dev/null)"
        if printf '%s\n' "$out2" | grep -q 'correct=yes'; then
            pass "./bar given 2 $ROUNDS is correct -- and that is the trap, not the result"
        else
            fail "./bar given 2 $ROUNDS should be correct; got: $out2"
        fi
        if printf '%s\n' "$out4" | grep -q 'deadlock=yes'; then
            pass "./bar given 4 $ROUNDS stops dead, as the watchdog reports"
        else
            fail "./bar given 4 $ROUNDS should stop rather than finish; got: $out4"
        fi
        ;;

    fixed_faster_than_given|fixed_faster_than_alt|fixed_cheaper_than_given|fixed_cheaper_than_alt)
        case "$CORE_CHECK" in
            *_cheaper_than_*) FLD=cpu  ; WORD="burns less CPU than" ;;
            *)                FLD=time ; WORD="beats"               ;;
        esac
        a=fixed
        b="${CORE_CHECK##*_than_}"
        over=4
        ta="$(best_of 3 "$a" "$over" "$FLD")"
        tb="$(best_of 3 "$b" "$over" "$FLD")"

        if [ -z "$ta" ] || [ -z "$tb" ]; then
            fail "could not read the $FLD back out of ./bar -- is it printing the whole line?"
            return
        fi
        ratio="$(awk -v x="$ta" -v y="$tb" 'BEGIN{ if (x > 0) printf "%.1f", y / x; else print "inf" }')"
        if awk -v x="$ta" -v y="$tb" -v m="$MARGIN" 'BEGIN{ exit !(y > x * m) }'; then
            pass "at $over threads, $a ($ta) $WORD $b ($tb) by ${ratio}x"
        else
            fail "at $over threads, $a ($ta) should clearly $WORD $b ($tb)"
            info "if your fix is correct but no cheaper, you have repaired the answer"
            info "without repairing the mechanism -- which is what RESULTS.md asks about"
        fi
        ;;

    *)
        fail "tests/expect.env asks for an unknown check: '$CORE_CHECK'"
        info "tell your TA -- this is our bug, not yours, and it is not your marks"
        ;;
    esac
}

# --------------------------------------------------------------- report -----
# section <file> <heading-prefix> : the body of one '## ' section with fenced
# code blocks, block quotes, table rows and the bold prompt lines removed, so
# that a word count means "words the student wrote".
section() {
    awk -v h="$2" '
        $0 ~ "^## " h { inside = 1; next }
        /^## /        { inside = 0 }
        inside        { print }
    ' "$1" | awk '
        /^```/    { fence = !fence; next }
        fence     { next }
        /^>/      { next }
        /^\|/     { next }
        /^\*\*/   { next }
        /REPLACE/ { next }
                  { print }
    '
}

words() { grep -oE '[A-Za-z]+' | wc -l; }

need_words() {   # need_words <min> <got> <what>
    if [ "$2" -ge "$1" ]; then
        pass "$3 is written ($2 words)"
    else
        fail "$3 is too short ($2 words, expected at least $1)"
    fi
}

test_report_prediction() {
    if [ ! -f PREDICTION.md ]; then
        fail "PREDICTION.md is missing. It is due at 0:20, before you compile anything."
        return
    fi
    if grep -q 'REPLACE THIS LINE' PREDICTION.md; then
        fail "PREDICTION.md still contains a 'REPLACE THIS LINE' placeholder"
    else
        pass "no placeholders left in PREDICTION.md"
    fi
    local w
    w="$(sed '/^>/d;/^\*\*/d;/^#/d;/^|/d' PREDICTION.md | words)"
    need_words 55 "$w" "the prediction sheet"
}

test_report_results_other() {
    if [ ! -f RESULTS.md ]; then
        fail "RESULTS.md is missing"
        return
    fi
    if grep -q 'REPLACE THIS LINE' RESULTS.md; then
        fail "RESULTS.md still contains a 'REPLACE THIS LINE' placeholder"
    else
        pass "no placeholders left in RESULTS.md"
    fi
    local disc
    disc="$(sed -n 's/^[Tt]ools and sources:[[:space:]]*//p' RESULTS.md | head -1)"
    if [ -n "$disc" ] && ! printf '%s' "$disc" | grep -q 'REPLACE'; then
        pass "the tools-and-sources disclosure is filled in"
    else
        fail "RESULTS.md must state your tools and sources, or say explicitly you used none"
    fi
    if grep -qiE '^[[:space:]]*[Cc]ores:[[:space:]]*[0-9]+' RESULTS.md; then
        pass "the machine's core count is recorded"
    else
        fail "RESULTS.md must record how many cores this machine has -- a barrier without a core count is not a measurement"
    fi
    local w
    w="$(section RESULTS.md 'S2' | words)"; need_words 90  "$w" "the S2 diagnosis"
    w="$(section RESULTS.md 'S3' | words)"; need_words 105 "$w" "the S3 attribution"
}

test_report_given3() {
    [ -f RESULTS.md ] || { fail "RESULTS.md is missing"; return; }
    local ng
    ng="$(awk '/^## S2/{f=1;next} /^## /{f=0} f' RESULTS.md | grep -c 'mode=given')"
    if [ "$ng" -ge 3 ]; then
        pass "S2 pastes $ng runs of the barrier you were handed"
    else
        fail "S2 must paste at least 3 runs of ./bar given (found $ng)"
    fi
}

test_report_twelve() {
    [ -f RESULTS.md ] || { fail "RESULTS.md is missing"; return; }
    local nm
    nm="$(awk '/^## S3/{f=1;next} /^## /{f=0} f' RESULTS.md | grep -c 'mode=')"
    if [ "$nm" -ge 12 ]; then
        pass "S3 pastes $nm timed runs"
    else
        fail "S3 must paste all twelve runs -- given, fixed and alt at 1, 2, 4 and 8 threads (found $nm)"
    fi
}

test_report_explainback() {
    [ -f RESULTS.md ] || { fail "RESULTS.md is missing"; return; }
    local w
    w="$(section RESULTS.md 'S4' | words)"; need_words 25  "$w" "the explain-back answer"
}

test_report() {
    echo "PREDICTION.md and RESULTS.md"
    test_report_prediction
    test_report_results_other
    test_report_given3
    test_report_twelve
    test_report_explainback
}

# ---------------------------------------------------------------------------
# Unit testing and coverage (not mutation -- deliberately left out for this
# lab). tests/unit_test.c calls create()/wait()/destroy() on bar_fixed/bar_alt
# directly. Not wired into `all` and not run by `make test` -- these are a
# separate, additional check, run by their own commands below.

test_unit() {
    if [ ! -f tests/unit_test.c ]; then
        fail "tests/unit_test.c does not exist"
        return
    fi
    nf="$(grep -c 'Test(fixed,' tests/unit_test.c)"
    na="$(grep -c 'Test(alt,'   tests/unit_test.c)"
    if [ "$nf" -ge 3 ]; then
        pass "tests/unit_test.c has $nf cases for bar_fixed"
    else
        fail "tests/unit_test.c needs at least 3 cases for bar_fixed (found $nf)"
    fi
    if [ "$na" -ge 3 ]; then
        pass "tests/unit_test.c has $na cases for bar_alt"
    else
        fail "tests/unit_test.c needs at least 3 cases for bar_alt (found $na)"
    fi

    if make unit-test >/tmp/unit_test.out 2>&1; then
        pass "make unit-test -- all of your own cases pass"
    else
        fail "make unit-test -- at least one of your own cases fails, times out, or does not compile"
        tail -20 /tmp/unit_test.out
    fi
}

UNIT_COV_THRESHOLD="${UNIT_COV_THRESHOLD:-70}"

test_unitcov() {
    if ! make unit-coverage >/tmp/unit_cov.out 2>&1; then
        fail "make unit-coverage did not run -- fix tests/unit_test.c first"
        tail -20 /tmp/unit_cov.out
        return
    fi
    pct="$(grep -oE '^lines: *[0-9]+(\.[0-9]+)?' /tmp/unit_cov.out | grep -oE '[0-9]+(\.[0-9]+)?')"
    if [ -z "$pct" ]; then
        fail "could not read a coverage percentage from make unit-coverage's output"
        return
    fi
    pct_i="${pct%.*}"
    if [ "$pct_i" -ge "$UNIT_COV_THRESHOLD" ]; then
        pass "make unit-coverage: ${pct}% (threshold ${UNIT_COV_THRESHOLD}%)"
    else
        fail "make unit-coverage: ${pct}%, below the ${UNIT_COV_THRESHOLD}% threshold"
    fi
}
# ---------------------------------------------------------------------------

# ---------------------------------------------------------------------------
# `score` -- a running total against ONLY the rows lab2-rubric.md itself
# calls pure script rows (not the two it calls "mixed": the measurement and
# table-complete rows, where a script can confirm presence but not whether
# it's actually correct). 18 from Part 1 + everything in Part 2 = 68 possible.
#
# This is NOT the lab mark. It is the ~68% of it a script can ever see. The
# rest (S1's P1-P3/P4, S2.1/S2.2/S2.3, S3.1/S3.2, S4's "names wrong + cost")
# plus the oral multiplier are not computed here, and cannot be -- they
# require reading prose and judging quality, which nothing on this page does.
row() {   # row <label> <marks> <function...>
    local label="$1" marks="$2" rc; shift 2
    if ( FAILED=0; "$@" >/dev/null 2>&1; rc=$?; [ "$rc" -eq 0 ] && [ "$FAILED" -eq 0 ] ); then
        printf '  %3d / %-3d  %s\n' "$marks" "$marks" "$label"
        SCORE=$((SCORE + marks))
    else
        printf '  %3d / %-3d  %s\n' 0 "$marks" "$label"
    fi
    POSSIBLE=$((POSSIBLE + marks))
}

test_score() {
    SCORE=0
    POSSIBLE=0

    make >/dev/null 2>&1

    echo "Part 1 -- sealed core (script-checked rows only)"
    row "PREDICTION.md present, no placeholder, written"   2  test_report_prediction
    row "given.c unmodified"                                2  test_given
    row "3+ given runs pasted"                              2  test_report_given3
    row "fixed exactly correct, all counts"                 6  test_fixed
    row "twelve runs pasted"                                2  test_report_twelve
    row "alt behaves as brief says"                         2  test_alt
    row "explain-back present"                              2  test_report_explainback
    echo
    echo "Part 2 -- testing"
    row "T1: 3+ cases each for bar_fixed/bar_alt"           15  test_unit_count
    row "T2: make unit-test -- all cases pass"              15  test_unit_pass
    row "T3: unit-coverage >= threshold"                    20  test_unitcov
    echo
    echo "Script-checked score: $SCORE / $POSSIBLE"
    echo "This is not the lab mark -- see the comment at the top of this section."

    if ! grep -q 'REPLACE' RESULTS.md 2>/dev/null; then
        disc="$(sed -n 's/^[Tt]ools and sources:[[:space:]]*//p' RESULTS.md 2>/dev/null | head -1)"
        if [ -z "$disc" ] || printf '%s' "$disc" | grep -q 'REPLACE'; then
            echo "NOTE: no tools-and-sources disclosure -- this is an automatic zero for"
            echo "the whole lab per lab2-rubric.md, regardless of the number above."
        fi
    fi
}

test_unit_count() {
    [ -f tests/unit_test.c ] || { fail "tests/unit_test.c does not exist"; return; }
    local nf na
    nf="$(grep -c 'Test(fixed,' tests/unit_test.c)"
    na="$(grep -c 'Test(alt,'   tests/unit_test.c)"
    [ "$nf" -ge 3 ] && [ "$na" -ge 3 ]
}

test_unit_pass() {
    make unit-test >/dev/null 2>&1
}
# ---------------------------------------------------------------------------

case "$WHICH" in
    given)  test_given ;;
    fixed)  test_fixed ;;
    alt)    test_alt ;;
    speed)  test_speed ;;
    report) test_report ;;
    unit)    test_unit ;;
    unitcov) test_unitcov ;;
    score)  test_score; exit 0 ;;
    all)    test_given; echo; test_fixed; echo; test_alt; echo
            test_speed; echo; test_report ;;
    *)      echo "usage: $0 [all|given|fixed|alt|speed|report|unit|unitcov|score]" >&2; exit 2 ;;
esac

echo
if [ "$FAILED" -eq 0 ]; then
    echo "all checks passed for: $WHICH"
else
    echo "there are failures above for: $WHICH"
fi
exit "$FAILED"
