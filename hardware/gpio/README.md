# zicBox GPIO

To be able to access GPIO pins for the hardware interface (rotary encoder and buttons) the application need to run as root. In order to void to run zicBox as root, we run a separate application to access the GPIO and communicate with the main zicBox application through OSC messages.

## Install

```sh
sudo apt-get install libpigpio-dev liblo-dev
```
