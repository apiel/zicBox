local ui = require("config/pixel/libs/ui")
local text = require("config/pixel/libs/component/text")
local encoder3 = require("config/pixel/libs/component/encoder3")
local textGrid = require("config/pixel/libs/component/textGrid")
local visibility = require("config/pixel/libs/containers/visibility")

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
                { key = "e", action = ".toggle" },

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
                -- { key = "e", action = ".toggle" },

                -- { key = "a", action = ".left" },
                -- { key = "d", action = ".right" },
            }
        })

    return y + h + 4
end

local function encoders(group, track, y)
    visibility(
        { x = 0, y = y, w = ScreenWidth, h = 50 },
        { VISIBILITY_GROUP = group, VISIBILITY_CONTEXT = "10 SHOW_WHEN 0" }
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
        { x = 0, y = y, w = ScreenWidth, h = 50 },
        { VISIBILITY_GROUP = group, VISIBILITY_CONTEXT = "10 SHOW_WHEN_OVER 0" }
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
    visibility({ x = 0, y = y, w = ScreenWidth, h = 50 }, { VISIBILITY_GROUP = group })

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
    text("Drum Sample(1)", { x = 0, y = 0, w = ScreenWidth, h = 8 }, { CENTERED = true })


    local keyInfoPosition = { x = 0, y = 216 }
    textGrid(
        {
            "&icon::musicNote::pixelated &icon::arrowUp::filled &icon::toggle::rect &icon::musicNote::pixelated Track",
            "&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled &icon::musicNote::pixelated &icon::play::filled"
        }, keyInfoPosition,
        {
            VISIBILITY_CONTEXT = { "254 SHOW_WHEN 0" },
            KEYMAPS = {
                { key = "w", action = "incGroup:-1" },

                { key = "r", action = "noteOn:DrumSample:59" },
                { key = "t", action = "contextToggle:254:1:0" },

                { key = "s", action = "incGroup:+1" },

                { key = "f", action = "noteOn:DrumSample:62" },
                { key = "g", action = "playPause" }
            }
        }
    )

    textGrid(
        {
            "1 2 3 4 Track",
            "5 6 7 8 Master"
        }, keyInfoPosition,
        {
            VISIBILITY_CONTEXT = { "254 SHOW_WHEN 1" },
            KEYMAPS = {
                { key = "q", action = track == 1 and "setView:Sequencer1" or "setView:Track1" },
                { key = "w", action = track == 2 and "setView:Sequencer2" or "setView:Track2" },
                { key = "e", action = track == 3 and "setView:Sequencer3" or "setView:Track3" },
                { key = "r", action = track == 4 and "setView:Sequencer4" or "setView:Track4" },
                { key = "t", action = "contextToggle:254:1:0" },
                { key = "a", action = track == 5 and "setView:Sequencer5" or "setView:Track5" },
                { key = "s", action = track == 6 and "setView:Sequencer6" or "setView:Track6" },
                { key = "d", action = track == 7 and "setView:Sequencer7" or "setView:Track7" },
                { key = "f", action = track == 8 and "setView:Sequencer8" or "setView:Track8" },
                { key = "g", action = "setView:Master" },
            }
        }
    )

    local y = 0

    y = 70
    y = progressBar(0, track, y)
    y = row(1, 11, y)
    y = row(2, 12, y)
    y = row(3, 13, y)
    y = row(4, 14, y)
    y = row(5, 15, y)
    y = row(6, 16, y)
    y = row(7, 17, y)
    y = row(8, 18, y)

    y = 15
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
