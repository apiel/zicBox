Chain:

clip next
clip load
clip add to chain

play chain
remove last from chain
clear chain (but this could optional)
loop mode: loop / hold

mode: scatter / trigger




pad layout of z,x,c,v:
- default: 
  - z: Chain pop (remove last item) (need to find a short name)
  - x: Chain add rest (need to find a short name)
  - c: Chain mode: loop / hold (need to find a short name)
  - v: mode: scatter / trigger (what does the last row do, toggle scatter vs trigger mode)
- when a clip pad is presed:
  - z: load clip mext
  - x: load clip now
  - c: add clip to chain
  - v: nothing

For the renderDynamicPadMatrix:
. first and second row are for clips (like it is now)
. third row is to start chain for the given track
. fourth row is either trig or scatter

In the chain area, under the master audio output, we need to show the chain from the 8 tracks. If u need more space, you can make the faddeer and the master audio output waveform smaller. To get inspiration on how chain should look like, have a look at zicXYv2, but feel free to make something more modern.




TODO add delete clip
    should we allow to give a name?
    what about copy and paste?