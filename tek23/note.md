# Todo list

- TODO +- should visible and we can scroll over and state should be saved for next generation... so when generating next, we apply again the same logic if different than default. It might even impact the generator directly.... ??

- TODO see if wavetable generator is enough, instead of FM. let see if FM on his own really bring something better than the wavetable, and maybe we could even bring FM in wavetable..

- TODO rename to Zic23

- TODO wavetable engine
- TODO random sample player

- TODO improve bass track generator

- TODO touch pad would be great

- NOTE should there be way to save??
  - save preset?
  - save track with pattern into clips: then we could chain clips...

- TODO undo, or state management. When generating a new pattern, we should be able to go back to previous one, but then again... maybe we need a pattern generation history

- NOTE could we have a second button for pattern generation, which generate a variation of the current pattern

- TODO cache tracks, if there probabilty only regenerate the track if current step differ from previous --> but then how to do with reverb and delay..

## Idea

- IDEA if we want to push the concept to something a bit more complicated, on the track there could be a + button, to first add sound generator, and add extra FX on top of it.
      - we could even compose the sound generator with many small pieces: often a sound engine is composed of multiple part, like an OSC, a sub osc, noise, and so on. The problem is how to configure all this from the UI and keep it simple??.. Then could it goes even further to add LFO or extra envelop for modulation which would make it even more complicated :p
    - But maybe we could do this just for the FX
