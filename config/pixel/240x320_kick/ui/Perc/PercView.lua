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
local PercTrack = ____constants.PercTrack
local ____constantsValue = require("config.pixel.240x320_kick.ui.constantsValue")
local bottomLeftKnob = ____constantsValue.bottomLeftKnob
local bottomRightKnob = ____constantsValue.bottomRightKnob
local topLeftKnob = ____constantsValue.topLeftKnob
local topRightKnob = ____constantsValue.topRightKnob
local ____TextGridPerc = require("config.pixel.240x320_kick.ui.Perc.TextGridPerc")
local TextGridPerc = ____TextGridPerc.TextGridPerc
function ____exports.PercView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(KnobValue, {
            value = "Volume VOLUME",
            position = topLeftKnob,
            encoder_id = 0,
            COLOR = "tertiary",
            track = PercTrack
        }),
        React.createElement(KnobValue, {
            value = "Perc CARRIER_FREQ",
            position = topRightKnob,
            encoder_id = 2,
            COLOR = "secondary",
            track = PercTrack
        }),
        React.createElement(KnobValue, {value = "Perc MOD_FREQ", position = bottomLeftKnob, encoder_id = 1, track = PercTrack}),
        React.createElement(KnobValue, {
            value = "Perc MOD_INDEX",
            position = bottomRightKnob,
            encoder_id = 3,
            COLOR = "secondary",
            track = PercTrack
        }),
        React.createElement(TextGridPerc, {selected = 1, viewName = name}),
        React.createElement(Common, {selected = 1, track = PercTrack})
    )
end
return ____exports
