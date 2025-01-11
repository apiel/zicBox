--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____GraphEncoder = require("config.libs.components.GraphEncoder")
local GraphEncoder = ____GraphEncoder.GraphEncoder
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local ScreenWidth = ____constants.ScreenWidth
local ____constantsValue = require("config.pixel.240x320_kick.ui.constantsValue")
local height = ____constantsValue.height
local ____TextGridCommon = require("config.pixel.240x320_kick.ui.TextGridCommon")
local TextGridCommon = ____TextGridCommon.TextGridCommon
local ____TextGridPage1 = require("config.pixel.240x320_kick.ui.Page1View.TextGridPage1")
local TextGridPage1 = ____TextGridPage1.TextGridPage1
function ____exports.WaveformView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(GraphEncoder, {
            position = {0, 0, ScreenWidth, height},
            plugin = "Drum23",
            data_id = "WAVEFORM",
            RENDER_TITLE_ON_TOP = false,
            encoders = {"0 WAVEFORM_TYPE", "2 MACRO", "1 SHAPE"}
        }),
        React.createElement(TextGridPage1, nil),
        React.createElement(TextGridCommon, {selected = 0})
    )
end
return ____exports
