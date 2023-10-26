# TODO

- TODO documentation!!

- TODO bring zicHost and ZicUi together, still giving the possibility to extract zicHost in app or in shared lib

## zicUi

- TODO hidden view that can host shared component or second state view
      - shared component could be imported using INCLUDE file to have a kind of layout file, meaning that each view could reuse a preset of component. To do we would need a place to define first this shared component. If this layout is reused in all the views, we need a hidden view to host the shared component.
      - A view can have multiple state (think about a shift mode view). While scrolling in `VIEW UI` state, to change from one view to the other, we dont want to scroll through those shifted views... Therefor, we need some hidden views, that's are not scrollable?

- TODO button PRESSED_INT_VALUE
       button can trigger NOTE or should finish note component
       button can easily set view base on name
       can button show a state
       can button show a value

- TODO Significantly improve sequencer UI

- TODO oscEncoder use time to make bigger step...

- TODO slider, e.g. for mixer

- TODO encoder responsive for small size
- TODO use function pointer on encoder initialisation to assign draw function base on type

- TODO list of value, e.g.:
                file1.wav
                file2.wav
              > file3.wav
                file4.wav
                file5.wav

- TODO optimized onUpdate value...?

- TODO keyboard?

- TODO componentEncoder option rotate 90°

```cpp
float rotationAngle = 90.0f;
SDL_RenderCopyEx(renderer, texture, NULL, &destRect, rotationAngle , NULL, SDL_FLIP_NONE);
```

See also https://www.parallelrealities.co.uk/tutorials/ttf/ttf1.php
`blitRotated` is actually using `SDL_RenderCopyEx` https://www.parallelrealities.co.uk/tutorials/bad/bad2.php

- TODO should we show the grains???

- NOTE should we use https://github.com/nlohmann/json for UI ?


## zicHost

- TODO sequencer cleanup? --> stepCount...
- TODO save sequencer state

- TODO try to make granular synth more versatile
            - play sample (already do it but maybe there would be a way to lock spray or other thing like this)
            - play wavetable (give the possibility to give sample coutn instead of len, lock spray, and maybe morphing by bank)

- TODO tempo plugin, that would shalow midi clock in favour of custom clock

- TODO MIXER track=1 vs track_b... should it be track_1 but ...?

- TODO Zic_Drum_Kick23 give possibility to play sample in the same time as the engine...
       Should it be also possible to record the generated kick inside a sample?
       Could it become a more generic drum engine...

- TODO puredata plugin

- TODO Fm plugin

- TODO reverb, delay
     - load/save different kind of delay and reverb from a config file
                save last state of the delay config... load the last state on start
                base on this state, we can create config files...
     - add lfo on time ratio

- INFO could sequencer use track...?
- TODO make sequencer being to have multiple target

- INFO IPC in order to have a main thread that is running all the time, even if the rest crash...
       https://en.wikipedia.org/wiki/Inter-process_communication

- TODO bitcrusher Resolution reduction: from float 32 bit to 16 to 8 ...

- FIXME audio plugin ALSA, output not working
    - make audio plugin following the same format so they might be compatible...

- TODO jack audio plugin

- TODO generate config file example

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
