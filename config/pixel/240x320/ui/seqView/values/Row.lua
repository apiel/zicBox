--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Keymaps = require("config.libs.components.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____SeqSynthBar = require("config.libs.components.SeqSynthBar")
local SeqSynthBar = ____SeqSynthBar.SeqSynthBar
local ____constants = require("config.pixel.240x320.ui.seqView.constants")
local ScreenWidth = ____constants.ScreenWidth
local y = 170
____exports.RowH = 14
local function RowKeymaps(____bindingPattern0)
    local synth
    synth = ____bindingPattern0.synth
    return React.createElement(Keymaps, {keys = {{key = "q", action = ".toggleOrPlayNote:" .. synth, context = "254:0"}, {key = "a", action = ".left", context = "254:0"}, {key = "d", action = ".right", context = "254:0"}, {key = "q", action = ".toggle", context = "254:1"}}})
end
local function getY(track)
    return y + (track - 1) * (____exports.RowH + 1)
end
function ____exports.RowDrum23(____bindingPattern0)
    local select_menu_context
    local name_color
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    name_color = ____bindingPattern0.name_color
    select_menu_context = ____bindingPattern0.select_menu_context
    return React.createElement(
        SeqSynthBar,
        {
            position = {
                0,
                getY(track),
                ScreenWidth,
                ____exports.RowH
            },
            seq_plugin = "Sequencer",
            track = track,
            group = group,
            name_color = name_color,
            name = track,
            items = "Click Freq. Amp. Wave FX",
            volume_plugin = "Volume VOLUME",
            select_menu_context = select_menu_context
        },
        React.createElement(RowKeymaps, {synth = "Drum23"})
    )
end
function ____exports.RowDrumSample(____bindingPattern0)
    local select_menu_context
    local name_color
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    name_color = ____bindingPattern0.name_color
    select_menu_context = ____bindingPattern0.select_menu_context
    return React.createElement(
        SeqSynthBar,
        {
            position = {
                0,
                getY(track),
                ScreenWidth,
                ____exports.RowH
            },
            seq_plugin = "Sequencer",
            track = track,
            group = group,
            name_color = name_color,
            name = track,
            items = "Sample",
            volume_plugin = "Volume VOLUME",
            select_menu_context = select_menu_context
        },
        React.createElement(RowKeymaps, {synth = "DrumSample"})
    )
end
return ____exports
