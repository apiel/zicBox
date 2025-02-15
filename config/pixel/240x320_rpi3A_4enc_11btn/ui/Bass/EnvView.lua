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
local BassTrack = ____constants.BassTrack
local ColorTrack2 = ____constants.ColorTrack2
local encBottomLeft = ____constants.encBottomLeft
local encTopLeft = ____constants.encTopLeft
local encTopRight = ____constants.encTopRight
local ____constantsValue = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constantsValue")
local bottomLeftKnob = ____constantsValue.bottomLeftKnob
local topLeftKnob = ____constantsValue.topLeftKnob
local topRightKnob = ____constantsValue.topRightKnob
local ____TextGridBass = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Bass.TextGridBass")
local TextGridBass = ____TextGridBass.TextGridBass
function ____exports.BassEnvView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(KnobValue, {
            audioPlugin = "Bass",
            param = "DURATION",
            bounds = topLeftKnob,
            encoderId = encTopLeft,
            color = "tertiary",
            track = BassTrack
        }),
        React.createElement(KnobValue, {
            audioPlugin = "Bass",
            param = "DECAY_LEVEL",
            bounds = topRightKnob,
            encoderId = encTopRight,
            color = "primary",
            track = BassTrack
        }),
        React.createElement(KnobValue, {
            audioPlugin = "Bass",
            param = "DECAY_TIME",
            bounds = bottomLeftKnob,
            encoderId = encBottomLeft,
            color = "quaternary",
            track = BassTrack
        }),
        React.createElement(TextGridBass, {selected = "Env", viewName = name}),
        React.createElement(Common, {selected = "Bass", track = BassTrack, selectedBackground = ColorTrack2})
    )
end
return ____exports
