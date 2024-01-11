# ZicBox

Customizable digital synth / groovebox...


| <img src='https://github.com/apiel/zicBox/blob/main/images/proto2.jpg?raw=true' width='480'> | <img src='https://github.com/apiel/zicBox/blob/main/images/demo3.png?raw=true' width='480'> | <img src='https://github.com/apiel/zicBox/blob/main/images/demo4.png?raw=true' width='480'>  | 
| :---:   | :---: | :---: | 


## Installation

```ssh
git clone --recurse-submodules git@github.com:apiel/zicBox.git
```

> If repo has already been cloned but submodule are missing, use `git submodule update --init` to pull them.

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

If not started audomatically, it can be started with:

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

## UI

UI is fully customizable using the `./config.ui` file.

> To get styling for .ui extension in VScode, use `Ctrl` + `Shift` + `P` and type `Change Language Mode`. Then select `Configure File Association for '.ui'...` and select `CoffeScript`.

Those `.ui` files are simple configuration files. It is base on [DustScript](https://github.com/apiel/dustscript) mainly using `KEY:VALUE` to set the configuration and providing some extra feature on top of it, like variable, if statement and while loop. However, using those scription features are not mandatory and the UI can be fully configured without it.

The user interface is composed of views and components. A view is composed of multiple components. A component is for example a button or a rotary encoder. Each of the UI components are external shared library that can be loaded dynamically when the application start.
