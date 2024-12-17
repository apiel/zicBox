local ui = require("config/pixel/libs/ui")
local text = require("config/pixel/libs/component/text")
local encoder3 = require("config/pixel/libs/component/encoder3")

local function row(group, track, y)
    local h = 15
    ui.component("SeqSynthBar",
        { "SEQ_PLUGIN", "SYNTH_PLUGIN" },
        { SEQ_PLUGIN = "Sequencer", SYNTH_PLUGIN = "DrumSample" },
        { x = 0, y = y, w = ScreenWidth, h = h }, {
            GROUP = group,
            TRACK = track,
            NAME_COLOR = "#00b300"
        })
    return y + h + 2
end

local function drumSample(track)
    text(
        { TEXT = "Drum Sample(" .. track .. ")" },
        { x = 0, y = 0, w = ScreenWidth, h = 8 },
        { CENTERED = true }
    )

    encoder3(
        { ENCODER_ID = 0, VALUE = "Volume VOLUME" },
        { x = 0, y = 15, w = W1_4, h = 50 },
        { COLOR = "primary", TRACK = track }
    )
    encoder3(
        { ENCODER_ID = 0, VALUE = "DrumSample START" },
        { x = W1_4, y = 15, w = W1_4, h = 50 },
        { COLOR = "secondary", TRACK = track }
    )
    encoder3(
        { ENCODER_ID = 0, VALUE = "DrumSample END" },
        { x = W2_4, y = 15, w = W1_4, h = 50 },
        { COLOR = "tertiary", TRACK = track }
    )
    encoder3(
        { ENCODER_ID = 0, VALUE = "DrumSample BROWSER" },
        { x = W3_4, y = 15, w = W1_4, h = 50 },
        { COLOR = "quaternary", TRACK = track, TYPE = "NUMBER" }
    )

    local y = 70
    y = row(0, track, y)
    y = row(1, track, y)
    y = row(2, track, y)
    y = row(3, track, y)
    y = row(4, track, y)
    y = row(5, track, y)
    y = row(6, track, y)
    y = row(7, track, y)
end

return drumSample
