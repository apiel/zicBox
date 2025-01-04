--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____SeqProgressBar = require("config.libs.components.SeqProgressBar")
local SeqProgressBar = ____SeqProgressBar.SeqProgressBar
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____constants = require("config.pixel.240x320.ui.constants")
local ScreenWidth = ____constants.ScreenWidth
function ____exports.ClipsView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(SeqProgressBar, {position = {0, 5, ScreenWidth, 5}, seq_plugin = "Sequencer 1", track = 1, active_color = "#23a123"}),
        React.createElement(Value, {
            value = "Tempo BPM",
            position = {W3_4, KeyInfoPosition[1], W1_4, 22},
            SHOW_LABEL_OVER_VALUE = 0,
            BAR_HEIGHT = 0,
            VALUE_FONT_SIZE = 16
        })
    )
end
return ____exports
