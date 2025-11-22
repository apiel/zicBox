#!/bin/bash

while true; do
    echo "[watcher] Starting app..."

    TMP_OUTPUT=$(mktemp)

    START_VIEW=${START_VIEW:-} WINDOW_POSITION=${WINDOW_POSITION:-} WATCH=true \
    make runZic 2>&1 | tee "$TMP_OUTPUT"

    # Extract the entire RESTART line:
    # Example: "RESTART: Track1 0,300"
    RESTART_LINE=$(grep -oP '^RESTART:\s*\K.*' "$TMP_OUTPUT")

    rm -f "$TMP_OUTPUT"

    if [ -n "$RESTART_LINE" ]; then
        # Split into VIEW and POSITION
        RESTART_VIEW=$(echo "$RESTART_LINE" | awk '{print $1}')
        WINDOW_POSITION=$(echo "$RESTART_LINE" | awk '{print $2}')

        echo "[watcher] App requested restart with view: $RESTART_VIEW"
        echo "[watcher] App requested window position: $WINDOW_POSITION"

        export START_VIEW="$RESTART_VIEW"
        export WINDOW_POSITION="$WINDOW_POSITION"
    else
        echo "[watcher] No restart requested. Exiting."
        break
    fi
done

echo "[watcher] Exiting."
