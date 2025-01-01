local ui = require("config/pixel/libs/ui")
local textGrid = require("config/pixel/libs/component/textGrid")
local value = require("config/pixel/libs/component/value")
local visibility = require("config/pixel/libs/containers/visibility")

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

local function clips(color, track, group, y)
    local pos = { x = (track - 1) * W1_8, y = y, w = W1_8 - 2, h = 200 }

    -- visibility(pos, { VISIBILITY_CONTEXT = { "254 SHOW_WHEN 0" } })
    ui.component("Clips",
        { "PLUGIN" },
        { PLUGIN = "SerializeTrack", },
        pos, {
            TRACK = track,
            GROUP = group,
            COLOR = color,
            KEYMAPS = {
                { key = "w", action = ".up", context = "254:0" },
                { key = "s", action = ".down", context = "254:0" },

                { key = "d", action = ".save", context = "254:1" },
            }
        })

    -- visibility(pos, { VISIBILITY_CONTEXT = { "254 SHOW_WHEN 1" } })
    -- ui.component("Clips",
    --     { "PLUGIN" },
    --     { PLUGIN = "SerializeTrack", },
    --     pos, {
    --         TRACK = track,
    --         GROUP = group,
    --         COLOR = color,
    --         KEYMAPS = {
    --             { key = "d", action = ".save" },
    --         }
    --     })
end

local function view(viewName)
    ui.view(viewName)

    -- clips
    textGrid(
        {
            "&icon::toggle::rect &icon::arrowUp::filled ...",
            "&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled"
        }, KeyInfoPosition,
        -- &icon::toggle --> load&play/stop current slected clip
        {
            VISIBILITY_CONTEXT = { "254 SHOW_WHEN 0" },
            KEYMAPS = {
                { key = "e", action = "contextToggle:254:1:0" },
                { key = "a", action = "incGroup:-1" },
                { key = "d", action = "incGroup:+1" },
            }
        }
    )



    -- clips shifted
    textGrid(
        {
            "Next &icon::play::filled ^...",
            "Seq Master Save"
        }, KeyInfoPosition,
        -- save --> save the current track to this clip
        -- next --> play the clip at the next round (pressing next several time would wait even more rounds...)
        -- seq should it be called main
        -- master should we prioritize synth edit instead of master
        --
        -- how to delete? should it be instead of master/synth an extra action button
        -- or should it be instead of the save... (but i think save is good to be quickly accessible)
        {
            VISIBILITY_CONTEXT = { "254 SHOW_WHEN 1" },
            KEYMAPS = {
                { key = "w", action = "playPause" },
                { key = "e", action = "contextToggle:254:1:0" },
                { key = "a", action = "setView:seq" },
            }
        }
    )

    value("Tempo BPM", { W3_4, KeyInfoPosition.y, W1_4, 22 },
        { SHOW_LABEL_OVER_VALUE = 0, BAR_HEIGHT = 0, VALUE_FONT_SIZE = 16 })

    local y = 5
    y = progressBar(y)

    clips(ColorTrack1, 1, 0, y)
    clips(ColorTrack2, 2, 1, y)
    clips(ColorTrack3, 3, 2, y)
    clips(ColorTrack4, 4, 3, y)
    clips(ColorTrack5, 5, 4, y)
    clips(ColorTrack6, 6, 5, y)
    clips(ColorTrack7, 7, 6, y)
    clips(ColorTrack8, 8, 7, y)
end

return view
