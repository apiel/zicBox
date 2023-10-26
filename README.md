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

> To get styling for .ui extension in VScode, use `Ctrl` + `Shift` + `P` and type `Change Language Mode`. Then select `Configure File Association for '.ui'...` and select `Properties`.

Those `.ui` files are simple configuration files using `KEY=VALUE`. Each lines is parsed one after the other, to load the UI in memory, meaning that the order of each configuration steps matter. Comment can be applied by placing a `#` at the beginning of the line (a comment, cannot be done at the end of a line).

The user interface is composed of views and components. A view is composed of multiple components. A component is for example a button or a rotary encoder. Each of the UI components are external shared library that can be loaded dynamically when the application start.

## Basic keys

### INCLUDE=

A `.ui` file can include another `.ui` file, using `INCLUDE=path/of/the/file.ui`. As soon as this `INCLUDE` is called, it will continue into the child file till his end, to finally come back to the original file.

```ini
INCLUDE=path/of/the/file.ui
```

### PLUGIN_COMPONENT=

A component must be load from a shared library (those `.so` files). To load those plugin components, use `PLUGIN_COMPONENT=given_name_to_component ../path/of/the/component.so`.

```ini
PLUGIN_COMPONENT=Encoder ../plugins/build/libzic_EncoderComponent.so
```

In this example, we load the shared library `../plugins/build/libzic_EncoderComponent.so` and we give it the name of `Encoder`. The `Encoder` name will be used later to place the components in the view.

### PLUGIN_CONTROLLER=

Beside the user interface, there is as well hardware controllers, like external midi controller or the builtin buttons and pots. To be able to interact with the user interface, those hardware controllers must also be loaded in the application, using `PLUGIN_CONTROLLER=../path/of/the/controller.so`

```ini
PLUGIN_CONTROLLER=../plugins/build/libzic_MidiEncoderController.so
DEVICE=Arduino Leonardo:Arduino Leonardo MIDI 1
```

Some controller can get extra configuration. Any `KEY=VALUE` following `PLUGIN_CONTROLLER=` will be forwarded to the controller. In this example, we say to the controller to load the midi device `Arduino Leonardo:Arduino Leonardo MIDI 1`.

### VIEW=

The user interface is composed of multiple views that contain the components. A view, represent a full screen layout. Use `VIEW=name_of_the_veiw` to create a view. All the following `COMPONENT=` will be assign to this view, till the next view.

```ini
VIEW=Main

# some components...

VIEW=Mixer

# some components...
# ...
```

### COMPONENT=

To place previously loaded components inside a view, use `COMPONENT=given_name_to_component x y w h`.

```ini
COMPONENT=Encoder 100 0 100 50
ENCODER_ID=1
VALUE=MultiModeFilter RESONANCE
```

A component can get extra configuration settings and any `KEY=VALUE` following `COMPONENT=` will be forwarded to the component.
In this example, we assign the hardware encoder id 1 to this component and we assign it to the resonance value from the multi mode filter audio plugin.

### SET_COLOR=

`SET_COLOR` give the possibility to customize the pre-defined color for the UI. To change a color, use `SET_COLOR=name_of_color #xxxxxx`.

```ini
SET_COLOR=overlay #00FFFF
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

## Components

> TBD...

## Controllers

> TBD...

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
