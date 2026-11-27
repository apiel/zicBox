we need a way to trigger a sound, with computer keyboard, but i would say also from UI.

it still use lot of CPU, i guess it is because UI is not properly optimised sometime it just reach 100% even doing nothing. There is no point to rerender the full UI, if nothing changed, however some part are animed, only those should change. I guess we can split the screen into small part...


For the engine, the cog is super ugly, also it should be on the right side. And engine label should be properly aligned.

For FM4, I feel like the algorithm representation is a bit too complicated and could be simplified. Also animation doesnt render so well.

Need to improve design of kick drum sequencer

create drum rack engine, using FM1 808/909 engine, sequencer would have multiple drums...

wavetable engine.


for whatever reason a note cannot be longer than 8 steps, i want a note to be able to be longer... Also if the note is too long and out of bound, it should restart at the beginning of the screen... Also we need to be able to set 64 steps: 16, 32, 64.
