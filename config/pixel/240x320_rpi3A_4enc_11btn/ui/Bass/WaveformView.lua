--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____View = require("config.libs.nativeComponents.View")
local View = ____View.View
local ____GraphEncoder = require("config.libs.nativeComponents.GraphEncoder")
local GraphEncoder = ____GraphEncoder.GraphEncoder
local ____Common = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.components.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local BassTrack = ____constants.BassTrack
local ColorTrack2 = ____constants.ColorTrack2
local encBottomLeft = ____constants.encBottomLeft
local encTopLeft = ____constants.encTopLeft
local encTopRight = ____constants.encTopRight
local ____constantsValue = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constantsValue")
local topValues = ____constantsValue.topValues
local ____TextGridBass = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Bass.TextGridBass")
local TextGridBass = ____TextGridBass.TextGridBass
function ____exports.BassWaveformView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    local pos = {table.unpack(topValues)}
    pos[2] = pos[2] + pos[4] * 0.5
    return React.createElement(
        View,
        {name = name},
        React.createElement(GraphEncoder, {
            bounds = pos,
            audioPlugin = "Bass",
            dataId = "WAVEFORM",
            track = BassTrack,
            renderValuesOnTop = false,
            encoders = {{encoderId = encTopLeft, value = "WAVEFORM_TYPE"}, {encoderId = encTopRight, value = "SHAPE"}, {encoderId = encBottomLeft, value = "MACRO"}}
        }),
        React.createElement(TextGridBass, {selected = "Wave", viewName = name}),
        React.createElement(Common, {selected = "Bass", track = BassTrack, selectedBackground = ColorTrack2})
    )
end
return ____exports
