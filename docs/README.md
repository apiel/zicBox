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


### VIEW

The user interface is composed of multiple views that contain the components. A view, represent a full screen layout. Use `VIEW: name_of_the_veiw` to create a view. All the following `COMPONENT: ` will be assign to this view, till the next view.

```coffee
# VIEW: ViewName

VIEW: Main

# some components...

VIEW: Mixer

# some components...
# ...
```

In some case, we need to create some hidden view. Those hidden views can be useful when defining a layout that is re-used in multiple view. It might also be useful, when a view have multiple state (e.g. shifted view...). In all those case, we do not want those view to be iterable. To define a hidden view, set `HIDDEN` flag after the view name.

```coffee
VIEW: Layout HIDDEN

# some components...
```


### STARTUP_VIEW

`STARTUP_VIEW` can be used to load a specific view on startup. This command should only be call at the end of the configuration file, once all the view has been initialised.

```coffee
#STARTUP_VIEW: ViewName

STARTUP_VIEW: Mixer
```

If `STARTUP_VIEW` is not defined, the first defined view (not `HIDDEN`) will be displayed.


### INCLUDE

A `.ui` file can include another `.ui` file, using `INCLUDE: path/of/the/file.ui`. As soon as this `INCLUDE` is called, it will continue into the child file till his end, to finally come back to the original file.

```coffee
INCLUDE: path/of/the/file.ui
```


### SET_COLOR

`SET_COLOR` give the possibility to customize the pre-defined color for the UI. To change a color, use `SET_COLOR: name_of_color #xxxxxx`.

```coffee
SET_COLOR: overlay #00FFFF
```

In this example, we change the `overlay` color to `#00FFFF`.

```cpp
    .colors = {
        .background = {0x21, 0x25, 0x2b, 255},  // #21252b
        .overlay = {0xd1, 0xe3, 0xff, 0x1e},    // #d1e3ff1e
        .on = {0x00, 0xb3, 0x00, 255},          // #00b300
        .white = {0xff, 0xff, 0xff, 255},       // #ffffff
        .blue = {0xad, 0xcd, 0xff, 255},        // #adcdff
        .red = {0xff, 0x8d, 0x99, 255},         // #ff8d99
    },
```

> This list might be outdated, to get the list of existing colors, look at `./styles.h`


### PLUGIN_COMPONENT

A component must be load from a shared library (those `.so` files). To load those plugin components, use `PLUGIN_COMPONENT: given_name_to_component ../path/of/the/component.so`.

```coffee
PLUGIN_COMPONENT: Encoder ../plugins/build/libzic_EncoderComponent.so
```

In this example, we load the shared library `../plugins/build/libzic_EncoderComponent.so` and we give it the name of `Encoder`. The `Encoder` name will be used later to place the components in the view.


### COMPONENT

To place previously loaded components inside a view, use `COMPONENT: given_name_to_component x y w h`.

```coffee
COMPONENT: Encoder 100 0 100 50
ENCODER_ID: 1
VALUE: MultiModeFilter RESONANCE
```

A component can get extra configuration settings and any `KEY: VALUE` following `COMPONENT: ` will be forwarded to the component.
In this example, we assign the hardware encoder id 1 to this component and we assign it to the resonance value from the multi mode filter audio plugin.
