# 07 Hardware

## Zic Pixel

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/hardware/ZicPixel/pixel.png" />

Small version of ZicBox using capacitive PCB keyboard with MPR121 ICs and smaller display. The goal is to reduce the cost and make ZicBox more affordable.

### Material


- 2x PCBs at JLCPCB 4€ each inlucing shipping
- 1x Raspberry Pi zero 2 25€
- 1x ST7789 LCD OLED Display 240x240 [3€ on Aliexpress](https://www.aliexpress.com/item/1005005990777548.html) [8€ on Amazon](https://www.amazon.de/-/en/DollaTek-Display-Module-Interface-Arduino/dp/B07QJY5H9G/)
- 4x switch encoders without detent [PEC12R-4025F-S0024](https://eu.mouser.com/ProductDetail/Bourns/PEC12R-4025F-S0024?qs=Zq5ylnUbLm7c1LzY1JyJgg%3D%3D&_gl=1*18jq081*_ga*MjEzNTYwMjcyNS4xNzA5NDU2NTk3*_ga_15W4STQT4T*MTcxMTM3NzQ1MC4zLjEuMTcxMTM3NzUxOS41MS4wLjA.*_ga_1KQLCYKRX3*MTcxMTM3NzQ1MC4zLjEuMTcxMTM3NzUxOS4wLjAuMA..) 1.50€ (but might get expensive with shipping, could use other encoder to reduce cost)
- 1x DAC [Adafruit UDA1334A](https://learn.adafruit.com/adafruit-i2s-stereo-decoder-uda1334a/raspberry-pi-wiring) 7€ (not available anymore) can find a copy on Amazon CJMCU-1334, 5x for 33€ =~ 5€ one piece (the DAC is optional and could be replaced by a USB audio card)
- 3x MPR121 capcitive sensor about 2.5€ per piece

Total of about 60€...

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/hardware/ZicPixel/pixel_mpr121.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/hardware/ZicPixel/pixel_front.png" />

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

**LCD Display 1.3in 240x240 SPI Interface ST7789**

To be able to use the display, a custom fork of SDL need to be installed.

```sh
sudo apt-get install libbcm2835-dev
git clone https://github.com/apiel/SDL.git -b SDL2
cd SDL
mkdir build
cd build
../configure
make
sudo make install
```

Then run zicBox like this:

```sh
sudo SDL_VIDEODRIVER=dummy ./zicBox
```

wip:

```sh
sudo apt-get install libbcm2835-dev
git clone https://github.com/apiel/SDL.git -b st7789
cd SDL
mkdir build
cd build
../configure
make
sudo make install
```

Then run zicBox like this:

```sh
sudo SDL_VIDEODRIVER=st7789 ./zicBox
```
