--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____GraphEncoder = require("config.libs.components.GraphEncoder")
local GraphEncoder = ____GraphEncoder.GraphEncoder
local ____Common = require("config.pixel.240x320_kick copy.ui.components.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_kick copy.ui.constants")
local BassTrack = ____constants.BassTrack
local ColorTrack2 = ____constants.ColorTrack2
local ____constantsValue = require("config.pixel.240x320_kick copy.ui.constantsValue")
local topValues = ____constantsValue.topValues
local ____TextGridBass = require("config.pixel.240x320_kick copy.ui.Bass.TextGridBass")
local TextGridBass = ____TextGridBass.TextGridBass
function ____exports.BassWaveformView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    local pos = topValues
    pos[2] = pos[2] + pos[4] * 0.5
    return React.createElement(
        View,
        {name = name},
        React.createElement(GraphEncoder, {
            position = pos,
            plugin = "Bass",
            data_id = "WAVEFORM",
            track = BassTrack,
            RENDER_TITLE_ON_TOP = false,
            encoders = {"0 WAVEFORM_TYPE", "2 MACRO", "1 SHAPE"}
        }),
        React.createElement(TextGridBass, {selected = 4, viewName = name}),
        React.createElement(Common, {selected = 3, track = BassTrack, selectedBackground = ColorTrack2})
    )
end
return ____exports
