--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Keymaps = require("config.libs.components.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____SeqSynthBar = require("config.libs.components.SeqSynthBar")
local SeqSynthBar = ____SeqSynthBar.SeqSynthBar
local ____constants = require("config.pixel.240x320.ui.constants")
local ScreenWidth = ____constants.ScreenWidth
____exports.RowH = 14
local function RowKeymaps(____bindingPattern0)
    local synth
    synth = ____bindingPattern0.synth
    return React.createElement(Keymaps, {keys = {{key = "q", action = ".toggleOrPlayNote:" .. synth, context = "254:0"}, {key = "a", action = ".left", context = "254:0"}, {key = "d", action = ".right", context = "254:0"}, {key = "q", action = ".toggle", context = "254:1"}}})
end
function ____exports.rowDrum23(____bindingPattern0)
    local select_menu_context
    local name_color
    local track
    local group
    local y
    y = ____bindingPattern0.y
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    name_color = ____bindingPattern0.name_color
    select_menu_context = ____bindingPattern0.select_menu_context
    return React.createElement(
        SeqSynthBar,
        {
            position = {0, y, ScreenWidth, ____exports.RowH},
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
function ____exports.rowDrumSample(____bindingPattern0)
    local select_menu_context
    local name_color
    local track
    local group
    local y
    y = ____bindingPattern0.y
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    name_color = ____bindingPattern0.name_color
    select_menu_context = ____bindingPattern0.select_menu_context
    return React.createElement(
        SeqSynthBar,
        {
            position = {0, y, ScreenWidth, ____exports.RowH},
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
