--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____KnobValue = require("config.libs.nativeComponents.KnobValue")
local KnobValue = ____KnobValue.KnobValue
local ____View = require("config.libs.nativeComponents.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.components.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local ColorTrack1 = ____constants.ColorTrack1
local Drum23Track = ____constants.Drum23Track
local encBottomLeft = ____constants.encBottomLeft
local encBottomRight = ____constants.encBottomRight
local encTopLeft = ____constants.encTopLeft
local encTopRight = ____constants.encTopRight
local ____constantsValue = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constantsValue")
local bottomLeftKnob = ____constantsValue.bottomLeftKnob
local bottomRightKnob = ____constantsValue.bottomRightKnob
local topLeftKnob = ____constantsValue.topLeftKnob
local topRightKnob = ____constantsValue.topRightKnob
local ____TextGridDrum23 = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Drum23.TextGridDrum23")
local TextGridDrum23 = ____TextGridDrum23.TextGridDrum23
function ____exports.Drum23View(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(KnobValue, {
            audioPlugin = "TrackFx",
            param = "VOLUME",
            bounds = topLeftKnob,
            encoderId = encTopLeft,
            color = "tertiary",
            track = Drum23Track
        }),
        React.createElement(KnobValue, {
            audioPlugin = "MMFilter",
            param = "CUTOFF",
            bounds = topRightKnob,
            encoderId = encTopRight,
            color = "secondary",
            valueSize = 8,
            type = "STRING",
            track = Drum23Track
        }),
        React.createElement(KnobValue, {
            audioPlugin = "Drum23",
            param = "GAIN_CLIPPING",
            bounds = bottomLeftKnob,
            encoderId = encBottomLeft,
            track = Drum23Track
        }),
        React.createElement(KnobValue, {
            audioPlugin = "MMFilter",
            param = "RESONANCE",
            bounds = bottomRightKnob,
            encoderId = encBottomRight,
            color = "secondary",
            track = Drum23Track
        }),
        React.createElement(Common, {selected = "Kick", track = Drum23Track, selectedBackground = ColorTrack1}),
        React.createElement(TextGridDrum23, {selected = "Main", viewName = name})
    )
end
return ____exports
