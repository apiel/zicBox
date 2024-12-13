#!/bin/bash

# Check if the script is run as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root. Use sudo or log in as root." >&2
   exit 1
fi

apt-get update
apt-get install vim build-essential librtmidi-dev libsndfile1-dev pulseaudio alsa-utils libbcm2835-dev libraspberrypi-dev -y

pulseaudio --start

# Write the content to /etc/asound.conf
cat << EOF > /etc/asound.conf
pcm.!default {
 type hw card 0
}
ctl.!default {
 type hw card 0
}
EOF

echo "/etc/asound.conf has been updated."

# Modify /boot/config.txt
CONFIG_FILE="/boot/firmware/config.txt"

# Comment out dtparam=audio=on if it exists
sed -i 's/^dtparam=audio=on/#&/' "$CONFIG_FILE"

# Uncomment dtparam=spi=on if it's commented out
sed -i 's/^#\(dtparam=spi=on\)/\1/' "$CONFIG_FILE"

# Uncomment dtparam=i2s=on if it's commented out
sed -i 's/^#\(dtparam=i2s=on\)/\1/' "$CONFIG_FILE"

# Ensure dtparam=spi=on is present
if ! grep -q "^dtparam=spi=on" "$CONFIG_FILE"; then
    echo "dtparam=spi=on" >> "$CONFIG_FILE"
fi

# Ensure dtparam=i2s=on is present
if ! grep -q "^dtparam=i2s=on" "$CONFIG_FILE"; then
    echo "dtparam=i2s=on" >> "$CONFIG_FILE"
fi

# Append other configurations at the end of /boot/config.txt
cat << EOF >> "$CONFIG_FILE"

dtoverlay=hifiberry-dac
dtoverlay=i2s-mmap

# disables the UART functionality
enable_uart=0
# disables the Bluetooth module, which may otherwise occupy the UART.
dtoverlay=disable-bt

force_eeprom_read=0
disable_poe_fan=1

# set gpio pins to input and pull up
# especially important for gpio 14 -> UART
gpio=14=ip,pu
gpio=7=ip,pu
gpio=8=ip,pu
gpio=12=ip,pu
gpio=13=ip,pu
gpio=15=ip,pu
gpio=16=ip,pu
gpio=23=ip,pu
gpio=24=ip,pu
gpio=25=ip,pu
EOF

echo "/boot/config.txt has been updated."

# Ask the user if they want to reboot
read -p "Would you like to reboot now? (y/N): " reboot_choice
case "$reboot_choice" in
    [yY][eE][sS]|[yY])
        echo "Rebooting now..."
        reboot
        ;;
    *)
        echo "Reboot skipped. Please reboot later to apply changes."
        ;;
esac
