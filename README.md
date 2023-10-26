## Installation

```sh
sudo apt install librtaudio-dev librtmidi-dev libsndfile1-dev
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

### Hint

Get list of audio card:

```sh
cat /proc/asound/cards
```

or 

```sh
arecord -l
```

or

```sh
aplay -l
```

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
