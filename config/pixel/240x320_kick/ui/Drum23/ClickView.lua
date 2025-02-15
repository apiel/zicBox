--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
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
local bottomLeftKnob = ____constantsValue.bottomLeftKnob
local bottomRightKnob = ____constantsValue.bottomRightKnob
local topLeftKnob = ____constantsValue.topLeftKnob
local topRightKnob = ____constantsValue.topRightKnob
local ____TextGridDrum23 = require("config.pixel.240x320_kick.ui.Drum23.TextGridDrum23")
local TextGridDrum23 = ____TextGridDrum23.TextGridDrum23
function ____exports.ClickView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(KnobValue, {
            audioPlugin = "Drum23",
            param = "CLICK",
            bounds = topLeftKnob,
            encoderId = 0,
            color = "tertiary",
            track = Drum23Track
        }),
        React.createElement(KnobValue, {
            audioPlugin = "Drum23",
            param = "CLICK_CUTOFF",
            bounds = topRightKnob,
            encoderId = 1,
            color = "primary",
            track = Drum23Track
        }),
        React.createElement(KnobValue, {
            audioPlugin = "Drum23",
            param = "CLICK_DURATION",
            bounds = bottomLeftKnob,
            encoderId = 2,
            color = "quaternary",
            track = Drum23Track
        }),
        React.createElement(KnobValue, {
            audioPlugin = "Drum23",
            param = "HIGH_FREQ_BOOST",
            bounds = bottomRightKnob,
            encoderId = 3,
            color = "secondary",
            track = Drum23Track
        }),
        React.createElement(TextGridDrum23, {selected = 4, viewName = name}),
        React.createElement(Common, {selected = 0, track = Drum23Track, selectedBackground = ColorTrack1})
    )
end
return ____exports
