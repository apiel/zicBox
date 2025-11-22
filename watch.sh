#!/bin/bash

while true; do
    echo "[watcher] Starting app..."

    TMP_OUTPUT=$(mktemp)

    START_VIEW=${START_VIEW:-} WINDOW_POSITION=${WINDOW_POSITION:-} WINDOW_SIZE=${WINDOW_SIZE:-} WATCH=true \
    make runZic 2>&1 | tee "$TMP_OUTPUT"

    # Extract everything after "RESTART:"
    # Example output: "Track1 0,300 1280x720"
    RESTART_LINE=$(grep -oP '^RESTART:\s*\K.*' "$TMP_OUTPUT")

    rm -f "$TMP_OUTPUT"

    if [ -n "$RESTART_LINE" ]; then
        # Extract 3 fields:
        RESTART_VIEW=$(echo "$RESTART_LINE" | awk '{print $1}')
        WINDOW_POSITION=$(echo "$RESTART_LINE" | awk '{print $2}')
        WINDOW_SIZE=$(echo "$RESTART_LINE" | awk '{print $3}')

        echo "[watcher] App requested restart with view: $RESTART_VIEW"
        echo "[watcher] App requested window position: $WINDOW_POSITION"
        echo "[watcher] App requested window size: $WINDOW_SIZE"

        export START_VIEW="$RESTART_VIEW"
        export WINDOW_POSITION="$WINDOW_POSITION"
        export WINDOW_SIZE="$WINDOW_SIZE"
    else
        echo "[watcher] No restart requested. Exiting."
        break
    fi
done

echo "[watcher] Exiting."
