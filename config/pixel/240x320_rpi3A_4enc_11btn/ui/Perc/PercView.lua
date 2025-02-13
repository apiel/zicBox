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
local ColorTrack6 = ____constants.ColorTrack6
local encBottomLeft = ____constants.encBottomLeft
local encBottomRight = ____constants.encBottomRight
local encTopLeft = ____constants.encTopLeft
local encTopRight = ____constants.encTopRight
local PercTrack = ____constants.PercTrack
local ____constantsValue = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constantsValue")
local bottomLeftKnob = ____constantsValue.bottomLeftKnob
local bottomRightKnob = ____constantsValue.bottomRightKnob
local topLeftKnob = ____constantsValue.topLeftKnob
local topRightKnob = ____constantsValue.topRightKnob
local ____TextGridPerc = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Perc.TextGridPerc")
local TextGridPerc = ____TextGridPerc.TextGridPerc
function ____exports.PercView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(KnobValue, {
            value = "TrackFx VOLUME",
            position = topLeftKnob,
            encoder_id = encTopLeft,
            COLOR = "tertiary",
            track = PercTrack
        }),
        React.createElement(KnobValue, {
            value = "Perc DURATION",
            position = topRightKnob,
            encoder_id = encTopRight,
            COLOR = "primary",
            track = PercTrack
        }),
        React.createElement(KnobValue, {
            value = "Perc BASE_FREQ",
            position = bottomLeftKnob,
            encoder_id = encBottomLeft,
            COLOR = "quaternary",
            track = PercTrack
        }),
        React.createElement(KnobValue, {
            value = "Perc TONE_DECAY",
            position = bottomRightKnob,
            encoder_id = encBottomRight,
            COLOR = "secondary",
            track = PercTrack
        }),
        React.createElement(TextGridPerc, {selected = "Main", viewName = name}),
        React.createElement(Common, {selected = "Perc", track = PercTrack, selectedBackground = ColorTrack6})
    )
end
return ____exports
