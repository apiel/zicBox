# Pico

Knobs:
Volume MasterFxValue1 MasterFxValue2  <------ so there would be a master FX that doesnt change if patch change. This master FX has 2 values similar to mc101 master fx
Enc1 Enc2 Enc3 Enc4
Enc5 Enc6 Enc7

All encoder are to edit patches, however Enc4 is push button so, should it edit the patch or used for menu navigation only?

With esp32:
https://www.hackster.io/news/minimizing-an-esp32-s3-development-board-down-to-the-module-itself-e6170256de30

# zic fiterbank

add passive mixer 5 stereo input :-)

# Next hardware

- 28 gpio available

- 2pins: i2c -> display, mpr121
- 3pins: DAC
- 18pins: 9 enc
- 5pins: 5 btn

# Mega Synth :p

- 3x
  - 1 Env
    - Modulate AMP
    - Modulate pitch
    - Modulate morph
  - Wavetable OSC
  - LFO / FM OSC
    - Modulate pitch
    - Modulate morph
  - multi FX per OSC or glolbal?

## Pixel TODO

- TODO hot reload, when updating a plugin we could recompile this plugin automatically

- TODO drone engine

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