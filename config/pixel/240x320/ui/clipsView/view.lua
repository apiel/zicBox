local ui = require("config/pixel/libs/ui")
local textGrid = require("config/pixel/libs/component/textGrid")
local value = require("config/pixel/libs/component/value")

local function progressBar(group, track, y)
    local progressH = 5
    ui.component("SeqProgressBar",
        { "SEQ_PLUGIN" },
        { SEQ_PLUGIN = "Sequencer 1", },
        { x = 0, y = y, w = ScreenWidth, h = progressH }, {
            TRACK = track,
            ACTIVE_COLOR = "#23a123",
            -- VOLUME_PLUGIN = "MasterVolume VOLUME",
            KEYMAPS = {
                -- { key = "q", action = "noteOn:Drum23:60:1" },
                -- { key = "e", action = ".toggle" },

                -- { key = "a", action = ".left" },
                -- { key = "d", action = ".right" },
            }
        })

    return y + 4 + progressH
end

local function view(viewName)
    ui.view(viewName)

    progressBar(0, 0, 0)

    -- clips
    textGrid(
        {
            "&icon::toggle::rect &icon::arrowUp::filled ...",
            "&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled"
        }, KeyInfoPosition,
        {
            VISIBILITY_CONTEXT = { "254 SHOW_WHEN 0" },
            KEYMAPS = {
                { key = "e", action = "shift:254:1:0" },
            }
        }
    )

    -- clips shifted
    textGrid(
        {
            "Menu &icon::play::filled ^...",
            "Seq ? Master"
        }, KeyInfoPosition,
        {
            VISIBILITY_CONTEXT = { "254 SHOW_WHEN 1" },
            KEYMAPS = {
                { key = "w", action = "playPause" },
                { key = "e", action = "shift:254:1:0" },
                { key = "a", action = "setView:seq" },
            }
        }
    )

    value("Tempo BPM", { W3_4, KeyInfoPosition.y, W1_4, 22 },
        { SHOW_LABEL_OVER_VALUE = 0, BAR_HEIGHT = 0, VALUE_FONT_SIZE = 16 })

end

return view
