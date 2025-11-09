#!/bin/bash

while true; do
    echo "[watcher] Starting app..."

    # Use a temporary file to capture the output while still displaying it live
    TMP_OUTPUT=$(mktemp)

    # Start app and pipe output to tee (for live view) and to temp file (for restart detection)
    START_VIEW=${START_VIEW:-} WATCH=true make runZic 2>&1 | tee "$TMP_OUTPUT"

    # Extract restart view from captured output
    RESTART_VIEW=$(grep -oP '^RESTART:\s*\K\S+' "$TMP_OUTPUT")

    # Clean up temp file
    rm -f "$TMP_OUTPUT"

    if [ -n "$RESTART_VIEW" ]; then
        echo "[watcher] App requested restart with view: $RESTART_VIEW"
        export START_VIEW="$RESTART_VIEW"
    else
        echo "[watcher] No restart requested. Exiting."
        break
    fi
done

echo "[watcher] Exiting."