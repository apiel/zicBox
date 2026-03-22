# Todo list

- TODO sample player which would be called Sample23
    - there should be a folder containing the different file which will be played
        - each file is assigned to a note, if there is 3 files we would have
                  - C4 file 1
                  - D4 file 2
                  - E4 file 3
                  - F4 file 1
                  - G4 file 2
                  - A4 file 3
                  - ...
                  - ----> what should be the logic, like if we have 24 files, should there be 12 files before C4 and 12 after taking C4 as center...? Or should there be something else smarter?
        - randomizer (very important!!!): should it be a sample and hold randomiser, or just a simple randomiser params. Or should this just be part of the sequencer, which can be useful for caching later.
        - should we have a mode which say next sample cannot be played if first one is not finished, useful in case of randomness
        - lets add some granular part...
        - reverb and delay
    - File are stored in AUDIO_FOLDER + "/packs" then in packs folder there is a folder for each pack, for example we could have a 909 folder containing 909 drum, or a spooky folder containing scary horror samples. We should be able to select the pack we want to use.
```c
#ifndef AUDIO_FOLDER
#include "host/constants.h"
#endif
```
    - To load the sample, we should use lib sndfile and i guess best should be to load all the sample in memory, i just hope we dont blow up the ram but today most laptop should have enough ram to handle this.
    - When loading a sample we should apply gain to it, to be sure that they all have the same sound level. I have a helper function for this:
```c
// audio/utils/applySampleGain.h

#pragma once

#include <cstdint>

void applySampleGain(float* buffer, uint64_t count)
{
    float max = 0.0;
    for (uint64_t i = 0; i < count; i++) {
        if (buffer[i] > max) {
            max = buffer[i];
        } else if (-buffer[i] > max) {
            max = -buffer[i];
        }
    }
    float gain = 1.0 / max;
    for (uint64_t i = 0; i < count; i++) {
        buffer[i] = buffer[i] * gain;
    }
}
```


- TODO piano modal

- TODO when triggering note with key 1 to 6, if the selected track in the sequencer correspond to the track we want to trigger, we should instead play the selected note.

- TODO scatter effect

- TODO master filter

- TODO improve bass track generator

- TODO touch pad would be great

- NOTE should there be way to save??
  - save preset?
  - save track with pattern into clips: then we could chain clips...

- TODO undo, or state management. When generating a new pattern, we should be able to go back to previous one, but then again... maybe we need a pattern generation history

- NOTE could we have a second button for pattern generation, which generate a variation of the current pattern

- TODO cache tracks, if there probabilty only regenerate the track if current step differ from previous --> but then how to do with reverb and delay..

- TODO EQ on each track

## Idea

- IDEA if we want to push the concept to something a bit more complicated, on the track there could be a + button, to first add sound generator, and add extra FX on top of it.
      - we could even compose the sound generator with many small pieces: often a sound engine is composed of multiple part, like an OSC, a sub osc, noise, and so on. The problem is how to configure all this from the UI and keep it simple??.. Then could it goes even further to add LFO or extra envelop for modulation which would make it even more complicated :p
    - But maybe we could do this just for the FX

## Simplify even more

- merge hihat and clap into one track

- merge bass / fm / wavetable into one single mega engine with more than 24 params SynthMega23 or just Synth23

- still need the random sample player... maybe not necessarly random. It could play a list of sample, either order or randomly...

- so in the end there would be only 5 tracks

## prompt

please just modify the code to add this feature. Generate all code. But dont modify anything which is not related with this feature. Again dont modify code unecessarly, only implement the feature.