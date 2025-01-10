--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Tape = require("config.libs.components.Tape")
local Tape = ____Tape.Tape
local ____Text = require("config.libs.components.Text")
local Text = ____Text.Text
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____ValueBpm = require("config.pixel.240x320.ui.components.ValueBpm")
local ValueBpm = ____ValueBpm.ValueBpm
local ____TextGrid = require("config.pixel.240x320.ui.tapeView.TextGrid")
local TextGridTape = ____TextGrid.TextGridTape
local TextGridTapeShifted = ____TextGrid.TextGridTapeShifted
function ____exports.TapeView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(Text, {text = "view: " .. name, position = {0, 0, 240, 240}}),
        React.createElement(Tape, {position = {0, 16, 240, 240}, filename = "rec"}),
        React.createElement(ValueBpm, nil),
        React.createElement(TextGridTape, nil),
        React.createElement(TextGridTapeShifted, nil)
    )
end
return ____exports
