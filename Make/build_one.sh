#!/bin/bash
#
# Used by the master build to build one library or project quietly.
# The full output goes to <dir>/build.log, the terminal only gets one line.
#
#   build_one.sh lib|project <dir> <state dir> <number of projects>
#
# A failing library stops the master build. A failing project is
# recorded in <state dir>/failed and reported at the end.

KIND=$1
DIR=$2
STATE=$3
TOTAL=$4

START=$(date +%s)

if [ "$KIND" = lib ]; then
    make -C "$DIR" NO_LIB_BUILD=1 > "$DIR/build.log" 2>&1
else
    make -C "$DIR" NO_LIB_BUILD=1 build > "$DIR/build.log" 2>&1
fi
RESULT=$?

SECS=$(( $(date +%s) - START ))

if [ "$KIND" = lib ]; then
    if [ $RESULT -eq 0 ]; then
        printf "  library %-28s OK    (%ss)\n" "$DIR" "$SECS"
    else
        printf "  library %-28s FAILED, see %s/build.log\n" "$DIR" "$DIR"
        grep -m 10 -E "error|undefined reference|multiple definition|No rule|\*\*\* [^[]" "$DIR/build.log"
    fi
    exit $RESULT
fi

# Count the finished projects (several builds run at the same time)
N=$(flock "$STATE/lock" bash -c "echo $DIR >> '$STATE/done'; wc -l < '$STATE/done'")

if [ $RESULT -eq 0 ]; then
    STATUS="OK"
else
    STATUS="FAILED"
    flock "$STATE/lock" bash -c "echo $DIR >> '$STATE/failed'"
fi

printf "[%3d/%d] %-6s %-46s (%ss)\n" "$N" "$TOTAL" "$STATUS" "$DIR" "$SECS"
exit 0
