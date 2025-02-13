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
local ColorTrack3 = ____constants.ColorTrack3
local encBottomLeft = ____constants.encBottomLeft
local encTopLeft = ____constants.encTopLeft
local encTopRight = ____constants.encTopRight
local Fm1Track = ____constants.Fm1Track
local ____constantsValue = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constantsValue")
local bottomLeftKnob = ____constantsValue.bottomLeftKnob
local topLeftKnob = ____constantsValue.topLeftKnob
local topRightKnob = ____constantsValue.topRightKnob
local ____TextGridFm1 = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Fm1 copy.TextGridFm1")
local TextGridFm1 = ____TextGridFm1.TextGridFm1
function ____exports.Fm1FxView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(KnobValue, {
            value = "FmDrum NOISE_LEVEL",
            position = topLeftKnob,
            encoder_id = encTopLeft,
            COLOR = "tertiary",
            track = Fm1Track
        }),
        React.createElement(KnobValue, {
            value = "FmDrum DISTORTION",
            position = topRightKnob,
            encoder_id = encTopRight,
            COLOR = "primary",
            track = Fm1Track
        }),
        React.createElement(KnobValue, {
            value = "FmDrum REVERB",
            position = bottomLeftKnob,
            encoder_id = encBottomLeft,
            COLOR = "quaternary",
            track = Fm1Track
        }),
        React.createElement(TextGridFm1, {selected = "Fx", viewName = name}),
        React.createElement(Common, {selected = "Fm1", track = Fm1Track, selectedBackground = ColorTrack3})
    )
end
return ____exports
