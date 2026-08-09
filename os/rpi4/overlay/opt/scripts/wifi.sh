#!/bin/sh

        
echo "Starting Wi-Fi connection..."
modprobe brcmfmac

# Wait up to 10 seconds for wlan0 to appear
i=0
while [ $i -lt 10 ] && ! ip link show wlan0 >/dev/null 2>&1; do
    echo "Waiting for wlan0..."
    sleep 1
    i=$((i+1))
done

ip link set wlan0 up
wpa_supplicant -i wlan0 -c /etc/wpa_supplicant.conf -B


# Wait for an IP address to be assigned, with a timeout
echo "Waiting for IP address on wlan0..."
i=0
while [ $i -lt 5 ] && ! ip addr show wlan0 | grep "inet"; do
    sleep 1
    i=$((i+1))
done

udhcpc -i wlan0 -b