#!/bin/bash

SRC_DIR="./application"

BUILD_SCRIPT="./scripts/build.sh"

DELAY=5

if ! command -v inotifywait >/dev/null 2>&1; then
    echo "Error: inotifywait not found. Please install it (e.g., sudo apt install inotify-tools)."
    exit 1
fi

echo "Watching $SRC_DIR for changes... (debounce: ${DELAY}s, Ctrl+C to stop)"

inotifywait -m -r -e modify,create,delete --format '%w%f' "$SRC_DIR" | while read FILE
do
    echo "Change detected in: $FILE"

    if [ -n "$TIMER_PID" ] && kill -0 "$TIMER_PID" 2>/dev/null; then
        kill "$TIMER_PID" 2>/dev/null
    fi

    (
        sleep "$DELAY"
        echo "Running build..."
        "$BUILD_SCRIPT"
    ) &
    TIMER_PID=$!
done
