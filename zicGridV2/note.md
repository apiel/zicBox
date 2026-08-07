TODO add delete clip
    should we allow to give a name?
    what about copy and paste?

TODO chain POP,REST,LOOP are too easy to click could create mistake
    instead we should have chain button, which when it is pressed, show the POP,REST,LOOP

TODO note repeat

TODO instead of trig/scatter, if seq is playing, then show scatter, else show trig
            make configurable scatter which can also do note repeat

TODO menu to turn off and so on ...




Update pad layout:

when not playing
CLIP LOAD ADD CHAIN
when playing
CLIP NEXT ADD CHAIN
CLIP NOW  ADD CHAIN

Then when CHAIN is pressed
POP REST LOOP CHAIN
when chain is released we go back to previous state

When CLIP is pressed
CLIP DELETE NAME empty
When CLIP is released we go back to previous state

PAD clip behavior should change as well
When sequencer is off, pressing a pad clip, loads the current clip.
When sequencer is playing and chain is playing, pressing a pad clip, adds the clip to the chain.
When sequencer is playing and chain is not playing, pressing a pad clip, will load clip next (start to play the clip at the next loop). Pressing it again when already set to load next will load right away.