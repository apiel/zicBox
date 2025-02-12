# 04 Hardware

## zicBox GPIO

To be able to access GPIO pins for the hardware interface (rotary encoder and buttons) the application need to run as root. In order to void to run zicBox as root, we can run a separate application to access the GPIO and communicate with the main zicBox application through OSC messages.

Everything is in `hardware/gpio/` folder.

However this is not mandatory and can instead use `EncoderController` and `Mpr121Controller` directly in ZicBox, in counter part of running it as root.

### Install

```sh
sudo apt-get install libpigpio-dev liblo-dev
make build
```

> [!NOTE]
> - `TODO` run zicGpio as a service.
