--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____KnobValue = require("config.libs.components.KnobValue")
local KnobValue = ____KnobValue.KnobValue
local ____View = require("config.libs.components.View")
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
function ____exports.ClickView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(KnobValue, {
            value = "Drum23 CLICK",
            position = topLeftKnob,
            encoder_id = encTopLeft,
            COLOR = "tertiary",
            track = Drum23Track
        }),
        React.createElement(KnobValue, {
            value = "Drum23 CLICK_CUTOFF",
            position = topRightKnob,
            encoder_id = encTopRight,
            COLOR = "primary",
            track = Drum23Track
        }),
        React.createElement(KnobValue, {
            value = "Drum23 CLICK_DURATION",
            position = bottomLeftKnob,
            encoder_id = encBottomLeft,
            COLOR = "quaternary",
            track = Drum23Track
        }),
        React.createElement(KnobValue, {
            value = "Drum23 HIGH_FREQ_BOOST",
            position = bottomRightKnob,
            encoder_id = encBottomRight,
            COLOR = "secondary",
            track = Drum23Track
        }),
        React.createElement(TextGridDrum23, {selected = "Click", viewName = name}),
        React.createElement(Common, {selected = "Kick", track = Drum23Track, selectedBackground = ColorTrack1})
    )
end
return ____exports
