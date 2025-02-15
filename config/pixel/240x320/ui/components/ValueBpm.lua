--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Value = require("config.libs.nativeComponents.Value")
local Value = ____Value.Value
local ____constants = require("config.pixel.240x320.ui.constants")
local KeyInfoPosition = ____constants.KeyInfoPosition
local W1_4 = ____constants.W1_4
local W3_4 = ____constants.W3_4
function ____exports.ValueBpm()
    return React.createElement(Value, {
        value = "Tempo BPM",
        bounds = {W3_4, KeyInfoPosition[2] - 2, W1_4, 24},
        SHOW_LABEL_OVER_VALUE = 0,
        LABEL_OVER_VALUE_X = 16,
        LABEL_FONT_SIZE = 8,
        BAR_HEIGHT = 0,
        VALUE_FONT_SIZE = 16
    })
end
return ____exports
