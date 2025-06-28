# 04 Hardware

<table>
    <tr>
        <td><a href="https://github.com/apiel/zicBox/wiki/04-Hardware#zic-grid"><b>Zic Grid</b></a></td>
        <td><a href="https://github.com/apiel/zicBox/wiki/04-Hardware#zic-pixel"><b>Zic Pixel</b></a></td>
    </tr>
    <tr>
        <td>
            <a href="https://github.com/apiel/zicBox/wiki/04-Hardware#zic-grid">
                <img src="https://raw.githubusercontent.com/apiel/zicBox/main/hardware/ZicGrid/zicgrid.png" />
            </a>
        </td>
        <td>
            <a href="https://github.com/apiel/zicBox/wiki/04-Hardware#zic-pixel">
                <img src="https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/pixel3A_2.png" width='480' />
            </a>
        </td>
    </tr>
</table>

> [!TIP]
> Before to order pcb, I would recommend you to join the [discord channel](https://discord.gg/65HTx7z9qg) to have a chat on the current status of the project and see which version might be the most suited for you. This project is in constant evolution and new PCB versions are regularly designed.

## PCB

PCBs are designed with [EasyEDA](https://easyeda.com/). You can load the copy of the projects using the easyeda json file available in the different folder of the builds.

To order the PCB, you need to send the Gerber file (gerber*.zip) to https://jlcpcb.com/. With slow shipping (about 10 days), it cost around 4€. Just upload the file using the button `Add gerber file` and normally there should be nothing to change. You can eventually change the color from `green` to `black`, it should not impact the price. Finally `Save to cart` and proceed to the checkout.

## Audio DAC

In my builds, I am using PCM5102 a cheap but good quality DAC that can be found on Amazon and Aliexpress. Depending where you buy it, you might need to solder some jumper on the DAC.

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/hardware/pcm5102_00.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/hardware/pcm5102_01.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/hardware/pcm5102_02.png" />


Our system will use the Alsa-project software to drive the I2S signal and sound system. This is usually installed by default but we can use the command below to make sure it’s up to date.
```sh
sudo apt-get install alsa-utils
```

We next need to create a configuration file for the Alsa sound system. This will define a number of audio devices and how they should be set up so that the sound system can then use them to generate and output the sound signals.

Paste the following code in `/etc/asound.conf`.

```
pcm.!default  {
 type hw card 0
}
ctl.!default {
 type hw card 0
}
```

This basically sets up a PCM audio device along with a mixer device to allow us to adjust the volume in software.

The final step is to edit the `/boot/config.txt` file to load the device tree overlays at start up.

First look for the line
```
dtparam=audio=on
```
This turns on the standard headphone outputs so we need to disable it by commenting out this line – just put a # at the start of the line.

Then at the bottom of the file we need to add the I2S overlays.

```
dtoverlay=hifiberry-dac
dtoverlay=i2s-mmap
```

Finally reboot the Raspberry Pi to get all the changes loaded.

To test, try playing a simple WAV file.

```sh
speaker-test -c2 --test=wav -w /usr/share/sounds/alsa/Front_Center.wav
```

## Gpio

In order to not interfer with gpio, uart (serial) should be disabled.

In `/boot/config.txt` add:
```
# disables the UART functionality
enable_uart=0
# disables the Bluetooth module, which may otherwise occupy the UART.
dtoverlay=disable-bt
```

Disable the Serial Service by running the following commands:
```
sudo systemctl stop serial-getty@ttyAMA0.service
sudo systemctl disable serial-getty@ttyAMA0.service
```

Then reboot. Check if serial is gone by running `ls -l /dev/serial*`
