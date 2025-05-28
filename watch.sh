#!/bin/bash

while true; do
    echo "[watcher] Starting app..."
    
    # Start the app, capture output
    OUTPUT=$(START_VIEW=${START_VIEW:-} WATCH=true make runPixel 2>&1)

    echo "$OUTPUT"

    # Extract restart view if any
    RESTART_VIEW=$(echo "$OUTPUT" | grep -oP '^RESTART:\s*\K\S+')

    if [ -n "$RESTART_VIEW" ]; then
        echo "[watcher] App requested restart with view: $RESTART_VIEW"
        export START_VIEW="$RESTART_VIEW"
    else
        echo "[watcher] No restart requested. Exiting."
        break
    fi

done
