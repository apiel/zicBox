sequencer will always be done no need of the button

dedicated master view

reoganise knobs by 8...:
in order to optimise the UI and make it even more responsive, i think we could have the knob grouped by 8 of 2 row of 4 knobs. For example Fm4 we have 
- operator FM
- AMP and LFO
- Operator envs
- Filter & FX

For Acid32, its even more simple:
- oscillator
- envelopes
- ladder filter
- spatial fx

For kick32v5 its a bit more complicated:
- body, the for 8 params
- body, the 4 other params + filter
- drive the first 8 params
- drive the 8 other params

then we could do something like css FLEX or GRID. which mean in on narrow window we would have, for acid32:
oscillator      ladder filter
envelopes       spatial fx

and on large window, it would be
oscillator    envelopes    ladder filter    spatial fx


