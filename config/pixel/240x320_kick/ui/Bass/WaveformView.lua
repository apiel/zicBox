--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____KnobValue = require("config.libs.components.KnobValue")
local KnobValue = ____KnobValue.KnobValue
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____GraphEncoder = require("config.libs.components.GraphEncoder")
local GraphEncoder = ____GraphEncoder.GraphEncoder
local ____Common = require("config.pixel.240x320_kick.ui.components.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local BassTrack = ____constants.BassTrack
local ColorTrack2 = ____constants.ColorTrack2
local ____constantsValue = require("config.pixel.240x320_kick.ui.constantsValue")
local bottomLeftKnob = ____constantsValue.bottomLeftKnob
local bottomRightKnob = ____constantsValue.bottomRightKnob
local topValues = ____constantsValue.topValues
local ____TextGridBass = require("config.pixel.240x320_kick.ui.Bass.TextGridBass")
local TextGridBass = ____TextGridBass.TextGridBass
function ____exports.BassWaveformView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(GraphEncoder, {
            position = topValues,
            plugin = "Bass",
            data_id = "WAVEFORM",
            encoders = {"0 MORPH", "2 STAIRCASE"},
            track = BassTrack,
            is_array = true
        }),
        React.createElement(KnobValue, {
            value = "Bass NOISE",
            position = bottomLeftKnob,
            encoder_id = 1,
            COLOR = "quaternary",
            track = BassTrack
        }),
        React.createElement(KnobValue, {
            value = "Bass GAIN_CLIPPING",
            position = bottomRightKnob,
            encoder_id = 3,
            COLOR = "secondary",
            track = BassTrack
        }),
        React.createElement(TextGridBass, {selected = 4, viewName = name}),
        React.createElement(Common, {selected = 3, track = BassTrack, selectedBackground = ColorTrack2})
    )
end
return ____exports
