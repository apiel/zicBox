
ZicHost take care to load and handle audio plugins. Each plugins have access to the 32 audio tracks buffer to manipulate them. Those plugins are called in loop in the same order as they have been instanciated. A plugin can be loaded multiple times under a different name.

[TBD...]


## Tempo

Tempo audio module is responsible for clocking events. The main purpose is to send clock events to other plugins.
A good example is the sequencer.


### Values

- `bpm` in beats per minute

### Config file

- `BPM: 120.0` to set default beat per minute



## Encoder2Component

<img src="data:image/png;base64,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" />

Encoders are used to control values.


### Config file

- `VALUE: pluginName keyName` is used to set the value to control

- `ENCODER_ID: 0` is used to set the encoder id that will interract with this component

- `TYPE: ...` is used to set the encoder type
    - `TYPE: BROWSE` TBD..
    - `TYPE: TWO_SIDED` is use for centered values like PAN <img src="data:image/png;base64,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" />


- `LABEL: custom_label` overwrite the value label

- `COLOR: #3791a1` set the ring color

- `BACKGROUND_COLOR: #000000` set the background color

- `TEXT_COLOR: #ffffff` set the text color

- `KNOB_COLOR: #888888` set the knob color (middle circle)

- `FLOAT_PRECISION: 2` set how many digits after the decimal point (by default none)

- `SHOW_KNOB: FALSE` show the knob (middle circle) (default TRUE)

- `SHOW_GROUP: TRUE` show group if the component is part of the current active group (default FALSE)

- `SHOW_VALUE: TRUE` show value (default TRUE)

- `SHOW_UNIT: TRUE` show unit (default TRUE)

- `FONT_UNIT_SIZE: 12` set the unit font size


Beside the user interface, there is as well hardware controllers, like external midi controller or the builtin buttons and pots. To be able to interact with the user interface, those hardware controllers must also be loaded in the application, using `PLUGIN_CONTROLLER: ../path/of/the/controller.so`

```coffee
PLUGIN_CONTROLLER: ../plugins/build/libzic_MidiEncoderController.so
DEVICE: Arduino Leonardo:Arduino Leonardo MIDI 1
```

Some controller can get extra configuration. Any `KEY: VALUE` following `PLUGIN_CONTROLLER: ` will be forwarded to the controller. In this example, we say to the controller to load the midi device `Arduino Leonardo:Arduino Leonardo MIDI 1`.


> TBD...



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


## Configuration

ZicBox is fully customizable using the `./config.ui` file.

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



ZicBox can be used without user interface, either by using zicHost application or by loading zicHost shared library into another application.

The zicHost application can be controlled by midi, by assigning a midi message to an audio plugin value.

```coffee
# Here we define the Digitone as midi controller
MIDIIN: Elektron Digitone MIDI 1

AUDIO_PLUGIN: AudioInput ./plugins/audio/build/libzic_AudioInputPulse.so
DEVICE: alsa_input.usb-Elektron_Music_Machines_Elektron_Digitone_000000000001-00.analog-stereo

AUDIO_PLUGIN: Distortion ./plugins/audio/build/libzic_EffectDistortion.so
# Here we assign message to control the drive distortion
# Where xx is the variable value that will be from 0 to 127
# And b0 48 is the fixed part of message corresponding to CC channel 1 number 0x48 (or 72)
DRIVE: b0 48 xx

AUDIO_PLUGIN: MultiModeFilter ./plugins/audio/build/libzic_EffectFilterMultiMode.so
CUTOFF: b0 4c xx
RESONANCE: b0 4d xx
```

## Audio plugins

ZicHost take care to load and handle audio plugins. Each plugins have access to the 32 audio tracks buffer to manipulate them. Those plugins are called in loop in the same order as they have been instanciated. A plugin can be loaded multiple times under a different name.

## zicHost.so

Usage example:

```cpp
#include <dlfcn.h>
#include <stdio.h>

int main()
{
    const char* path = "./zicHost.so";
    printf("Loading host from %s\n", path);
    void* handle = dlopen(path, RTLD_LAZY);
    if (handle == NULL) {
        printf("Failed to load %s: %s\n", path, dlerror());
        return 1;
    }
    printf("Loaded %s\n", path);
    dlclose(handle);
}
```

Build: `g++ -o demo demo.cpp`
