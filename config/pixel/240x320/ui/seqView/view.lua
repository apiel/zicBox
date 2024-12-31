local ui = require("config/pixel/libs/ui")
local textGrid = require("config/pixel/libs/component/textGrid")
local values = require("config/pixel/240x320/ui/seqView/values")
local visibility = require("config/pixel/libs/containers/visibility")
local value = require("config/pixel/libs/component/value")

local function _row(color, group, track, menuContext, items, rowH, keymaps)
    ui.component("SeqSynthBar",
        { "SEQ_PLUGIN" },
        { SEQ_PLUGIN = "Sequencer" },
        { x = 0, y = 0, w = ScreenWidth, h = rowH }, {
            GROUP = group,
            TRACK = track,
            NAME_COLOR = color,
            NAME = track,
            ITEMS = items,
            VOLUME_PLUGIN = "Volume VOLUME",
            KEYMAPS = keymaps,
            SELECT_MENU_CONTEXT = menuContext,
        })
end

local function row(color, group, track, y, menuContext, synth, items)
    local rowH = 14

    visibility({ x = 0, y = y, w = ScreenWidth, h = rowH }, { VISIBILITY_CONTEXT = { "254 SHOW_WHEN 0" } })
    _row(color, group, track, menuContext, items, rowH, {
        { key = "q", action = ".toggleOrPlayNote:" .. synth },
        { key = "a", action = ".left" },
        { key = "d", action = ".right" },
    })

    visibility({ x = 0, y = y, w = ScreenWidth, h = rowH }, { VISIBILITY_CONTEXT = { "254 SHOW_WHEN 1" } })
    _row(color, group, track, menuContext, items, rowH, {
        { key = "q", action = ".toggle" },
        { key = "a", action = "setView:clips" },
    })

    return y + 1 + rowH
end

local function rowDrum23(color, group, track, y, menuContext)
    return row(color, group, track, y, menuContext, "Drum23", { "Click", "Freq.", "Amp.", "Wave", "FX" })
end

local function rowDrumSample(color, group, track, y, menuContext)
    return row(color, group, track, y, menuContext, "DrumSample", { "Sample" })
end

local function progressBar(group, track, y)
    local progressH = 5
    ui.component("SeqProgressBar",
        { "SEQ_PLUGIN" },
        { SEQ_PLUGIN = "Sequencer 1", },
        { x = 0, y = y, w = ScreenWidth, h = progressH }, {
            GROUP = group,
            TRACK = track,
            ACTIVE_COLOR = "#23a123",
            VOLUME_PLUGIN = "MasterVolume VOLUME",
            KEYMAPS = {
                { key = "q", action = "noteOn:Drum23:60:1" },
                -- { key = "e", action = ".toggle" },

                -- { key = "a", action = ".left" },
                -- { key = "d", action = ".right" },
            }
        })

    return y + 4 + progressH
end

local function view(viewName)
    ui.view(viewName)

    -- track
    textGrid(
        {
            "&icon::musicNote::pixelated &icon::arrowUp::filled ...",
            "&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled"
        }, KeyInfoPosition,
        {
            VISIBILITY_CONTEXT = { "254 SHOW_WHEN 0", "10 SHOW_WHEN 0" },
            KEYMAPS = {
                { key = "w", action = "incGroup:-1" },
                { key = "e", action = "shift:254:1:0" },

                { key = "s", action = "incGroup:+1" },

                -- { key = "f", action = "noteOn:DrumSample:62" },
                -- { key = "g", action = "playPause" }
            }
        }
    )

    -- track step edit
    textGrid(
        {
            "&icon::toggle::rect &icon::arrowUp::filled ...",
            "&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled"
        }, KeyInfoPosition,
        {
            VISIBILITY_CONTEXT = { "254 SHOW_WHEN 0", "10 SHOW_WHEN_OVER 0" },
            KEYMAPS = {
                { key = "w", action = "incGroup:-1" },
                { key = "e", action = "shift:254:1:0" },

                { key = "s", action = "incGroup:+1" },

                -- { key = "f", action = "noteOn:DrumSample:62" },
                -- { key = "g", action = "playPause" }
            }
        }
    )

    -- track shifted
    textGrid(
        {
            "&icon::toggle::rect &icon::play::filled ^...",
            "Clips Edit Master"
        }, KeyInfoPosition,
        {
            VISIBILITY_GROUP = { "SHOW_WHEN_NOT 0" },
            VISIBILITY_CONTEXT = { "254 SHOW_WHEN 1" },
            KEYMAPS = {
                -- { key = "q", action = "playPause" },
                { key = "w", action = "playPause" }, -- stepToggle
                { key = "e", action = "shift:254:1:0" },
                -- { key = "a", action = "setView:menu" },
                { key = "s", action = "setView:sub" },
                { key = "d", action = "setView:Master" },
            }
        }
    )

    -- progressbar shifted
    textGrid(
        {
            "Menu &icon::play::filled ^...",
            "Clips ? Save"
        }, KeyInfoPosition,
        {
            VISIBILITY_GROUP = { "SHOW_WHEN 0" },
            VISIBILITY_CONTEXT = { "254 SHOW_WHEN 1" },
            KEYMAPS = {
                { key = "a", action = "setView:clips" },
            }
        }
    )

    value("Tempo BPM", { W3_4, KeyInfoPosition.y, W1_4, 22 },
        { SHOW_LABEL_OVER_VALUE = 0, BAR_HEIGHT = 0, VALUE_FONT_SIZE = 16 })

    ui.component("Spectrogram",
        {},
        {},
        { x = 0, y = 4, w = ScreenWidth, h = 70 }, {
            TRACK = 0,
            -- BACKGROUND_COLOR = "#2c323a",
            DATA = "Spectrogram BUFFER",
            TEXT = "Pixel"
        })

    -- text("Pixel", { x = 0, y = 20, w = ScreenWidth, h = 16 }, { CENTERED = true, FONT_SIZE = 16 })

    local y = 0

    y = 160
    y = progressBar(0, 0, y)
    y = rowDrum23(ColorTrack1, 1, 1, y, 11)
    y = rowDrum23(ColorTrack2, 2, 2, y, 12)
    y = rowDrumSample(ColorTrack3, 3, 3, y, 13)
    y = rowDrumSample(ColorTrack4, 4, 4, y, 14)
    y = rowDrumSample(ColorTrack5, 5, 5, y, 15)
    y = rowDrumSample(ColorTrack6, 6, 6, y, 16)
    y = rowDrumSample(ColorTrack7, 7, 7, y, 17)
    y = rowDrumSample(ColorTrack8, 8, 8, y, 18)

    -- "FX", "Op1", ".", "Op2", ".", "Op3", ".", "Op4", "."

    y = 84
    values.master(0, 0, y)
    values.drum23(1, 1, y, 11)
    values.drum23(2, 2, y, 12)
    values.drumSample(3, 3, y, 13)
    values.drumSample(4, 4, y, 14)
    values.drumSample(5, 5, y, 15)
    values.drumSample(6, 6, y, 16)
    values.drumSample(7, 7, y, 17)
    values.drumSample(8, 8, y, 18)
end

return view
