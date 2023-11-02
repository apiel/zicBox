## Installation

Audio:

```sh
sudo apt install librtaudio-dev librtmidi-dev libsndfile1-dev
```

User interface:

```sh
sudo apt-get install fonts-roboto libsdl2-dev libsdl2-ttf-2.0-0 libsdl2-ttf-dev
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

UI is fully customizable using the `./ui/index.ui` file.

> To get styling for .ui extension in VScode, use `Ctrl` + `Shift` + `P` and type `Change Language Mode`. Then select `Configure File Association for '.ui'...` and select `CoffeScript`.

Those `.ui` files are simple configuration files. It is base on [DustScript](https://github.com/apiel/dustscript) mainly using `KEY:VALUE` to set the configuration and providing some extra feature on top of it, like variable, if statement and while loop. However, using those scription features are not mandatory and the UI can be fully configured without it.

The user interface is composed of views and components. A view is composed of multiple components. A component is for example a button or a rotary encoder. Each of the UI components are external shared library that can be loaded dynamically when the application start.

### Basic keys

#### INCLUDE

A `.ui` file can include another `.ui` file, using `INCLUDE: path/of/the/file.ui`. As soon as this `INCLUDE` is called, it will continue into the child file till his end, to finally come back to the original file.

```coffee
INCLUDE: path/of/the/file.ui
```

#### PLUGIN_COMPONENT

A component must be load from a shared library (those `.so` files). To load those plugin components, use `PLUGIN_COMPONENT: given_name_to_component ../path/of/the/component.so`.

```coffee
PLUGIN_COMPONENT: Encoder ../plugins/build/libzic_EncoderComponent.so
```

In this example, we load the shared library `../plugins/build/libzic_EncoderComponent.so` and we give it the name of `Encoder`. The `Encoder` name will be used later to place the components in the view.

#### PLUGIN_CONTROLLER

Beside the user interface, there is as well hardware controllers, like external midi controller or the builtin buttons and pots. To be able to interact with the user interface, those hardware controllers must also be loaded in the application, using `PLUGIN_CONTROLLER: ../path/of/the/controller.so`

```coffee
PLUGIN_CONTROLLER: ../plugins/build/libzic_MidiEncoderController.so
DEVICE: Arduino Leonardo:Arduino Leonardo MIDI 1
```

Some controller can get extra configuration. Any `KEY: VALUE` following `PLUGIN_CONTROLLER: ` will be forwarded to the controller. In this example, we say to the controller to load the midi device `Arduino Leonardo:Arduino Leonardo MIDI 1`.

#### VIEW

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

#### COMPONENT

To place previously loaded components inside a view, use `COMPONENT: given_name_to_component x y w h`.

```coffee
COMPONENT: Encoder 100 0 100 50
ENCODER_ID: 1
VALUE: MultiModeFilter RESONANCE
```

A component can get extra configuration settings and any `KEY: VALUE` following `COMPONENT: ` will be forwarded to the component.
In this example, we assign the hardware encoder id 1 to this component and we assign it to the resonance value from the multi mode filter audio plugin.

#### SET_COLOR

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

### Components

TBD.: Every component can set a group

#### ButtonComponent

TBD.

Button component has 2 states, `pressed` and `release`. Those 2 states are optionals and can do different action. The default action is to set a plugin value:

```coffee
COMPONENT: Button 600 0 100 50
# ON_PRESS: PluginName VALUE_KEY 0.0
ON_PRESS: Distortion DRIVE 40.0
ON_RELEASE: Distortion DRIVE 0.0
LABEL: My button
```

Button can set a view using `$SET_VIEW`:

```coffee
COMPONENT: Button 600 0 100 50
# ON_RELEASE: $SET_VIEW view_name
ON_RELEASE: $SET_VIEW my_name
LABEL: My button
```

To load set the previous view, use `$SET_VIEW $previous`

```coffee
COMPONENT: Button 600 0 100 50
ON_PRESS: $SET_VIEW my_name
ON_RELEASE: $SET_VIEW $previous
LABEL: Shifted view
SHARED_COMPONENT: Test
```

Button can trigger a midi note with `$NOTE_ON` and `$NOTE_OFF`:

```coffee
COMPONENT: Button 200 0 300 50
# ON_PRESS: $NOTE_ON PluginName note velocity
ON_PRESS: $NOTE_ON Kick23 48 127
# ON_RELEASE: $NOTE_OFF PluginName note
ON_RELEASE: $NOTE_OFF Kick23 48
LABEL: Trigger
```

> Do not forget the `$NOTE_OFF` when synth have sustained notes, else the note will keep playing.


## Controllers

> TBD...

## zicHost

ZicBox can be used without used interface, either by using zicHost application or by loading zicHost shared library into another application.

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

### Audio plugins

ZicHost take care to load and handle audio plugins. Each plugins have access to the 32 audio tracks buffer to manipulate them. Those plugins are called in loop in the same order as they have been instanciated. A plugin can be loaded multiple times under a different name.

### zicHost.so

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
