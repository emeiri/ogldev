#!/bin/bash
#
# Used by the master build: print the summary and then the problems one by one.
#
#   report.sh <state dir>

STATE=$1

START=$(cat "$STATE/start")
SECS=$(( $(date +%s) - START ))
TOTAL=$(cat "$STATE/done" 2>/dev/null | wc -l)
FAILED=$(cat "$STATE/failed" 2>/dev/null | wc -l)

echo
echo "Built $TOTAL projects in $((SECS / 60))m $((SECS % 60))s: $((TOTAL - FAILED)) OK, $FAILED failed"

if [ -s "$STATE/skipped" ]; then
    echo "Skipped $(wc -l < "$STATE/skipped") Vulkan projects because VULKAN_SDK is not set"
fi

[ "$FAILED" -eq 0 ] && exit 0

I=0
sort "$STATE/failed" | while read -r DIR; do
    I=$((I + 1))
    echo
    echo "==== Problem $I/$FAILED: $DIR   (full log: $DIR/build.log)"
    ERRORS=$(grep -E "error|undefined reference|multiple definition|No rule|cannot find" "$DIR/build.log" | head -10)
    if [ -n "$ERRORS" ]; then
        echo "$ERRORS"
    else
        tail -10 "$DIR/build.log"
    fi
done

exit 1
