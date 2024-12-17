local ui = require("config/pixel/libs/ui")
local text = require("config/pixel/libs/component/text")

local function row(group, track, y)
    ui.component("SeqSynthBar",
        { "SEQ_PLUGIN", "SYNTH_PLUGIN" },
        { SEQ_PLUGIN = "Sequencer", SYNTH_PLUGIN = "DrumSample" },
        { x = 0, y = y, w = ScreenWidth, h = 18 }, {
            GROUP = group,
            TRACK = track,
            NAME_COLOR = "#00b300"
        })
    return y + 25
end

local function drumSample(track)
    text(
        { TEXT = "Drum Sample(" .. track .. ")" },
        { x = 0, y = 0, w = ScreenWidth, h = 8 },
        { CENTERED = true }
    )

    local y = 15
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
