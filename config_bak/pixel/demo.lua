local ui = require("config/pixel/libs/ui")

-- require("config/pixel/audio/main")
zic("LOAD_CONFIG", "config/pixel/audio/main.cfg plugins/config/build/x86/libzic_DustConfig.so")

ui.setWindowPosition(400, 500)

ScreenWidth = 320
ScreenHeight = 240
ui.setScreenSize(ScreenWidth, ScreenHeight)

ui.view("Main")

local text = require("config/pixel/libs/component/text")
text("Hello world", { x = 0, y = 0, w = ScreenWidth, h = 8 }, { CENTERED = true, FONT_SIZE = 24 })

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

local graph = require("config/pixel/libs/component/graphEncoder")
graph(
    { PLUGIN = "Drum23", DATA_ID = "WAVEFORM" },
    { x = 120, y = 80, w = 200, h = 50 },
    { TRACK = 1, GROUP = 1, ENCODERS = { "0 WAVEFORM_TYPE", "1 SHAPE", "2 MACRO" } }
)

local keyInfoBar = require("config/pixel/libs/component/keyInfoBar")
keyInfoBar(
    {
        ROW1 = "1 2 3 4 Shift",
        ROW2 = "&icon::arrowDown &icon::musicNote::pixelated Live &icon::pause &icon::play"
    },
    { x = 0, y = 130 },
    {
        KEYMAPS = {
            "Keyboard 'q' incGroup:-1",
            "Keyboard 'w' setView:Sequencer1",
            "Keyboard 'e' noteOn:Drum23:58",
            "Keyboard 'r' noteOn:Drum23:59",
            "Keyboard 't' contextToggle:254:1:0",
            "Keyboard 'a' incGroup:+1",
            "Keyboard 's' noteOn:Drum23:60",
            "Keyboard 'd' noteOn:FM:61",
            "Keyboard 'f' noteOn:FM:62",
            { key = "g", action = "playPause" }
        }
    }
)

local stepEdit = require("config/pixel/libs/component/stepEdit")
stepEdit(
    {
        DATA = { pluginName = "Sequencer", dataId = "GET_STEP", stepIndex = 0 },
        COUNTER_DATA_ID = "STEP_COUNTER"
    },
    { x = 220, y = 180, w = 80, h = 25 },
    {
        GROUP = 0,
        TRACK = 1,
        GROUP_RANGE = { 0, 7 },
        SEQUENCE_DATA_ID = "IS_PLAYING",
        GLOBAL_SHIFT = 254
    }
)

local drumEnvelop = require("config/pixel/libs/component/drumEnvelop")
drumEnvelop(
    { PLUGIN = "Drum23", ENVELOP_DATA_ID = "0" },
    { x = 0, y = 180, w = 200, h = 50 },
    { GROUP = 0, TRACK = 1 }
)
