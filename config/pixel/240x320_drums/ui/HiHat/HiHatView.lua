--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____KnobValue = require("config.libs.components.KnobValue")
local KnobValue = ____KnobValue.KnobValue
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_drums.ui.components.Common")
local Common = ____Common.Common
local ____TextGridDrums = require("config.pixel.240x320_drums.ui.components.TextGridDrums")
local TextGridDrums = ____TextGridDrums.TextGridDrums
local ____constants = require("config.pixel.240x320_drums.ui.constants")
local HiHatTrack = ____constants.HiHatTrack
local ____constantsValue = require("config.pixel.240x320_drums.ui.constantsValue")
local bottomLeftKnob = ____constantsValue.bottomLeftKnob
local bottomRightKnob = ____constantsValue.bottomRightKnob
local topLeftKnob = ____constantsValue.topLeftKnob
local topRightKnob = ____constantsValue.topRightKnob
function ____exports.HiHatView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(KnobValue, {
            value = "TrackFx VOLUME",
            position = topLeftKnob,
            encoder_id = 0,
            COLOR = "tertiary",
            track = HiHatTrack
        }),
        React.createElement(KnobValue, {
            value = "HiHat DURATION",
            position = topRightKnob,
            encoder_id = 2,
            COLOR = "secondary",
            track = HiHatTrack
        }),
        React.createElement(KnobValue, {value = "HiHat TRANSIENT_INTENSITY", position = bottomLeftKnob, encoder_id = 1, track = HiHatTrack}),
        React.createElement(KnobValue, {
            value = "HiHat METALLIC_TONE_MIX",
            position = bottomRightKnob,
            encoder_id = 3,
            COLOR = "secondary",
            track = HiHatTrack
        }),
        React.createElement(TextGridDrums, {selected = 3, viewName = name, target = "HiHat"}),
        React.createElement(Common, {selected = 1, track = HiHatTrack})
    )
end
return ____exports
