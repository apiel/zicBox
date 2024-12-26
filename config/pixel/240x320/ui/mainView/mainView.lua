local ui = require("config/pixel/libs/ui")
local textGrid = require("config/pixel/libs/component/textGrid")
local values = require("config/pixel/240x320/ui/mainView/values")

local function row(group, track, y, items)
    local rowH = 14
    ui.component("SeqSynthBar",
        { "SEQ_PLUGIN" },
        { SEQ_PLUGIN = "Sequencer", },
        { x = 0, y = y, w = ScreenWidth, h = rowH }, {
            GROUP = group,
            TRACK = track,
            NAME_COLOR = "#23a123",
            -- NAME_PLUGIN = "DrumSample BROWSER",
            NAME = track,
            ITEMS = items,
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

local function mainView()
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
                { key = "s", action = "setView:sub" },
                { key = "d", action = "setView:Master" },
            }
        }
    )

    local y = 0

    -- y = 30
    y = 60
    y = progressBar(0, 1, y)
    y = row(1, 11, y, { "Main", "FX", "Amp.", "Freq.", "Wave", "Clic" })
    y = row(2, 12, y, { "Main", "FX", "Amp.", "Freq.", "Wave", "Clic" })
    y = row(3, 13, y, {})
    y = row(4, 14, y, {})
    y = row(5, 15, y, {})
    y = row(6, 16, y, {})
    y = row(7, 17, y, { "Main", "FX", "Op1", ".", "Op2", ".", "Op3", ".", "Op4", "." })
    y = row(8, 18, y, { "Main", "FX", "Op1", ".", "Op2", ".", "Op3", ".", "Op4", "." })

    y = 0
    values.master(0, 1, y)
    values.main(1, 11, y)
    values.main(2, 12, y)
    values.main(3, 13, y)
    values.main(4, 14, y)
    values.main(5, 15, y)
    values.main(6, 16, y)
    values.main(7, 17, y)
    values.main(8, 18, y)
end

return mainView
