#!/bin/sh
      
echo "Update Pixel firmware..."

# Backup the current firmware
rm -rf /opt/zicBox_backup || true
mkdir -p /opt/zicBox_backup
rsync -a --exclude 'data' /opt/zicBox/ /opt/zicBox_backup/

# Download the firmware
wget -O /tmp/zicPixel.zip https://github.com/apiel/zicBox/releases/download/zicPixel/zicPixel.zip --no-check-certificate

# Unzip the firmware
unzip -o /tmp/zicPixel.zip -d /opt/zicBox

# Clean up
rm -rf /tmp/zicPixel.

# Remove latest release file, so checkUpdate.sh will regenerate it and not use the old one
rm /opt/scripts/.last_release

# We still try to save the current release info
LATEST=$(wget -qO- "https://api.github.com/repos/apiel/zicBox/releases/tags/zicPixel" --no-check-certificate \
    | grep '"published_at":' \
    | head -n 1 \
    | sed -e 's/.*"published_at": *"//' -e 's/".*//')

# LATEST not empty
if [ -n "$LATEST" ]; then
    echo "Save state $LATEST"
    echo "$LATEST" > "$STATE_FILE"
fi

echo "Done"

# Restart zic
reboot
