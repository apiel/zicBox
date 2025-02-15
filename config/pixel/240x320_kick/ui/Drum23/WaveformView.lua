--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____GraphEncoder = require("config.libs.nativeComponents.GraphEncoder")
local GraphEncoder = ____GraphEncoder.GraphEncoder
local ____KnobValue = require("config.libs.nativeComponents.KnobValue")
local KnobValue = ____KnobValue.KnobValue
local ____View = require("config.libs.nativeComponents.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_kick.ui.components.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local ColorTrack1 = ____constants.ColorTrack1
local Drum23Track = ____constants.Drum23Track
local ____constantsValue = require("config.pixel.240x320_kick.ui.constantsValue")
local bottomRightKnob = ____constantsValue.bottomRightKnob
local topValues = ____constantsValue.topValues
local ____TextGridDrum23 = require("config.pixel.240x320_kick.ui.Drum23.TextGridDrum23")
local TextGridDrum23 = ____TextGridDrum23.TextGridDrum23
function ____exports.WaveformView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(GraphEncoder, {
            bounds = topValues,
            plugin = "Drum23",
            data_id = "WAVEFORM",
            RENDER_TITLE_ON_TOP = false,
            encoders = {"0 WAVEFORM_TYPE", "1 SHAPE", "2 MACRO"},
            track = Drum23Track
        }),
        React.createElement(KnobValue, {
            bounds = bottomRightKnob,
            audioPlugin = "Drum23",
            param = "PITCH",
            encoderId = 3,
            color = "secondary",
            track = Drum23Track
        }),
        React.createElement(TextGridDrum23, {selected = 1, viewName = name}),
        React.createElement(Common, {selected = 0, track = Drum23Track, selectedBackground = ColorTrack1})
    )
end
return ____exports
