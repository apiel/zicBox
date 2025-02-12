# 03 Controller plugins

Beside the user interface, there is as well hardware controllers, like external midi controller or the builtin buttons and pots. To be able to interact with the user interface, those hardware controllers must also be loaded in the application, using `PLUGIN_CONTROLLER: ../path/of/the/controller.so`

```coffee
PLUGIN_CONTROLLER: ../plugins/build/libzic_MidiEncoderController.so
DEVICE: Arduino Leonardo:Arduino Leonardo MIDI 1
```

Some controller can get extra configuration. Any `KEY: VALUE` following `PLUGIN_CONTROLLER: ` will be forwarded to the controller. In this example, we say to the controller to load the midi device `Arduino Leonardo:Arduino Leonardo MIDI 1`.
