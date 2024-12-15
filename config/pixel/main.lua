local ui = require("config/pixel/libs/ui")

-- require("config/pixel/audio/main")
zic("LOAD_CONFIG", "config/pixel/audio/main.cfg plugins/config/build/x86/libzic_DustConfig.so")

ui.setWindowPosition(400, 500)

ScreenWidth = 320
ScreenHeight = 240
ui.setScreenSize(ScreenWidth, ScreenHeight)

ui.view("Main")

local text = require("config/pixel/libs/component/text")
text(
    { TEXT = "Hello world" },
    { x = 0, y = 0, w = ScreenWidth, h = 8 },
    { CENTERED = true, FONT_SIZE = 24 }
)

local adsr = require("config/pixel/libs/component/adsr")
adsr(
    { PLUGIN = "FM", VALUES = "ATTACK_0 DECAY_0 SUSTAIN_0 RELEASE_0" },
    { x = 0, y = 20, w = ScreenWidth, h = 50 },
    { TRACK = 5, GROUP = 1 }
)

local encoder3 = require("config/pixel/libs/component/encoder3")
encoder3(
    { ENCODER_ID = 0, VALUE = "MMFilter CUTOFF" },
    { x = 0, y = 80, w = 50, h = 50 },
    { TRACK = 2 }
)

local fmAlgo = require("config/pixel/libs/component/fmAlgo")
fmAlgo(
    { ENCODER_ID = 1, VALUE = "FM ALGO" },
    { x = 60, y = 80, w = 50, h = 50 },
    { TRACK = 4 }
)

local graph = require("config/pixel/libs/component/graph")
graph(
    { PLUGIN = "SynthDrum23", DATA_ID = "WAVEFORM" },
    { x = 120, y = 80, w = 200, h = 50 },
    { TRACK = 1, GROUP = 1, ENCODERS = { "0 WAVEFORM_TYPE", "1 SHAPE", "2 MACRO" } }
)

-- COMPONENT: KeyInfoBar 0 216
-- SHIFT_VISIBILITY: 254 0
-- ROW1: &icon::arrowUp Seq Clip Synth Track
-- KEYMAP: Keyboard 'q' incGroup:-1
-- KEYMAP: Keyboard 'w' setView:Sequencer($track)
-- KEYMAP: Keyboard 'e' noteOn:($synth):58
-- KEYMAP: Keyboard 'r' noteOn:($synth):59
-- KEYMAP: Keyboard 't' shift:254:1:0
-- ROW2: &icon::arrowDown &icon::musicNote::pixelated Live &icon::pause &icon::play
-- KEYMAP: Keyboard 'a' incGroup:+1
-- KEYMAP: Keyboard 's' noteOn:($synth):60
-- KEYMAP: Keyboard 'd' noteOn:($synth):61
-- KEYMAP: Keyboard 'f' noteOn:($synth):62
-- KEYMAP: Keyboard 'g' playPause

local keyInfoBar = require("config/pixel/libs/component/keyInfoBar")
keyInfoBar(
    {
        ROW1 = "1 2 3 4 Shift",
        ROW2 = "&icon::arrowDown &icon::musicNote::pixelated Live &icon::pause &icon::play"
    },
    { x = 0, y = 130 },
    {
        -- SHIFT_VISIBILITY = { index = 254, value = 0 }
    }
)
