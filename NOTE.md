# ESP32

| PCM5102 Board | WeActStudio PCM5100A Board                   |
| ------------- | -------------------------------------------- |
| VIN           | VCC                                          |
| GND           | GND                                          |
| LRCK          | WS                                           |
| DIN           | DIN                                          |
| BCK           | BCK                                          |
| SCK           | MC                                           |
| *(none)*      | SD (tie HIGH or check docs for enable logic) |

## Timeline

Let's make a desktop version for now!

IDEA think about timeline view, with no step limit

6 tracks? :p (= 6 encoder ^^)

in addition of midi note, patch are also assign over time. Transition between patch could use morph if they use the same engine, but how about effect? so in the end morph could simply be by playing 2 voices parallel... then we would have to preprocess cause i am not sure rpi zero can handle this.
But then if we have a system with 2 voices, could we even have polyphony...?

## Pixel TODO

- TODO esp32

- TODO simplify draw.text...

- TODO explore https://github.com/microsoft/muzic

- IDEA In serialisation, could save string value as well when it is type VALUE_STRING. Then reload using the string... but not always easy, for example mmfilter cutoff is string, but shouldn't use string to reload it. So, maybe we would have to introduce a new flag, like serialize: STRING, or even provide lambda function to hydrate with string

- TODO sample engine with modulation...

- TODO hot reload, when updating a plugin we could recompile this plugin automatically

- TODO reload variations when switching workspace

- TODO live recording midi keyboard
- TODO test midi keyboard on pixel

- TODO see if we can make OS read only and data on separate partition to reduce chance of corruption

- TODO load pixel has mass storage?

- TODO write user manual

- FIXME tempo issue

- TODO DESKTOP :-) version, using pixel design but all in one screen

- TODO ARP for synth, like note repeat, put a encoder to select ARP scale, from none to whatever... the when clicking trigger note, it would play the note or ARP, no matter if sequencer is playing or not.

- TODO pattern generative for given style of music
- TODO random patch

- TODO try the orange pi!!! we could then remove DAC... would just need audio in

- TODO dsi display
      - sudo sh -c 'setterm --cursor off --blank force --clear > /dev/tty1'

- TODO instead to name each synth with a different name stick to their orignal name and use track to distinct them, so no need of Drum1, Drum2, Drum3...

- TODO mouse on rpi
- TODO enable motion/mouse on desktop
  - when mouse over envelop, some point/circle appear to show what can be dragged...

- TODO orange pi version: builtin DAC free up a bunch of gpio, also more powerful, 24 small pin header might provide even 2 more gpio?

- TODO alsa auto scale latency: Dynamic latency management

- TODO use helpers/format.h fToString everywhere instead of custom precision formating

- FIXME might not need action2, just have to insert twice the event...

- TODO start thread in https://llllllll.co/
-                      https://www.elektronauts.com/c/the-lounge/other-gear/23

# Audio idea

- IDEA drone engine

- IDEA play sound dj style :p so next to live playing, could also play a full sound...

- IDEA patch or pattern morphing between 2 kicks for example...

- IDEA multi sample for pitch... engine

## debug

```sh
gdb zicBox
```

then `run`


then `bt`.

# i2s expender

MCP23017

# bare metal or rtos

Try to build a baremetal synth https://github.com/rsta2/circle/tree/master/sample/29-miniorgan
                               https://github.com/rsta2/circle/blob/master/sample/34-sounddevices/oscillator.cpp
                               https://github.com/rsta2/minisynth

or to build something on freertos