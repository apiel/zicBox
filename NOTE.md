## Framework improvement

- TODO draw should use gpu to render --> opengl

- TODO use https://github.com/nlohmann/json for config
    - just need to find a way to inform jsx to whether pass json format or string format
    ---> maybe for the moment should only be for component...

- TODO use `#pragma once`

- TODO make plugin removable

- TODO ui component, should they be allowed to have child...

- TODO load vst and lv2 plugin?

## Pixel TODO

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

## Next TODO

- TODO create separate asset folder > might put it in wiki...

- TODO start thread in https://llllllll.co/

- FIXME open playing noteOn

- TODO EffectGrain can be active without triggering any note...

- TODO save/load project

- TODO scatter/grid effect
    - when pressing a btn it would apply (only while button is pressed and then revert original setting):
        - enable granular on some specific tracks
        - update filter (could it even apply a modulation on the filter?)
        - update sequencer pattern, for example to make kick rolling
        - apply a reverb/delay or any other kind of effect
        --> So all in all it should be able to apply a set of plugin value for a certain amount of time and reset once the key is released. Also, it might need to be able to trigger noteOn, e.g. for EffectGrain? or maybe not, EffectGrain should be able to run without noteOn

- TODO advance clip/variation transition...
    - should sequencer take care to load variation?
- TODO sequencer STATUS change logic:
    - -1 should be infinite loop
    - 0 off
    - 1,2,3,... should be how long it wait before to start 
      (or should it be to toggle status, so if it is already playing, it will stop)

- TODO keyboad should handle longpress (see button...)

- TODO sample:
    - there could be multiple loop area and jump from one to the other, either one after the other or randomly... there could also be a way to define the transition between the loop area, e.g. mixing the 2 area and slowly fadding out from one to the other.
    - wavetable:
        - fix start position of sample marker when start pos is not to 0
        - morph on wavetable, start and end stick to sustain position when both are touching sustain window
        - spray window
    - granularize sample...
    - sample slice

- TODO kick23 could change morph position on every noteON. It would either be an incrementation or random or...
    Would this principle make sense as on pitch or other drum parameter?

- TODO FM ui waveform, maybe just show the 4 envelop...

## zicUi

- Responsive UI ?

- TODO python config https://docs.python.org/3/extending/embedding.html

- TODO keyboard / keypad
    - see we can extract some logic to have global action that are not specific to a UI plugin
    - should LED and keypad be the same object
        or could we assume that some LED doesn't necessarly belong to a key
        LED could be associated to an encoder or just completely standalone...

- TODO Delay make a visual representation of the delay.

- TODO live waveform visualisation
    - might need for this to create an audio plugin to keep a buffer of the audio output

- TODO GridSequencer
    - TODO touch could be used to move up/down/left/right
    - TODO save single track variation/clip under a given name
    - TODO load a specific track variation/clip

- TODO make label all at the same position...

- TODO dekstop version
    - display multiple view at once...
    - ZicBox hardware can control ZicBox desktop throw OSC or USB...

- TODO pattern selector in sequencer
       pattern category

- TODO popup message

- TODO spectrum analyser, to show current sound coming out base on frequency and amplitude...

- TODO when touching the screen, it could either play a note and change pitch base on Y
                              or start ARP change pitch with Y and change pattern with X
                              or act as a chaos pad
     --> maybe there should be a way to stop the motion populating, so a component could
         take over the motion, e.g. an invisible frame on top of the whole view that would trigger the notes.

- TODO pad ARP, y is taking care of semi tone and x change ARP pattern. Could it even be multi touch?

- TODO pad motion recorder

- INFO Should SDL in draw be injectable? What if we want to use those i2c 1306 display or similar...

- TODO can button show a state
       can button show a value

- TODO make pad possibility, like dual touch... same as resize in the granular synth ...

- TODO slider, e.g. for mixer

- TODO encoder responsive for small size
- TODO use function pointer on encoder initialisation to assign draw function base on type

- TODO button can call data? Example for saving granular synth...

- TODO list of value, e.g.:
                file1.wav
                file2.wav
              > file3.wav
                file4.wav
                file5.wav

- TODO optimized onUpdate value...?

- TODO ? SequencerBarComponent: Remove text info
                                Make motion optional

- TODO component option rotate 90°

```cpp
float rotationAngle = 90.0f;
SDL_RenderCopyEx(renderer, texture, NULL, &destRect, rotationAngle , NULL, SDL_FLIP_NONE);
```

See also https://www.parallelrealities.co.uk/tutorials/ttf/ttf1.php
`blitRotated` is actually using `SDL_RenderCopyEx` https://www.parallelrealities.co.uk/tutorials/bad/bad2.php

- TODO auto update...
    - from release
    - from git repo?

- filter frequency response curve https://stackoverflow.com/questions/76193236/implement-2nd-order-low-pass-filter-in-c-how-to-compute-coefficients

## zicHost

- TODO libpd do the same as for lua, if lib not installed, skip...?

- TODO instead of usual sequencer, link multiple sample together and then a note would trigger the series of samples... A bit like wavetstate

- TODO look at https://github.com/pichenettes/eurorack/tree/master/shades

- TODO could use lookup table for filter resolution

- TODO SynthSample 
    - fix openned voice
    - multi sample mix 2 (or even 3) sample together

- TODO make audio input and output selectable with val...

- TODO Fm plugin

- TODO puredata plugin
    - FIXME missing libpd on rpi

- TODO save track patch

- TODO use clap plugin https://nakst.gitlab.io/tutorial/clap-part-1.html

- TODO SynthSample
    - fix start position of sample marker when start pos is not to 0
    - morph on wavetable, start and end stick to sustain position when both are touching sustain window
    - spray window

- TODO tempo plugin, that would shalow midi clock in favour of custom clock
    --> give a way to select either midi or internal clock

- TODO explore usage of Tensorflow Nsynth

- TODO modulation that would be a mix of envelop and lfo:
    - multiphase envelop (up to 8 or even more)
    - each phase can configure time and amplitude (should it be ms or percentage of a global amount?)
    - each phase can be linear or a curve to be able to simulate sin...
    - a sustain point can be set at any point of the envelop, also in the middle of a phase
    - to simulate an LFO, the sustain point can loop over a certain window, meaning that sustain as a starting point and a end point

- TODO Sequencer Polyend play style ???

- TODO sequencer 
        - 1/2 step length
        - if step len is 0 it should play infinite

- TODO MIXER track=1 vs track_b... should it be track_1 but ...?

- TODO wavetable player as in granular it doesnt make sense
        pad one wavetable where X and Y morph on different wavetable...

- TODO reverb, delay
     - load/save different kind of delay and reverb from a config file
                save last state of the delay config... load the last state on start
                base on this state, we can create config files...
     - add lfo on time ratio

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

## Pixel

- FIXME
    - mpr121 labelling
    - smaller hole for encoder
    - make alphanum in white instead of silver and may reduce special char
    - make icon in silver instead of white in order to make it more conductive
    - pad effect seem not to work properlly
    - display hole is too big
    - might move number in empty spot? Or need to find a better way to utilize those empty spot..
    - add a pin going to front panel for LED (and 5v)
    - expose Lout and Rout via pin
    
- use all the encoder left to make a mega midi interface...

## method function pointer

See if I can use

```cpp
#include <functional>
#include <vector>

class MyClass1{
public:
    void hello(float value)
    {
        printf("hello %f\n", value);
    }
};

class MyClass2 {
public:
    void world(float value)
    {
        printf("world %f\n", value);
    }
};

using Callback_t = std::function<void()>;

template <typename T>
Callback_t MakeCallback(void (T::*f)(float), float val)
{
    T* p = new T;
    return [=]() { (p->*f)(val); };
}

int main()
{
    std::vector<Callback_t> vetcor = {
        MakeCallback(&MyClass1::hello, 0.123),
        MakeCallback(&MyClass2::world, 0.456)
    };

    for (std::size_t i = 0; i < vetcor.size(); i++) {
        vetcor[i]();
    }
    return 0;
}
```

## Hardware

- TODO hardware look for touch encoder or button encoder
    - https://www.mouser.at/datasheet/2/54/PEC12R-777795.pdf
    - 22mm https://www.mouser.at/ProductDetail/Bourns/PEC12R-4022F-S0024?qs=6oMev5NRZMHfikcgfDwwZA%3D%3D
    - 30mm https://www.mouser.at/ProductDetail/Bourns/PEC12R-4030F-S0024?qs=15%2Ff%2FzFZJSBasYPWBClbAA%3D%3D

- TODO mini analog mixer
    - https://www.mouser.at/ProductDetail/Alps-Alpine/RK10J12R0A0B?qs=nR5Mw3RKkX4o8TMzstLlKA%3D%3D

- TODO pixel harmonic https://www.ziaspace.com/_academic/BP_Scale/BP_modes/files/1_3_sharp4_6-copy.png
    https://www.youtube.com/watch?v=DREjkHQpCYQ

- TODO make gpio interface run as service

- TODO gpio interface use config file

- TODO install file (make setup/prepare and make install ?)
- TODO update using makefile?

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
