local core = require("config/pixel/libs/core")

---------------------------- AUDIO -----------------------------

-- require("config/pixel/audio/main")
core.loadDustConfig("config/pixel/audio/main.cfg")

--------------------------- CONTROLLER -------------------------

core.controller("MidiEncoder", "libzic_MidiEncoderController.so",
    {
        { "DEVICE",         "Arduino Leonardo:Arduino Leonardo MIDI 1" },
        { "ENCODER_TARGET", "7 0" },
        { "ENCODER_TARGET", "6 1" },
        { "ENCODER_TARGET", "5 2" },
        { "ENCODER_TARGET", "4 3" },
        { "ENCODER_TARGET", "3 4" },
        { "ENCODER_TARGET", "2 5" },
        { "ENCODER_TARGET", "1 6" },
        { "ENCODER_TARGET", "0 7" },
    }
)

------------------------------ UI ------------------------------

require("config/pixel/ui/main")
