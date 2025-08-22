## Pixel TODO

- TODO pixel seq, add knob to scroll

- TODO reload variations when switching workspace

- TODO mute shortcut, make it part of the Volume plugin, it should save last state before to mute. (in grid & pixel, use shift to mute?)

- FIXME tempo issue

- FIXME sample pitch

- TODO improve multi drum:
- TODO might not need sampleOn sampleOff ?? or maybe yes to keep the logic in one place... <--- subjective
- TODO linearInterpolation in generator class repeated. need to see if we can use the already existing one...
- TODO add velocity to all drum engines
- TODO optimize drum engines

- TODO multi synth engine, like drum engine

- TODO pattern generative for given style of music

- TODO pixel should i had on master the EffectBandIsolatorFx and EffectFilteredMultiFx? or simple multi fx or filter?

- TODO provide a way to save preset on github repo
            - ---> see how to handle ssh or instead use token, but the idea would be to store as less as possible info on zicBox app side (if possible nothing) and delegate everything to the OS...

- TODO look at MiniDexed for bare metal OS https://github.com/probonopd/MiniDexed

- TODO multi drum kick engine: work on the frequency envelop (ask Michi :p)


- TODO GRID variation
        - -----> show pattern in variation <<-- NOTE 
        - would need a way to save all variation at once
              - ----> right now pressing shift + bank reload the current variation
                    - instead if pressing bank with selecting the same bank would reload the current variation
                    - then we could use shift to either save all variation to their current selected variation
                      or use it to reload all variations.... (maybe save is more important) 
        - would need a way to reload all variation at once
-----------------------------------

- TODO drag and drop editor!! 
    ---> should this be a web interface, that show result in live on the hardware... The web interface just allow to drag some abstract element and let us configure them, but final rendering happen in the hardware..

-----------------------------------

- TODO try those multi dac stuff: https://diyelectromusic.com/2024/05/27/rpi-5-quad-stereo-sound-with-pcm5102a/

- TODO send event set tempo

- TODO transpose
        - might be necessary to put transpose within the synth engine itself, with sequencer would be too complicated... ---> if note off is trigger after transpose changed..
        - OR SHOULD we even transpose??

- TODO add granular track?
    - should it just be a sample track
    - or should it be a master effect track that can collect sound from multiple track and shape them in live, could be between scatter effect and sequencer....

- TODO dsi display
      - sudo sh -c 'setterm --cursor off --blank force --clear > /dev/tty1'

- TODO instead to name each synth with a different name stick to their orignal name and use track to distinct them, so no need of Drum1, Drum2, Drum3...

- TODO mouse on rpi
- TODO enable motion/mouse on desktop
  - when mouse over envelop, some point/circle appear to show what can be dragged...

- TODO timeline samples arranger... / and bunch of effect on top of it like granular...

- TODO have a look at https://github.com/micknoise/Maximilian might use this...

- FIXME pulse not working in zicOs!! seems to work???

- FIXME zicOs slow to start, seems to be because of udhcpc
  - so maybe the wifi connection should not be a priority and even start it on demand and not as a service
  - also we have to look if alternatively, we could communicate over usb... but isnt the usb port only to power...
  - investigate if patchbox wouldn't be better or even raspi os lite...
  - remove useless service like cron


- TODO need a way to have a list of preset

- TODO capture, like on ableton move

- TODO play sound dj style :p so next to live playing, could also play a full sound...

- TODO RPi3 version: when pressing mode button in seq view, usually use to show keyboard mode, we could show directional button...

- TODO patch or pattern morphing between 2 kicks for example...

- TODO set wifi config

- FIXME tempo issue between thread ^^ (need to figure out how to reproduce)

- TODO optimize drum engine by caching them... <-- the problem is that what use lot of resource are actually the effect, then caching become way more complicated!
              - create a vector of note + buffer[48 * 5000] or 48k * 5 (5sec <--> 5000ms) --> it would use about 0.9155 MB of RAM per note
              - when note on trigger, we look in the vector if note is already cached
                  - --> if not, we generate the cache for it
              - once we found the cached not, we assign his buffer to the main buffer reference to be played
              - if a parameter change, we clear the vector, and the whole caching process start again
- TODO optimize SynthMetalic

- TODO cache tracks or bar into an audio buffer <-- this can only work if there if no step condition, no step motion, no sample and hold...

- TODO advance sample player (with granular??)

- TODO alsa auto scale latency: Dynamic latency management

- TODO synth with multiple swappable engines!!

- TODO record drums to play them in drum rack...
- TODO drum rack (polyphonic?)

- IDEA hardware could use the direction button...

- TODO master track --> add effect: sample reducer, distortion, delay...? 
    - --> instead of mixer drum and mixer synth, use sent effect to track fx1 and track fx2
- TODO perform / grid effect / scatter effect: apply effect for some time, apply granular effect, pitch down

- TODO instead of EffectGainVolume could use Mixer to set volume

- TODO random patch

- TODO wavetable generator

- TODO wavetable synth: add freq mod curve?

- TODO mouse on raspi

- TODO list value --> for sample browser, to have a list of file...

- TODO synth!!

- TODO equalizer!

- FIXME fix envelop when we release before to reach decay point...

- TODO view history, to be able to go back to previous view...

- TODO reuse clip view, but instead give a way to assign any track to a given spot, but also scatter/grid effect... and then 4 button to select them...

- TODO more steps in sequencer

- TODO use helpers/format.h fToString everywhere instead of custom precision formating

- TODO data id, do something if the ID does not exist... and maybe not rely at all on numeric id anymore!

- TODO clip scrollbar

- FIXME transition between wavetable and waveform is not so smooth...

- TODO Filter... make an advance filter with muliple choice, basic LPF filter, HPF, MMfilter, moog filter, and more...

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

- TODO cleanup draw text, need some refactoring there... a bit messy

- TODO Use Smart Pointers (std::unique_ptr or std::shared_ptr)
   Use tools like Valgrind (Linux/macOS) or AddressSanitizer (Clang/GCC) to check for memory leaks:
```sh
valgrind --leak-check=full ./pixel.x86
```

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
-                      https://www.elektronauts.com/c/the-lounge/other-gear/23

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

then `run`


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

# i2s expender

MCP23017

# bare metal or rtos

Try to build a baremetal synth https://github.com/rsta2/circle/tree/master/sample/29-miniorgan
                               https://github.com/rsta2/circle/blob/master/sample/34-sounddevices/oscillator.cpp
                               https://github.com/rsta2/minisynth

or to build something on freertos