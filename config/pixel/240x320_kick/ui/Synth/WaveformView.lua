--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Encoder3 = require("config.libs.components.Encoder3")
local Encoder3 = ____Encoder3.Encoder3
local ____GraphEncoder = require("config.libs.components.GraphEncoder")
local GraphEncoder = ____GraphEncoder.GraphEncoder
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_kick.ui.components.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local W1_4 = ____constants.W1_4
local W3_4 = ____constants.W3_4
local ____constantsValue = require("config.pixel.240x320_kick.ui.constantsValue")
local encoderH = ____constantsValue.encoderH
local height = ____constantsValue.height
local ____TextGridSynth = require("config.pixel.240x320_kick.ui.Synth.TextGridSynth")
local TextGridSynth = ____TextGridSynth.TextGridSynth
function ____exports.WaveformView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(GraphEncoder, {
            position = {0, 0, W3_4 - 2, height},
            plugin = "Drum23",
            data_id = "WAVEFORM",
            RENDER_TITLE_ON_TOP = false,
            encoders = {"0 WAVEFORM_TYPE", "2 MACRO", "1 SHAPE"}
        }),
        React.createElement(Encoder3, {position = {W3_4, (height - encoderH) * 0.5, W1_4, encoderH}, value = "Drum23 DURATION", encoder_id = 3, color = "quaternary"}),
        React.createElement(TextGridSynth, {selected = 1}),
        React.createElement(Common, {selected = 0})
    )
end
return ____exports
