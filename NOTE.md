## Framework improvement

- TODO enable motion/mouse on desktop

- TODO make all component having keyboard
- TODO make all component access to group and context visibility

### component
- TODO use https://github.com/nlohmann/json for config
    - just need to find a way to inform jsx to whether pass json format or string format
    ---> maybe for the moment should only be for component...
    wget -O json.hpp https://github.com/nlohmann/json/releases/latest/download/json.hpp
    or
    sudo apt install nlohmann-json3-dev

    Keep the config key (command) and only value/params would become json:
    - COMPONENT { pluginName: "Rect", position: [0, 0, 240, 320], color: "background" }
    - PLUGIN_COMPONENT { pluginName: "Rect", path: "@/plugins/components/Pixel/build/x86/libzic_rectComponent.so"  }
    - AUDIO_PLUGIN { pluginName: "Effect", track: 1 }
    - AUDIO_PLUGIN_ALIAS { pluginName: "Effect", path: "@/plugins/audio/build/x86/libzic_effect.so"  }


- TODO use std::string as much as possible and other C++ thing to simplify life :p

- TODO look if container concept is fully used... OR maybe get rid of it!!
  - for example, is plugin still listed under the main view, and receive event if parent container is not visible?
  - ...
  - instead of updatePosition and updateCompontentPosition, container should be responsible to render the child component and decide wether on child should be rendered or not.

### other

- TODO use `#pragma once`

- TODO Use Smart Pointers (std::unique_ptr or std::shared_ptr)
   Use tools like Valgrind (Linux/macOS) or AddressSanitizer (Clang/GCC) to check for memory leaks:
```sh
valgrind --leak-check=full ./pixel.x86
```

- TODO make plugin removable
  
## Pixel TODO

- TODO equalizer!

- TODO drum23 for click instead of noise... use transient samples...

- TODO fix envelop when we release before to reach decay point...

- TODO view history, to be able to go back to previous view...

- TODO sequencer could edit macro... lets give 3 or 4 macro that could be edited by sequencer...

- TODO reuse clip view, but instead give a way to assign any track to a given spot, but also scatter/grid effect... and then 4 button to select them...

- TODO envelop, instead to always calculate time ratio and do linear interpolation, could precalculate step value for each sample by using props.samplerRate to define incrementation steps...

- TODO if step is not enable, if turning encoder step is enable. Todo for all seqs...

- TODO more steps in sample seq and sequencer

- TODO audio plugin do same as for component to load .so file

- TODO use helpers/format.h fToString everywhere instead of custom precision formating

- TODO data id, do something if the ID does not exist... and maybe not rely at all on numeric id anymore!

- TODO clip scrollbar

- NOTE should we move Ui in separate thread to have better understanding of CPU usage

- IDEA load/unload instrument dynamically
    - --> drum (or simple) only with 7 parameters (should it even allow to have something else than knob/values representation..?)
        - sorting ofr the params, would define how they show in the UI
        - show we allow noteOn to get note value? If yes, StepEditMono would need to allow semitone edition...
    - --> synth 23 parameters.. but more complex because for sure we want to show waveform, envelop... also how to name the page of each parameters..

- TODO need to rework the snare to have this clappy sound

- TODO FM synth --> algo param to decide which kind of waveform to combine
              - --> and if it does FM or substractive ...
              - --> filter envelop

- FIXME transition between wavetable and waveform is not so smooth...

- TODO instead of EffectGainVolume could use Mixer to set volume

- TODO reduce data id in SynthDrum23 and update DrumEnvelopComponent or simply remove it
- TODO fix envelop step edit

- TODO clips
  - instead to play right away when clicking toggle button, maybe it should play at the next loop, and we need to play right away the would be a different combination key
  - allow to start all the clip from the same row at once, maybe instead of the play button or else add a new column
  - delete button should ask confirmation (as workspace is saving automatically cannot restore)
  - clips allow more than 16 by adding a scrolling system
  - give a way to make some extra color code to classify depending if it is used for playibg live or for generating sample?

- TODO Filter... make an advance filter with muliple choice, basic LPF filter, HPF, MMfilter, moog filter, and more...

- TODO master track --> add effect: sample reducer, distortion, delay...? 
    - --> instead of mixer drum and mixer synth, use sent effect to track fx1 and track fx2
- TODO perform / grid effect / scatter effect: apply effect for some time, apply granular effect, pitch down

- TODO add color code in text char...
- TODO paragraph --> multiline text component

- TODO filebrowser
    - serialised might want to use filename instead of file index

- IDEA instead of 4 op fm synth make a synth with 3 osc (or even only 2?) that can be fm or not but also use different waveform from wavetable
  - the 3 envelop could be used for amp but also freq?
  - instead of LUT use sinf

- FIXME might not need action2, just have to insert twice the event...

- IDEA val->setString should it instead get an array of string
    - ["Resonance", "37", "%"]
    - ["LP", "78", "%", "HP", "22", "%"]
    - by default using the props to build the string value but can be customised using setString

- TODO make everything start automatically on RPi
    ---> also provide a way to auto update system

- IDEA could we make overlapping component somehow refresh if the one under update?? :p
        --> should this be manual or should it be done automatically
            - if it is automatic it could endup in a mess if UI is badely designed
              could have a flag to enable it:
                    FIND_PARENT_UPDATE: true
            - but to do it manual, we would ne a way to identify component
              or only link with previous component:
                    CHILD_FROM_PREVIOUS: true

- TODO can EffectGrain and EffectDelay be one single effect to make an advance GrainDelay effect...?
- TODO make a simple EffectDelay with preset... or re-use the exisiting one?
- TODO or should it just be GRAIN track, DELAY track and REVERB track (alias delay?)

- IDEA clip/variation could be on one single page for every track
    --> 1 row per track, each row moving from left to right to select a clip
    --> clip can be play instantly at the next loop or in 2 loop (3?, ...)

- TODO use .emplace_back instead of .push_back in vector?
- TODO use std::set when possible

- TODO sequencer play cord

- TODO add modulation on bitcrusher :-)
- TODO wavetable create, where we can pick a specific part of a given wavetable and past it into a new one
       - might also want to use waveform generator and even fm algorigthm?

- TODO port zicPixel to Web assembly

- TODO sudo display

- TODO get rid of SDL component and unify everything
      --> move everything to buffer base UI and only pass use the SDL to render buffer to the screen
      --> same for ST7789 and SSD1306
      --> at the end there should be a common draw library to draw shapes
                and specifc parent lib to render pixels to the screen
      --> however on SSD1306 there cannot be anti alias, so maybe this one should stay separated...

- FIXME thickness on rectangle

- TODO Encoder group: OnEncoder event should take group as param, so if we put an external encoder Interface, we can overwrite the current group? Should it really do this? Maybe it should be the view manager... maybe not :p might still be a good idea to pass it and use this to make the comparison.

- IDEA try deno compile and see if in anyway can be used as config file
      --> https://docs.deno.com/runtime/reference/cli/compile/

- IDEA extra keypad extension: make a 5x5 rgb keypad using keycap but with transparent cap. They can be put in chain, so we could have a 5x10 keypad, or even more...

- TODO TODO
- IDEA cache audio track to reduce computation?
    We should cache several plugin at once, caching a single plugin is not bringing signitificant improvement: e.g. SynthDrumSample (very close to what cache would be) is using about 2% cpu and SynthDrum23 is using 4% cpu. However, SynthDrum23 + Distortion + MMFilter is using 10% cpu.
    See `void set(float value, void* data = NULL)` in `plugins/audio/mapping.h` calling `onUpdateFn` callback function, could use the same concept to trigger recording of a new cache. However, ui is already using this, so either we could need a vector of callback function, or 2 of them... Some way to debounce it would make sense. How to record the cache, without to impact the audio output?
    **Find a way to record one note:** Create an audio plugin to cache output from all previous plugin. This cache plugin would watch for parameter change. If parameter of one of the previous plugin change, it would cache the audio output for a strategic note (low tone note) and then allowing to speedup reading cache to get higher note.
    **All track caching** the whole track caching would make it hard for sequencer motion and probability, most likely not the right way...

- TODO speed up compile time
  - cross compile for rpi directly on laptop

## Improvement performance


- TODO draw should use gpu to render --> opengl
  - does seems to be necessary for the moment, UI on rpi use almost no CPU (at least with the current design)
  - cpu load seems to be higher on desktop because of SDL (i think mainly because of hardware event like keyboard...)

Since Raspberry Pi uses **embedded graphics** and doesn’t always support full OpenGL, **OpenGL ES (GLES) is the best choice**. Here's why:

| Feature         | OpenGL (GLEW) | OpenGL ES (GLES) |
|---------------|--------------|----------------|
| **Supported on Raspberry Pi** | ❌ No (unless using full Mesa OpenGL) | ✅ Yes (default GPU driver) |
| **Supported on Ubuntu Desktop** | ✅ Yes | ✅ Yes (via `libgles2-mesa-dev`) |
| **Lightweight for Embedded** | ❌ No (heavier, desktop-focused) | ✅ Yes (optimized for embedded) |
| **Framebuffer Rendering Support** | 🚧 Harder to configure | ✅ Easier on Raspberry Pi |
| **Anti-Aliasing (MSAA)** | ✅ Yes | ✅ Yes (simpler, but works) |
| **Simple 2D Drawing (Lines, Shapes, AA)** | ✅ Yes | ✅ Yes |
| **Text Rendering** | ✅ Yes (via FreeType/GLFW) | ✅ Yes (but slightly different method) |

- **Raspberry Pi prefers OpenGL ES** → It’s lightweight and works with its GPU.
- **Ubuntu supports OpenGL ES** through `libgles2-mesa-dev`.
- **Framebuffer rendering (SPI Display)** is easier with **GLES on Raspberry Pi**.

Since your **application is 2D-focused**, you don't need full OpenGL. OpenGL ES is enough for **lines, shapes, anti-aliasing, and text rendering**.

How to Set Up GLES for Both Platforms:
**For Ubuntu Desktop (Windowed Rendering)**
Install the necessary packages:
```bash
sudo apt update
sudo apt install libgles2-mesa-dev libglfw3-dev
```
**For Raspberry Pi (Framebuffer Rendering)**
Raspberry Pi’s VideoCore GPU supports GLES natively.
Use EGL (instead of GLFW) to create an OpenGL ES context without a window and render to a framebuffer.

On Raspberry Pi OS, install:
```bash
sudo apt install libegl1-mesa-dev libgles2-mesa-dev
```

Code Approach for Both Platforms:
```cpp
#ifdef USE_GLFWWINDOW // Define this for Ubuntu
#include <GLFW/glfw3.h>
#else
#include <EGL/egl.h>
#include <GLES2/gl2.h> // OpenGL ES 2.0 headers
#endif

#include <iostream>

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Draw a simple rectangle
    glBegin(GL_QUADS);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex2f(-0.5f, -0.5f);
        glVertex2f(0.5f, -0.5f);
        glVertex2f(0.5f, 0.5f);
        glVertex2f(-0.5f, 0.5f);
    glEnd();
    
    glFlush();
}

int main() {
#ifdef USE_GLFWWINDOW
    // Ubuntu version (GLFW windowed rendering)
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    GLFWwindow* window = glfwCreateWindow(800, 600, "GLES Window", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    while (!glfwWindowShouldClose(window)) {
        renderScene();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
#else
    // Raspberry Pi version (Framebuffer rendering)
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, nullptr, nullptr);
    
    EGLConfig config;
    EGLint numConfigs;
    eglChooseConfig(display, nullptr, &config, 1, &numConfigs);

    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, nullptr);
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context);

    while (true) { // Infinite loop for embedded display
        renderScene();
    }

    eglDestroyContext(display, context);
    eglTerminate(display);
#endif

    return 0;
}
```

Next Steps, enable Anti-Aliasing (MSAA):
```cpp
glEnable(GL_MULTISAMPLE);
```
Draw Shapes (Lines, Circles, Arcs)
- Use GL_LINES, GL_LINE_LOOP, GL_TRIANGLE_FAN for smooth shapes.
- You may need a simple 2D shader for better anti-aliasing.
Render Text (Bitmap or Vector)
- Use FreeType or bitmap fonts.
- Convert text to triangles for GLES compatibility.

## Next TODO

- TODO create separate asset folder > might put it in wiki...

- TODO start thread in https://llllllll.co/

- TODO scatter/grid effect
    - when pressing a btn it would apply (only while button is pressed and then revert original setting):
        - enable granular on some specific tracks
        - update filter (could it even apply a modulation on the filter?)
        - update sequencer pattern, for example to make kick rolling
        - apply a reverb/delay or any other kind of effect
        --> So all in all it should be able to apply a set of plugin value for a certain amount of time and reset once the key is released. Also, it might need to be able to trigger noteOn, e.g. for EffectGrain? or maybe not, EffectGrain should be able to run without noteOn

- TODO keyboad should handle longpress (see button...)

- TODO sample:
    - there could be multiple loop area and jump from one to the other, either one after the other or randomly... there could also be a way to define the transition between the loop area, e.g. mixing the 2 area and slowly fadding out from one to the other.
    - wavetable:
        - fix start position of sample marker when start pos is not to 0
        - morph on wavetable, start and end stick to sustain position when both are touching sustain window
        - spray window
    - granularize sample...
    - sample slice

- TODO popup message

- TODO instead of usual sequencer, link multiple sample together and then a note would trigger the series of samples... A bit like wavetstate

- TODO puredata plugin
    - FIXME missing libpd on rpi

- TODO use clap plugin https://nakst.gitlab.io/tutorial/clap-part-1.html
- TODO load vst and lv2 plugin?
  - or even use clap format for audio plugin?

- TODO explore usage of Tensorflow Nsynth

- INFO IPC in order to have a main thread that is running all the time, even if the rest crash...
       https://en.wikipedia.org/wiki/Inter-process_communication

- TODO bitcrusher Resolution reduction: from float 32 bit to 16 to 8 ...

- FIXME audio plugin ALSA, output not working
    - make audio plugin following the same format so they might be compatible...

- TODO jack audio plugin

- TODO handle note on/off from midi keyboard

- TODO handle rotary encoder from midi... (keyboard and osc)

- TODO ? use lv2 plugin
       https://github.com/lv2/lilv/blob/master/tools/lv2apply.c
       https://github.com/jeremysalwen/lv2file

## Hardware

- TODO hardware look for touch encoder or button encoder
    - https://www.mouser.at/datasheet/2/54/PEC12R-777795.pdf
    - 22mm https://www.mouser.at/ProductDetail/Bourns/PEC12R-4022F-S0024?qs=6oMev5NRZMHfikcgfDwwZA%3D%3D
    - 30mm https://www.mouser.at/ProductDetail/Bourns/PEC12R-4030F-S0024?qs=15%2Ff%2FzFZJSBasYPWBClbAA%3D%3D

- TODO mini analog mixer
    - https://www.mouser.at/ProductDetail/Alps-Alpine/RK10J12R0A0B?qs=nR5Mw3RKkX4o8TMzstLlKA%3D%3D

- TODO pixel harmonic https://www.ziaspace.com/_academic/BP_Scale/BP_modes/files/1_3_sharp4_6-copy.png
    https://www.youtube.com/watch?v=DREjkHQpCYQ

## Audio app inspirations

- http://samplr.net/

- https://www.borderlands-granular.com/app/

- https://www.bleass.com/samplewiz2/

## more

- elektron overbridge https://github.com/dagargo/overwitch

- rsta2 circle https://github.com/rsta2/circle Circle is a C++ bare metal programming environment for the Raspberry Pi. (give possibility to run an application without OS..)

- calculate midi note https://gist.github.com/YuxiUx/ef84328d95b10d0fcbf537de77b936cd

## debug

```sh
gdb zicBox
```

then `bt`.

# st7789 7wire

https://github.com/juj/fbcp-ili9341/issues/178#issuecomment-759897048

```sh
git clone https://github.com/juj/fbcp-ili9341.git
cd fbcp-ili9341
```

Then edit display.h and paste in

```sh
#define DISPLAY_SPI_DRIVE_SETTINGS (1 | BCM2835_SPI0_CS_CPOL | BCM2835_SPI0_CS_CPHA)
```
under the line

```sh
#define DISPLAY_SPI_DRIVE_SETTINGS (0)
```

Then continue with

```sh
mkdir build
cd build
```sh

In v2:
```sh
cmake -DST7789VW=ON -DGPIO_TFT_DATA_CONTROL=25 -DGPIO_TFT_RESET_PIN=27 -DGPIO_TFT_BACKLIGHT=18 -DSTATISTICS=0 -DSPI_BUS_CLOCK_DIVISOR=40 -DUSE_DMA_TRANSFERS=OFF -DDISPLAY_ROTATE_180_DEGREES=1 ..
```

In v3:
```sh
cmake -DST7789VW=ON -DGPIO_TFT_DATA_CONTROL=6 -DGPIO_TFT_RESET_PIN=5 -DGPIO_TFT_BACKLIGHT=13 -DSTATISTICS=0 -DSPI_BUS_CLOCK_DIVISOR=40 -DUSE_DMA_TRANSFERS=OFF -DDISPLAY_ROTATE_180_DEGREES=1 ..
```

Finally run

```sh
make -j
sudo ./fbcp-ili9341
```

In `/boot/config.txt`, add in the bottom:

```
hdmi_group=2
hdmi_mode=87
hdmi_cvt=240 240 60 1 0 0 0
hdmi_force_hotplug=1
```

and comment out:
```
# dtoverlay=vc4-kms-v3d
```
