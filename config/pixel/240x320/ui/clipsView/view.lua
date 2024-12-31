local ui = require("config/pixel/libs/ui")
local textGrid = require("config/pixel/libs/component/textGrid")
local value = require("config/pixel/libs/component/value")

local function progressBar(y)
    local progressH = 5
    ui.component("SeqProgressBar",
        { "SEQ_PLUGIN" },
        { SEQ_PLUGIN = "Sequencer 1", },
        { x = 0, y = y, w = ScreenWidth, h = progressH }, {
            TRACK = 1,
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

local function clips(track, x, y)
    ui.component("Clips",
        { "PLUGIN" },
        { PLUGIN = "SerializeTrack", },
        { x = x, y = y, w = W1_8 - 2, h = 200 }, {
            TRACK = track,
        })
end

local function view(viewName)
    ui.view(viewName)

    local y = 0
    y = progressBar(y)

    y = y + 5

    clips(1, 0, y)
    clips(2, W1_8, y)
    clips(3, W2_8, y)
    clips(4, W3_8, y)
    clips(5, W4_8, y)
    clips(6, W5_8, y)
    clips(7, W6_8, y)
    clips(8, W7_8, y)

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
