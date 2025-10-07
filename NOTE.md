# Pico

Knobs:
Volume MasterFxValue1 MasterFxValue2  <------ so there would be a master FX that doesnt change if patch change. This master FX has 2 values similar to mc101 master fx
Enc1 Enc2 Enc3 Enc4
Enc5 Enc6 Enc7

All encoder are to edit patches, however Enc4 is push button so, should it edit the patch or used for menu navigation only?

With esp32:
https://www.hackster.io/news/minimizing-an-esp32-s3-development-board-down-to-the-module-itself-e6170256de30

# zic grain

Get one loop sample and play it in continue (?), then the sample is split a frequency layer using the filterbank (let say 3 or 4 layer). Each layer can do a bunch of thing on the sample between, loop point, effect, grain, and so on...

# zic fiterbank

add passive mixer 5 stereo input :-)

## Pixel TODO

- TODO DOC DOC

- TODO kick improve frequency env generator and improve transient!xx
- TODO ER-1 if duration longer than sample, then stretch sample
- TODO drone engine

- TODO wifi setup

- TODO live recording midi keyboard
- TODO test midi keyboard on pixel

- TODO see if we can make OS read only and data on separate partition to reduce chance of corruption

- TODO load pixel has mass storage?

- TODO in menu could use knob or a button to navigate between multiple options...

- TODO might need to have build with arm for zero2w but also for rpi4, not necerssarly the same ...
- rething build / folder
- pixel.json (when we build we should say what should be the name of the config file) + config.json as fallback
- makefile

- TODO write user manual

- TODO improve multi drum:
- TODO might not need sampleOn sampleOff ?? or maybe yes to keep the logic in one place... <--- subjective
- TODO optimize drum engines

- TODO multi sample for pitch..

- TODO reload variations when switching workspace

- TODO conditional plugin loading:
      - Use bash command / script to make a condition...
      - Should it be inline plugin A or B: audioPlugin(['AudioOutputPulse', 'AudioOutputAlsa', 'sh exist pulse'], { alias: 'AudioOutput' }),
      - Or have a check for each ...: audioPlugin('AudioOutputPulse', { alias: 'AudioOutput', condition: 'sh exist pulse' }), <--- seems better to me
      - Should it have a check if plugin name is already used for given track, if it has been used (because previous condition was true) do not load...

- FIXME tempo issue

- TODO DESKTOP :-) version, using pixel design but all in one screen

- TODO ARP for synth, like note repeat, put a encoder to select ARP scale, from none to whatever... the when clicking trigger note, it would play the note or ARP, no matter if sequencer is playing or not.

- TODO pattern generative for given style of music

- TODO pixel should i had on master the EffectBandIsolatorFx and EffectFilteredMultiFx? or simple multi fx or filter?

- TODO try to make design as generic as possible, synth name vs track...

- TODO look at MiniDexed for bare metal OS https://github.com/probonopd/MiniDexed

- TODO try the orange pi!!! we could then remove DAC... would just need audio in

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

- TODO orange pi version: builtin DAC free up a bunch of gpio, also more powerful, 24 small pin header might provide even 2 more gpio?

- TODO play sound dj style :p so next to live playing, could also play a full sound...

- TODO patch or pattern morphing between 2 kicks for example...

- TODO set wifi config

- FIXME tempo issue between thread ^^ (need to figure out how to reproduce)

- TODO alsa auto scale latency: Dynamic latency management

- TODO random patch

- TODO use helpers/format.h fToString everywhere instead of custom precision formating

- FIXME might not need action2, just have to insert twice the event...

- TODO start thread in https://llllllll.co/
-                      https://www.elektronauts.com/c/the-lounge/other-gear/23

- TODO instead of usual sequencer, link multiple sample together and then a note would trigger the series of samples... A bit like wavetstate

- TODO puredata plugin
    - FIXME missing libpd on rpi

- TODO explore usage of Tensorflow Nsynth

- INFO IPC in order to have a main thread that is running all the time, even if the rest crash...
       https://en.wikipedia.org/wiki/Inter-process_communication

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