local ui = require("config/pixel/libs/ui")
local encoder3 = require("config/pixel/libs/component/encoder3")
local textGrid = require("config/pixel/libs/component/textGrid")
local visibility = require("config/pixel/libs/containers/visibility")
local value = require("config/pixel/libs/component/value")

local function row(group, track, y)
    local rowH = 14
    ui.component("SeqBar",
        { "SEQ_PLUGIN" },
        { SEQ_PLUGIN = "Sequencer", },
        { x = 0, y = y, w = ScreenWidth, h = rowH }, {
            GROUP = group,
            TRACK = track,
            NAME_COLOR = "#23a123",
            -- NAME_PLUGIN = "DrumSample BROWSER",
            NAME = track,
            VOLUME_PLUGIN = "Volume VOLUME",
            KEYMAPS = {
                { key = "q", action = "noteOn:DrumSample:57" },
                -- { key = "e", action = ".stepToggle" },

                { key = "a", action = ".left" },
                { key = "d", action = ".right" },
            }
        })


    return y + 1 + rowH
end

local function row2(group, track, y)
    local rowH = 10
    ui.component("SynthBar",
        {  },
        {  },
        { x = 0, y = y + 1, w = ScreenWidth, h = rowH }, {
            GROUP = group,
            TRACK = track,
            KEYMAPS = {
            }
        })


    return y + 1 + 2 + rowH
end

local function progressBar(group, track, y)
    local progressH = 5
    ui.component("SeqProgressBar",
        { "SEQ_PLUGIN" },
        { SEQ_PLUGIN = "Sequencer 11", },
        { x = 0, y = y, w = ScreenWidth, h = progressH }, {
            GROUP = group,
            TRACK = track,
            ACTIVE_COLOR = "#23a123",
            VOLUME_PLUGIN = "VolumeDrive VOLUME",
            KEYMAPS = {
                { key = "q", action = "noteOn:DrumSample:57:11" },
                -- { key = "e", action = ".stepToggle" },

                -- { key = "a", action = ".left" },
                -- { key = "d", action = ".right" },
            }
        })

    return y + 4 + progressH
end

local function encoders(group, track, y)
    visibility(
        "DrumSample" .. track,
        { x = 0, y = y, w = ScreenWidth, h = 24 },
        { VISIBILITY_GROUP = group, VISIBILITY_CONTEXT = "10 SHOW 0" }
    )

    value("Volume VOLUME", { x = 0, y = 0, w = W2_4 - 2, h = 11 },
        { LABEL_COLOR = "tertiary", BAR_COLOR = TertiaryBar, ENCODER_ID = 0, TRACK = track, GROUP = group })
    value("DrumSample START", { x = 0, y = 12, w = W2_4 - 2, h = 11 },
        { LABEL_COLOR = "primary", BAR_COLOR = PrimaryBar, ENCODER_ID = 1, TRACK = track, GROUP = group })
    value("DrumSample BROWSER", { x = W2_4, y = 0, w = W2_4 - 2, h = 11 },
        { LABEL_COLOR = "secondary", BAR_COLOR = SecondaryBar, ENCODER_ID = 2, TRACK = track, GROUP = group, SHOW_LABEL = false, })
    value("DrumSample END", { x = W2_4, y = 12, w = W2_4 - 2, h = 11 },
        { LABEL_COLOR = "quaternary", BAR_COLOR = QuaternaryBar, ENCODER_ID = 3, TRACK = track, GROUP = group })

    visibility(
        "DrumSample" .. track,
        { x = 0, y = y, w = ScreenWidth, h = 24 },
        { VISIBILITY_GROUP = group, VISIBILITY_CONTEXT = "10 HIDE 0" }
    )

    value("Sequencer STEP_VELOCITY", { x = 0, y = 0, w = W2_4 - 2, h = 11 },
        { LABEL_COLOR = "tertiary", BAR_COLOR = TertiaryBar, ENCODER_ID = 0, TRACK = track, GROUP = group })
    value("Sequencer STEP_CONDITION", { x = 0, y = 12, w = W2_4 - 2, h = 11 },
        { LABEL_COLOR = "primary", BAR_COLOR = PrimaryBar, ENCODER_ID = 1, TRACK = track, GROUP = group })
    value("Sequencer STEP_ENABLED", { x = W2_4, y = 0, w = W2_4 - 2, h = 11 },
        { LABEL_COLOR = "secondary", BAR_COLOR = SecondaryBar, ENCODER_ID = 2, TRACK = track, GROUP = group })
    -- value("Volume VOLUME", { x = W2_4, y = 12, w = W2_4 - 2, h = 11 },
    --     { LABEL_COLOR = "quaternary", BAR_COLOR = QuaternaryBar, ENCODER_ID = 3, TRACK = track, GROUP = group })
end

local function encodersMaster(group, track, y)
    visibility("DrumSampleMaster", { x = 0, y = y, w = ScreenWidth, h = 50 }, { VISIBILITY_GROUP = group })

    value("VolumeDrive VOLUME", { x = 0, y = 0, w = W2_4 - 2, h = 11 },
        { LABEL_COLOR = "tertiary", BAR_COLOR = TertiaryBar, ENCODER_ID = 0, TRACK = track, GROUP = group })
    value("VolumeDrive DRIVE", { x = 0, y = 12, w = W2_4 - 2, h = 11 },
        { LABEL_COLOR = "primary", BAR_COLOR = PrimaryBar, ENCODER_ID = 1, TRACK = track, GROUP = group })
    value("MMFilter CUTOFF", { x = W2_4, y = 0, w = W2_4 - 2, h = 11 },
        {
            LABEL_COLOR = "secondary",
            BAR_COLOR = SecondaryBar,
            USE_STRING_VALUE = true,
            SHOW_LABEL = false,
            ENCODER_ID = 2,
            TRACK = track,
            GROUP = group
        })
    value("MMFilter RESONANCE", { x = W2_4, y = 12, w = W2_4 - 2, h = 11 },
        { LABEL_COLOR = "quaternary", BAR_COLOR = QuaternaryBar, ENCODER_ID = 3, TRACK = track, GROUP = group })
end

local function drumSample(track)
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

    local y = 0

    y = 30
    y = progressBar(0, track, y)
    y = row2(1, 11, y)
    y = row(2, 12, y)
    y = row(3, 13, y)
    y = row(4, 14, y)
    y = row(5, 15, y)
    y = row(6, 16, y)
    y = row(7, 17, y)
    y = row(8, 18, y)

    y = 0
    encodersMaster(0, track, y)
    encoders(1, 11, y)
    encoders(2, 12, y)
    encoders(3, 13, y)
    encoders(4, 14, y)
    encoders(5, 15, y)
    encoders(6, 16, y)
    encoders(7, 17, y)
    encoders(8, 18, y)
end

return drumSample
