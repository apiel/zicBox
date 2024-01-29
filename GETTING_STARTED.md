# 01 Getting started

ZicBox is working on Linux plateform, the end target is to run on RaspberryPi but it should run on any linux machine. In my case, I am actively using it on my Ubuntu desktop for development purpose. I beleive that it could also easily run on MacOs, but I don't have a Mac, so I cannot try out (if you manage to do so, please let me know).

## Configuration

ZicBox is fully customizable using configuration files. Everyhting is build base on plugins, from user interface, to audio module and hardware controller. ZicBox then wire everything so that everything can communicate together.

The main configuration file is `./config.ui`.

> To get styling for .ui extension in VScode, use `Ctrl` + `Shift` + `P` and type `Change Language Mode`. Then select `Configure File Association for '.ui'...` and select `CoffeScript`.

Those `.ui` files are simple configuration files. It is base on [DustScript](https://github.com/apiel/dustscript) mainly using `KEY:VALUE` to set the configuration and providing some basic extra feature on top of it, like variable, if statement and while loop. However, using those script features are not mandatory and the UI can be fully configured without it.

The user interface is composed of views and components. A view is composed of multiple components. A component is for example a button or a rotary encoder. Each of the UI components are external shared library that can be loaded dynamically when the application start.

From the audio perspective, there is an audio buffer splitted in tracks (32 by default). Audio plugins are assign per track but can also manipulate multiple tracks, like the audio mixer.

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/zicbox.drawio.png" />

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/zicbox.drawio.svg" />

## Installation

```ssh
git clone --recurse-submodules git@github.com:apiel/zicBox.git
```

> If repo has already been cloned but submodule are missing, use `git submodule update --init` to pull them.
> For `ssh` submodule `git submodule update --init .gitmodules_ssh`

Compiler:
```sh
sudo apt install build-essential
```

Audio:

```sh
sudo apt install librtmidi-dev libsndfile1-dev
```

User interface:

```sh
sudo apt-get install fonts-roboto libsdl2-dev libsdl2-ttf-2.0-0 libsdl2-ttf-dev
```

Controller:

```sh
sudo apt-get install liblo-dev
```


### On RPi

Might Need to install pulse audio as well:

```sh
sudo apt-get install alsa-base pulseaudio
```

If not started automatically, it can be started with:

```sh
pulseaudio --start
```

Get list of audio card:

```sh
cat /proc/asound/cards
# or
arecord -l
# or
aplay -l
```
