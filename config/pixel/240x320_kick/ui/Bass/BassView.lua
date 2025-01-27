--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____KnobValue = require("config.libs.components.KnobValue")
local KnobValue = ____KnobValue.KnobValue
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_kick.ui.components.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local BassTrack = ____constants.BassTrack
local ColorTrack3 = ____constants.ColorTrack3
local ____constantsValue = require("config.pixel.240x320_kick.ui.constantsValue")
local bottomLeftKnob = ____constantsValue.bottomLeftKnob
local bottomRightKnob = ____constantsValue.bottomRightKnob
local topLeftKnob = ____constantsValue.topLeftKnob
local topRightKnob = ____constantsValue.topRightKnob
local ____TextGridBass = require("config.pixel.240x320_kick.ui.Bass.TextGridBass")
local TextGridBass = ____TextGridBass.TextGridBass
function ____exports.BassView(____bindingPattern0)
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
            track = BassTrack
        }),
        React.createElement(KnobValue, {
            value = "Bass CUTOFF",
            position = topRightKnob,
            encoder_id = 2,
            COLOR = "primary",
            track = BassTrack
        }),
        React.createElement(KnobValue, {
            value = "Bass PITCH",
            position = bottomLeftKnob,
            encoder_id = 1,
            COLOR = "quaternary",
            track = BassTrack
        }),
        React.createElement(KnobValue, {
            value = "Bass RESONANCE",
            position = bottomRightKnob,
            encoder_id = 3,
            COLOR = "secondary",
            track = BassTrack
        }),
        React.createElement(TextGridBass, {selected = 0, viewName = name}),
        React.createElement(Common, {selected = 3, track = BassTrack, selectedBackground = ColorTrack3})
    )
end
return ____exports
