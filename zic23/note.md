# Todo list

- TODO Copy step on the same track, copy all pattern from one track to the other 
  - The next feature to implement is copy/paste using ctrl+c and ctrl+v. When we are on a step, ctrl+c copy a step and track index, when doing ctrl+v on the another step of the same track it copy the step to it. If we do ctrl+v on another track, the whole track is copied.

- TODO random sample player

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

## Idea

- IDEA if we want to push the concept to something a bit more complicated, on the track there could be a + button, to first add sound generator, and add extra FX on top of it.
      - we could even compose the sound generator with many small pieces: often a sound engine is composed of multiple part, like an OSC, a sub osc, noise, and so on. The problem is how to configure all this from the UI and keep it simple??.. Then could it goes even further to add LFO or extra envelop for modulation which would make it even more complicated :p
    - But maybe we could do this just for the FX


## prompt

please just modify the code to add this feature. Generate all code. But dont modify anything which is not related with this feature. Again dont modify code unecessarly, only implement the feature.