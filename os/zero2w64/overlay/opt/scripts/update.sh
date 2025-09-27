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
rm -rf /tmp/zicPixel.zip

echo "Done"

# Restart zic
reboot
