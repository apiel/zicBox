local ui = require("config/pixel/libs/ui")
local text = require("config/pixel/libs/component/text")
local encoder3 = require("config/pixel/libs/component/encoder3")
local textGrid = require("config/pixel/libs/component/textGrid")
local visibility = require("config/pixel/libs/containers/visibility")
local value = require("config/pixel/libs/component/value")

local function row(group, track, y)
    local h = 15
    ui.component("SeqSynthBar",
        { "SEQ_PLUGIN" },
        { SEQ_PLUGIN = "Sequencer", },
        { x = 0, y = y, w = ScreenWidth, h = h }, {
            GROUP = group,
            TRACK = track,
            NAME_COLOR = "#00b300",
            NAME_PLUGIN = "DrumSample BROWSER",
            VOLUME_PLUGIN = "Volume VOLUME",
            KEYMAPS = {
                { key = "q", action = "noteOn:DrumSample:57" },
                -- { key = "e", action = ".stepToggle" },

                { key = "a", action = ".left" },
                { key = "d", action = ".right" },
            }
        })


    return y + h + 2
end

local function progressBar(group, track, y)
    local h = 5
    ui.component("SeqProgressBar",
        { "SEQ_PLUGIN" },
        { SEQ_PLUGIN = "Sequencer 11", },
        { x = 0, y = y, w = ScreenWidth, h = h }, {
            GROUP = group,
            TRACK = track,
            ACTIVE_COLOR = "#00b300",
            VOLUME_PLUGIN = "VolumeDrive VOLUME",
            KEYMAPS = {
                { key = "q", action = "noteOn:DrumSample:57:11" },
                -- { key = "e", action = ".stepToggle" },

                -- { key = "a", action = ".left" },
                -- { key = "d", action = ".right" },
            }
        })

    return y + h + 4
end

local function encoders(group, track, y)
    visibility(
        "DrumSample" .. track,
        { x = 0, y = y, w = ScreenWidth, h = 50 },
        { VISIBILITY_GROUP = group, VISIBILITY_CONTEXT = "10 SHOW 0" }
    )

    encoder3(
        { ENCODER_ID = 0, VALUE = "Volume VOLUME" },
        { x = 0, y = 0, w = W1_4, h = 50 },
        { COLOR = "primary", TRACK = track, GROUP = group }
    )
    encoder3(
        { ENCODER_ID = 1, VALUE = "DrumSample START" },
        { x = W1_4, y = 0, w = W1_4, h = 50 },
        { COLOR = "secondary", TRACK = track, GROUP = group }
    )
    encoder3(
        { ENCODER_ID = 2, VALUE = "DrumSample END" },
        { x = W2_4, y = 0, w = W1_4, h = 50 },
        { COLOR = "tertiary", TRACK = track, GROUP = group }
    )
    encoder3(
        { ENCODER_ID = 3, VALUE = "DrumSample BROWSER" },
        { x = W3_4, y = 0, w = W1_4, h = 50 },
        { COLOR = "quaternary", TRACK = track, TYPE = "NUMBER", GROUP = group }
    )

    visibility(
        "DrumSample" .. track,
        { x = 0, y = y, w = ScreenWidth, h = 50 },
        { VISIBILITY_GROUP = group, VISIBILITY_CONTEXT = "10 HIDE 0" }
    )

    encoder3(
        { ENCODER_ID = 0, VALUE = "Sequencer STEP_VELOCITY" },
        { x = 0, y = 0, w = W1_4, h = 50 },
        { COLOR = "primary", TRACK = track, GROUP = group }
    )
    encoder3(
        { ENCODER_ID = 1, VALUE = "Sequencer STEP_CONDITION" },
        { x = W1_4, y = 0, w = W1_4, h = 50 },
        { COLOR = "secondary", TRACK = track, GROUP = group }
    )
    encoder3(
        { ENCODER_ID = 2, VALUE = "Sequencer STEP_ENABLED" },
        { x = W2_4, y = 0, w = W1_4, h = 50 },
        { COLOR = "tertiary", TRACK = track, GROUP = group }
    )
    -- encoder3(
    --     { ENCODER_ID = 3, VALUE = "Volume VOLUME" },
    --     { x = W3_4, y = 0, w = W1_4, h = 50 },
    --     { COLOR = "quaternary", TRACK = track, GROUP = group }
    -- )
end

local function encodersMaster(group, track, y)
    visibility("DrumSampleMaster", { x = 0, y = y, w = ScreenWidth, h = 50 }, { VISIBILITY_GROUP = group })

    encoder3(
        { ENCODER_ID = 0, VALUE = "VolumeDrive VOLUME" },
        { x = 0, y = 0, w = W1_4, h = 50 },
        { COLOR = "tertiary", TRACK = track, GROUP = group, USE_SECOND_COLOR = 0.5 }
    )
    encoder3(
        { ENCODER_ID = 1, VALUE = "VolumeDrive DRIVE" },
        { x = W1_4, y = 0, w = W1_4, h = 50 },
        { COLOR = "secondary", TRACK = track, GROUP = group, TYPE = "TWO_VALUES" }
    )
    encoder3(
        { ENCODER_ID = 2, VALUE = "MMFilter CUTOFF" },
        { x = W2_4, y = 0, w = W1_4, h = 50 },
        { COLOR = "primary", TRACK = track, GROUP = group, TYPE = "TWO_SIDED" }
    )
    encoder3(
        { ENCODER_ID = 3, VALUE = "MMFilter RESONANCE" },
        { x = W3_4, y = 0, w = W1_4, h = 50 },
        { COLOR = "primary", TRACK = track, GROUP = group }
    )
end

local function drumSample(track)
    text(
        { TEXT = "Drum Sample(1)" },
        { x = 0, y = 0, w = ScreenWidth, h = 8 },
        { CENTERED = true }
    )

    textGrid(
        {
            "&icon::musicNote::pixelated &icon::arrowUp::filled ...",
            "&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled"
        }, KeyInfoPosition,
        {
            SHIFT_VISIBILITY = { index = 254, value = 0 },
            KEYMAPS = {
                { key = "w", action = "incGroup:-1" },
                { key = "e", action = "shift:254:1:0" },

                { key = "s", action = "incGroup:+1" },

                -- { key = "f", action = "noteOn:DrumSample:62" },
                -- { key = "g", action = "playPause" }
            }
        }
    )

    textGrid(
        {
            "&icon::play::filled &icon::toggle::rect ^...",
            "Menu Sub Master"
        }, KeyInfoPosition,
        {
            SHIFT_VISIBILITY = { index = 254, value = 1 },
            KEYMAPS = {
                { key = "q", action = "playPause" },
                -- { key = "w", action = "setView:Track2" }, // stepToggle
                { key = "e", action = "shift:254:1:0" },
                { key = "a", action = "setView:menu" },
                { key = "s", action = "setView:sub" .. track },
                { key = "d", action = "setView:Master" },
            }
        }
    )

    value({}, { x = W2_4, y = KeyInfoPosition.y, w = W1_4 - 2, h = 11 }, { VALUE = "VolumeDrive VOLUME" })
    value({}, { x = W3_4, y = KeyInfoPosition.y, w = W1_4 - 2, h = 11 }, { VALUE = "MMFilter RESONANCE" })

    local y = 0

    y = 70
    y = progressBar(0, track, y)
    y = row(1, 11, y)
    y = row(2, 12, y)
    y = row(3, 13, y)
    -- y = row(4, 14, y)
    -- y = row(5, 15, y)
    -- y = row(6, 16, y)
    -- y = row(7, 17, y)
    -- y = row(8, 18, y)

    y = 15
    encodersMaster(0, track, y)
    encoders(1, 11, y)
    encoders(2, 12, y)
    encoders(3, 13, y)
    -- encoders(4, 14, y)
    -- encoders(5, 15, y)
    -- encoders(6, 16, y)
    -- encoders(7, 17, y)
    -- encoders(8, 18, y)
end

return drumSample
