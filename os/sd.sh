#!/bin/bash
set -e

DEVICE="/dev/sda"
BOOT_PART="${DEVICE}1"
ROOT_PART="${DEVICE}2"
BOOT_MOUNT="/media/alex/boot"
ROOT_MOUNT="/media/alex/rootfs1"

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")

if [ -f $SCRIPT_DIR/.wpa_supplicant.conf ]; then
    cp $SCRIPT_DIR/.wpa_supplicant.conf $ROOT_MOUNT/etc/wpa_supplicant.conf
    echo "Copied $SCRIPT_DIR/.wpa_supplicant.conf"
else
    echo "No $SCRIPT_DIR/.wpa_supplicant.conf found"
fi

# Create mount points
mkdir -p "$BOOT_MOUNT"
mkdir -p "$ROOT_MOUNT"

echo "[*] Refreshing partition table..."
partprobe "$DEVICE"
sleep 2

echo "[*] Current partition layout:"
lsblk -o NAME,SIZE,FSTYPE,MOUNTPOINT "$DEVICE"

echo
read -rp ">>> Confirm you want to resize ${ROOT_PART} only (boot partition will be kept)? (y/N): " CONFIRM
if [[ "$CONFIRM" != "y" && "$CONFIRM" != "Y" ]]; then
    echo "Aborting."
    exit 1
fi

# Unmount if mounted
for PART in "$BOOT_PART" "$ROOT_PART"; do
    MOUNTPOINT=$(lsblk -n -o MOUNTPOINT "$PART")
    if [ -n "$MOUNTPOINT" ]; then
        echo " - Unmounting $PART ($MOUNTPOINT)"
        umount "$PART" || true
    fi
done

# Find start of root partition
ROOT_START=$(parted -m "$DEVICE" unit s print | grep "^2:" | cut -d: -f2 | sed 's/s//')
echo "Root partition starts at sector: $ROOT_START"

# Resize root partition to fill disk
echo "[*] Resizing partition with parted..."
parted "$DEVICE" ---pretend-input-tty <<EOF
resizepart 2 100%
Yes
quit
EOF

# Refresh partition table
echo "[*] Re-reading partition table..."
partprobe "$DEVICE"
sleep 2

echo "[*] Checking filesystem on $ROOT_PART..."
e2fsck -f "$ROOT_PART" || true

echo "[*] Resizing filesystem on $ROOT_PART..."
resize2fs "$ROOT_PART"

# # Mount partitions again
# mount "$BOOT_PART" "$BOOT_MOUNT"
# mount "$ROOT_PART" "$ROOT_MOUNT"

# echo
# echo "[✓] Resize completed and SD card mounted back!"
# lsblk -o NAME,SIZE,FSTYPE,MOUNTPOINT "$DEVICE"

# sudo cp ./my_wpa_supplicant.conf $ROOT_MOUNT/etc/wpa_supplicant.conf || true


echo
echo "[✓] Resize completed and SD card mounted back!"
echo "You can now unplug the SD card."
