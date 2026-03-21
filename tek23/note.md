# Todo list

- TODO +- action should be visible on the UI, next to the square "G" to generate the pattern, we should put anther square showing the current state of "+-", i guess as value we could show 64, 32, 16, 8, 4
  If we can scroll over it, it would also apply the actionlike it would be with the key - and the key +. Now if we generate a pattern again, if the state is different than 64, we apply respectively stretchTrackSequence after generating the sequence.

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


## prompt

please just modify the code to add this feature. Generate all code. But dont modify anything which is not related with this feature. Again dont modify code unecessarly, only implement the feature.