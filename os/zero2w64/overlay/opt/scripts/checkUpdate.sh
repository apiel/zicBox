#!/bin/sh

STATE_FILE="/opt/scripts/.last_release"

# optional delay (default 0)
DELAY=${1:-0}
if [ "$DELAY" -gt 0 ] 2>/dev/null; then
    echo "Delaying $DELAY seconds."
    sleep "$DELAY"
fi

# Fetch release info using wget
LATEST=$(wget -qO- "https://api.github.com/repos/apiel/zicBox/releases/tags/zicPixel" --no-check-certificate \
    | grep '"published_at":' \
    | head -n 1 \
    | sed -e 's/.*"published_at": *"//' -e 's/".*//')

# If fetch failed, exit quietly
[ -z "$LATEST" ] && exit 1

# First run: save and exit
if [ ! -f "$STATE_FILE" ]; then
    echo "Save state $LATEST"
    echo "$LATEST" > "$STATE_FILE"
    exit 0
fi

STORED=$(cat "$STATE_FILE")

# Compare timestamps
if [ "$LATEST" != "$STORED" ]; then
    echo "Update available $STORED -> $LATEST."
    echo "setView:UpdateAvailable" > "/opt/zicBox/message.txt"
    echo "$LATEST" > "$STATE_FILE"
else
    echo "No update available."
fi
