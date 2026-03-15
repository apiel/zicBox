# Todo list

- TODO improve bass track engine and generator

- TODO touch pad would be great

- NOTE should there be way to save??

- TODO undo, or state management. When generating a new pattern, we should be able to go back to previous one, but then again... maybe we need a pattern generation history

- NOTE could we have a second button for pattern generation, which generate a variation of the current pattern

- TODO when pressing D, we duplicate the content of the already edited sequencer. To know which track to apply this, we just do it base on the currently selected step. 
    The logic is the following, we split the sequencer in 4, 8, 16, 32:
    - if only 4 step are edited, then we duplicated those for step to the next 4, so we get 8 step
    - if only 8 step are edited, then we duplicated those for step to the next 8, we 16 steps
    - ...
  - so for example we first edit the first 4 step, we click D, we get a copy of those 4 step just after, then we click D again, we get a copy of the 16 steps after...

## Idea

- IDEA if we want to push the concept to something a bit more complicated, on the track there could be a + button, to first add sound generator, and add extra FX on top of it.
      - we could even compose the sound generator with many small pieces: often a sound engine is composed of multiple part, like an OSC, a sub osc, noise, and so on. The problem is how to configure all this from the UI and keep it simple??.. Then could it goes even further to add LFO or extra envelop for modulation which would make it even more complicated :p
    - But maybe we could do this just for the FX
