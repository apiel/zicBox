local ui = require("config/pixel/libs/ui")
local text = require("config/pixel/libs/component/text")
local encoder3 = require("config/pixel/libs/component/encoder3")
local keyInfoBar = require("config/pixel/libs/component/keyInfoBar")
local groupVisibility = require("config/pixel/libs/containers/groupVisibility")

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

                { key = "a", action = ".left" },
                { key = "d", action = ".right" },
            }
        })


    return y + h + 2
end

local function encoders(group, track)
    groupVisibility("DrumSample" .. track, group)

    encoder3(
        { ENCODER_ID = 0, VALUE = "Volume VOLUME" },
        { x = 0, y = 15, w = W1_4, h = 50 },
        { COLOR = "primary", TRACK = track, GROUP = group }
    )
    encoder3(
        { ENCODER_ID = 1, VALUE = "DrumSample START" },
        { x = W1_4, y = 15, w = W1_4, h = 50 },
        { COLOR = "secondary", TRACK = track, GROUP = group }
    )
    encoder3(
        { ENCODER_ID = 2, VALUE = "DrumSample END" },
        { x = W2_4, y = 15, w = W1_4, h = 50 },
        { COLOR = "tertiary", TRACK = track, GROUP = group }
    )
    encoder3(
        { ENCODER_ID = 3, VALUE = "DrumSample BROWSER" },
        { x = W3_4, y = 15, w = W1_4, h = 50 },
        { COLOR = "quaternary", TRACK = track, TYPE = "NUMBER", GROUP = group }
    )
end

local function drumSample(track)
    text(
        { TEXT = "Drum Sample(1)" },
        { x = 0, y = 0, w = ScreenWidth, h = 8 },
        { CENTERED = true }
    )


    local keyInfoPosition = { x = 0, y = 216 }
    keyInfoBar(
        {
            ROW1 =
            "&icon::musicNote::pixelated &icon::arrowUp::filled &icon::musicNote::pixelated &icon::musicNote::pixelated Track",
            ROW2 =
            "&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled &icon::musicNote::pixelated &icon::play::filled"
        }, keyInfoPosition,
        {
            SHIFT_VISIBILITY = { index = 254, value = 0 },
            KEYMAPS = {
                { key = "w", action = "incGroup:-1" },
                { key = "e", action = "noteOn:DrumSample:58" },
                { key = "r", action = "noteOn:DrumSample:59" },
                { key = "t", action = "shift:254:1:0" },

                { key = "s", action = "incGroup:+1" },

                { key = "f", action = "noteOn:DrumSample:62" },
                { key = "g", action = "playPause" }
            }
        }
    )

    keyInfoBar(
        {
            ROW1 = "1 2 3 4 Track",
            ROW2 = "5 6 7 8 Master"
        }, keyInfoPosition,
        {
            SHIFT_VISIBILITY = { index = 254, value = 1 },
            KEYMAPS = {
                { key = "q", action = track == 1 and "setView:Sequencer1" or "setView:Track1" },
                { key = "w", action = track == 2 and "setView:Sequencer2" or "setView:Track2" },
                { key = "e", action = track == 3 and "setView:Sequencer3" or "setView:Track3" },
                { key = "r", action = track == 4 and "setView:Sequencer4" or "setView:Track4" },
                { key = "t", action = "shift:254:1:0" },
                { key = "a", action = track == 5 and "setView:Sequencer5" or "setView:Track5" },
                { key = "s", action = track == 6 and "setView:Sequencer6" or "setView:Track6" },
                { key = "d", action = track == 7 and "setView:Sequencer7" or "setView:Track7" },
                { key = "f", action = track == 8 and "setView:Sequencer8" or "setView:Track8" },
                { key = "g", action = "setView:Master" },
            }
        }
    )

    local y = 70
    y = row(0, 11, y)
    y = row(1, 12, y)
    y = row(2, 13, y)
    y = row(3, 14, y)
    y = row(4, 15, y)
    y = row(5, 16, y)
    y = row(6, 17, y)
    y = row(7, 18, y)

    encoders(0, 11)
    encoders(1, 12)
    encoders(2, 13)
    encoders(3, 14)
    encoders(4, 15)
    encoders(5, 16)
    encoders(6, 17)
    encoders(7, 18)
end

return drumSample
