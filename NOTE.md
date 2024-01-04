# TODO

- TODO documentation!!

- use string instead of char...

## keypad

- TODO use keypad as a keyboard while configuring a patch...

## zic sample

- TODO save...
    might need to auto save to be able to save clips
    --> how to trigger save??
        maybe AudioPluginHandler.serialize should became an audioPlugin, then could be trigger from components

- TODO yellow button to select the views
    - short click on yellow button will toggle to the next view
    - long press, combine with the 12 params buttons give access directly to a given params view

- TODO save single track clip under a given name
- TODO load a specific track clip

- TODO ?? when 2 button color are pressed at the same time, Y get first value and X the second ??
- TODO when 1 button, Y +1, X +5

## zicUi

- TODO pattern selector in sequencer
       pattern category

- TODO popup message

- TODO when touching the screen, it could either play a note and change pitch base on Y
                              or start ARP change pitch with Y and change pattern with X
                              or act as a chaos pad
     --> maybe there should be a way to stop the motion populating, so a component could
         take over the motion, e.g. an invisible frame on top of the whole view that would trigger the notes.

- TODO pad ARP, y is taking care of semi tone and x change ARP pattern. Could it even be multi touch?

- TODO pad motion recorder

- TODO button shift can change encoder resolution

- TODO oscEncoder use time to make bigger step...

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

- TODO make audio input and output selectable with val...

- TODO save track patch

- TODO SynthSample
    - fix start position of sample marker when start pos is not to 0
    - morph on wavetable, start and end stick to sustain position when both are touching sustain window
    - spray window

- TODO tempo plugin, that would shalow midi clock in favour of custom clock

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

- TODO Zic_Drum_Kick23 give possibility to play sample in the same time as the engine...
       Should it be also possible to record the generated kick inside a sample?
       Could it become a more generic drum engine...

- TODO wavetable player as in granular it doesnt make sense
        pad one wavetable where X and Y morph on different wavetable...

- TODO puredata plugin

- TODO Fm plugin

- TODO reverb, delay
     - load/save different kind of delay and reverb from a config file
                save last state of the delay config... load the last state on start
                base on this state, we can create config files...
     - add lfo on time ratio

- INFO could sequencer use track...?
- TODO make sequencer being able to have multiple target

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

## debug

```sh
gdb zicBox
```

then `bt`.