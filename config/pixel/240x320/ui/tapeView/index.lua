--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Keymap = require("config.libs.components.Keymap")
local Keymap = ____Keymap.Keymap
local ____Tape = require("config.libs.components.Tape")
local Tape = ____Tape.Tape
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____ValueBpm = require("config.pixel.240x320.ui.components.ValueBpm")
local ValueBpm = ____ValueBpm.ValueBpm
local ____TextGrid = require("config.pixel.240x320.ui.tapeView.TextGrid")
local TextGridTape = ____TextGrid.TextGridTape
function ____exports.TapeView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(
            Tape,
            {position = {0, 0, 240, 280}, filename = "rec", bpm_value = "Tempo BPM", tape_plugin = "Tape PLAY_STOP"},
            React.createElement(Keymap, {key = "q", action = ".playStop"}),
            React.createElement(Keymap, {key = "a", action = ".left"}),
            React.createElement(Keymap, {key = "d", action = ".right"})
        ),
        React.createElement(ValueBpm, nil),
        React.createElement(TextGridTape, nil)
    )
end
return ____exports
