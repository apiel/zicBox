local ui = require("config/pixel/libs/ui")
local text = require("config/pixel/libs/component/text")

local function drumSample(track)
    text(
        { TEXT = "Drum Sample(" .. track .. ")" },
        { x = 0, y = 0, w = ScreenWidth, h = 8 },
        { CENTERED = true }
    )

    ui.component("SeqSynthBar", { "PLUGIN" }, { PLUGIN = "Sequencer" }, { x = 0, y = 10, w = ScreenWidth, h = 20 }, {
        TRACK = track
    })
end

return drumSample
