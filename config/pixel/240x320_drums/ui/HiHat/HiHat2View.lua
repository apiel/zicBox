--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____KnobValue = require("config.libs.nativeComponents.KnobValue")
local KnobValue = ____KnobValue.KnobValue
local ____View = require("config.libs.nativeComponents.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_drums.ui.components.Common")
local Common = ____Common.Common
local ____TextGridDrums = require("config.pixel.240x320_drums.ui.components.TextGridDrums")
local TextGridDrums = ____TextGridDrums.TextGridDrums
local ____constants = require("config.pixel.240x320_drums.ui.constants")
local HiHatTrack = ____constants.HiHatTrack
local ____constantsValue = require("config.pixel.240x320_drums.ui.constantsValue")
local bottomLeftKnob = ____constantsValue.bottomLeftKnob
local topLeftKnob = ____constantsValue.topLeftKnob
local topRightKnob = ____constantsValue.topRightKnob
function ____exports.HiHat2View(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(KnobValue, {
            value = "HiHat BAND_FREQ",
            bounds = topLeftKnob,
            encoderId = 0,
            COLOR = "tertiary",
            track = HiHatTrack
        }),
        React.createElement(KnobValue, {
            value = "HiHat BAND_Q",
            bounds = topRightKnob,
            encoderId = 2,
            COLOR = "secondary",
            track = HiHatTrack
        }),
        React.createElement(KnobValue, {value = "HiHat TONE_BRIGHTNESS", bounds = bottomLeftKnob, encoderId = 1, track = HiHatTrack}),
        React.createElement(TextGridDrums, {selected = 3, viewName = name, target = "HiHat"}),
        React.createElement(Common, {selected = 1, track = HiHatTrack})
    )
end
return ____exports
