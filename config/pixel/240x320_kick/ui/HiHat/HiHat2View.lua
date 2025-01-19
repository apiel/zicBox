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
local HiHatTrack = ____constants.HiHatTrack
local ____constantsValue = require("config.pixel.240x320_kick.ui.constantsValue")
local bottomLeftKnob = ____constantsValue.bottomLeftKnob
local topLeftKnob = ____constantsValue.topLeftKnob
local ____TextGridHiHat = require("config.pixel.240x320_kick.ui.HiHat.TextGridHiHat")
local TextGridHiHat = ____TextGridHiHat.TextGridHiHat
function ____exports.HiHat2View(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(KnobValue, {
            value = "HiHat BAND_FREQ",
            position = topLeftKnob,
            encoder_id = 0,
            COLOR = "tertiary",
            track = HiHatTrack
        }),
        React.createElement(KnobValue, {value = "HiHat BAND_Q", position = bottomLeftKnob, encoder_id = 1, track = HiHatTrack}),
        React.createElement(TextGridHiHat, {selected = 3, viewName = name}),
        React.createElement(Common, {selected = 1})
    )
end
return ____exports
