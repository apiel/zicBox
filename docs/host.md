# zicHost

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
