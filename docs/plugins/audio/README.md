# Audio plugins

ZicHost take care to load and handle audio plugins. Each plugins have access to the 32 audio tracks buffer to manipulate them. Those plugins are called in loop in the same order as they have been instanciated. A plugin can be loaded multiple times under a different name.

[TBD...]


## Tempo

Tempo audio module is responsible for clocking events. The main purpose is to send clock events to other plugins.
A good example is the sequencer.


### Values

- `bpm` in beats per minute

### Config file

- `BPM: 120.0` to set default beat per minute