# 07 Hardware

## Zic Pixel

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/hardware/ZicPixel/pixel3.png" />

Small version of ZicBox using capacitive PCB keyboard with MPR121 ICs and smaller display. The goal is to reduce the cost and make ZicBox more affordable.

### Material


- 2x PCBs at JLCPCB 4€ each inlucing shipping
- 1x Raspberry Pi zero 25€
- 1x [Waveshare 1.47in LCD module](https://www.waveshare.com/wiki/1.47inch_LCD_Module) 20€ on Amazon, 13€ on Aliexpress
- 4x encoders wihtout detent [PEC12R-4025F-N0024](https://eu.mouser.com/ProductDetail/Bourns/PEC12R-4025F-N0024?qs=Zq5ylnUbLm4HSBD7%2FFgU%2FA%3D%3D&countryCode=DE&currencyCode=EUR&_gl=1*1nd7s7x*_ga*Nzc0OTY5NDMwLjE2OTg1MDM2NzE.*_ga_15W4STQT4T*MTcwNTk0NTcwNi4xMi4wLjE3MDU5NDU3MDcuNTkuMC4w*_ga_1KQLCYKRX3*MTcwNTk0NTcwNi4yLjAuMTcwNTk0NTcwNy4wLjAuMA..) 1.13€ (but might get expensive with shipping, could use other encoder to reduce cost)
- 1x DAC [Adafruit UDA1334A](https://learn.adafruit.com/adafruit-i2s-stereo-decoder-uda1334a/raspberry-pi-wiring) 7€ (not available anymore) can find a copy on Amazon CJMCU-1334, 5x for 33€ =~ 5€ one piece (the DAC is optional and could be replaced by a USB audio card)
- 3x MPR121 capcitive sensor about 2.5€ per piece

Total of about 60€...

### Drivers

#### Raspberry Pi

To setup hardware drivers for the audio DAC and display, it is better to set them up manually to avoid configuration conflicts.

**Configure the I2S Sound Software**

Our system will use the Alsa-project software to drive the I2S signal and sound system. This is usually installed by default but we can use the command below to make sure it’s up to date.
```sh
sudo apt-get install alsa-utils
```

We next need to create a configuration file for the Alsa sound system. This will define a number of audio devices and how they should be set up so that the sound system can then use them to generate and output the sound signals.

Paste the following code in `/etc/asound.conf`.

```
pcm.speakerbonnet {
   type hw card 0
}

pcm.dmixer {
   type dmix
   ipc_key 1024
   ipc_perm 0666
   slave {
     pcm "speakerbonnet"
     period_time 0
     period_size 1024
     buffer_size 8192
     rate 44100
     channels 2
   }
}

ctl.dmixer {
    type hw card 0
}

pcm.softvol {
    type softvol
    slave.pcm "dmixer"
    control.name "PCM"
    control.card 0
}

ctl.softvol {
    type hw card 0
}

pcm.!default {
    type             plug
    slave.pcm       "softvol"
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

> source https://bytesnbits.co.uk/raspberry-pi-i2s-sound-output/ and https://learn.adafruit.com/adafruit-i2s-stereo-decoder-uda1334a/raspberry-pi-usage


**SPI 1.47inch LCD display with FBCP Porting**

Framebuffer uses a video output device to drive a video display device from a memory buffer containing complete frame data. Simply put, a memory area is used to store the display content, and the display content can be changed by changing the data in the memory.
There is an open source project on github: fbcp-ili9341. Compared with other fbcp projects, this project uses partial refresh and DMA to achieve a speed of up to 60fps.

Download Drivers
```
sudo apt-get install cmake libraspberrypi-dev raspberrypi-kernel-headers -y
cd ~
wget https://files.waveshare.com/upload/1/18/Waveshare_fbcp.zip
unzip Waveshare_fbcp.zip
cd Waveshare_fbcp/
sudo chmod +x ./shell/*

```

Raspberry Pi's `vc4-kms-v3d` will cause fbcp to fail, so we need to close vc4-kms-v3d before installing it in fbcp.

in `/boot/config.txt`
Just comment the following line by adding a `#` in front:
```sh
# dtoverlay=vc4-kms-v3d
# max_framebuffers=2
```

Then reboot to apply changes.

Build FBCP for your display:
```sh
mkdir build
cd build
cmake -DSPI_BUS_CLOCK_DIVISOR=20 -DWAVESHARE_1INCH47_LCD=ON -DBACKLIGHT_CONTROL=ON -DSTATISTICS=0 ..
make -j
sudo ./fbcp
```

After running `fbcp` the screen should update and show either the terminal or the desktop environment.

Finally, to automatically start it:
```sh
sudo cp ~/Waveshare_fbcp/build/fbcp /usr/local/bin/fbcp
```

And in `/etc/rc.local` add `fbcp&` before exit 0. Note that you must add "&" to run in the background, otherwise the system may not be able to start.

> source https://www.waveshare.com/wiki/1.47inch_LCD_Module
