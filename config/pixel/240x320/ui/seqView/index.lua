--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____SeqProgressBar = require("config.libs.components.SeqProgressBar")
local SeqProgressBar = ____SeqProgressBar.SeqProgressBar
local ____Value = require("config.libs.components.Value")
local Value = ____Value.Value
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____constants = require("config.pixel.240x320.ui.constants")
local KeyInfoPosition = ____constants.KeyInfoPosition
local ScreenWidth = ____constants.ScreenWidth
local W1_4 = ____constants.W1_4
local W3_4 = ____constants.W3_4
local ____TextGrid = require("config.pixel.240x320.ui.seqView.TextGrid")
local TextGridProgressBar = ____TextGrid.TextGridProgressBar
local TextGridProgressBarShifted = ____TextGrid.TextGridProgressBarShifted
function ____exports.SeqView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(TextGridProgressBar, nil),
        React.createElement(TextGridProgressBarShifted, nil),
        React.createElement(Value, {
            value = "Tempo BPM",
            position = {W3_4, KeyInfoPosition[2], W1_4, 22},
            SHOW_LABEL_OVER_VALUE = 0,
            BAR_HEIGHT = 0,
            VALUE_FONT_SIZE = 16
        }),
        React.createElement(SeqProgressBar, {
            position = {0, 160, ScreenWidth, 5},
            seq_plugin = "Sequencer 1",
            track = 0,
            volume_plugin = "MasterVolume VOLUME",
            active_color = "#23a123",
            group = 0
        })
    )
end
return ____exports
