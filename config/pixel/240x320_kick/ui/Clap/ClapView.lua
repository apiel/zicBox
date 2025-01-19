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
local ClapTrack = ____constants.ClapTrack
local ____constantsValue = require("config.pixel.240x320_kick.ui.constantsValue")
local bottomLeftKnob = ____constantsValue.bottomLeftKnob
local bottomRightKnob = ____constantsValue.bottomRightKnob
local topLeftKnob = ____constantsValue.topLeftKnob
local topRightKnob = ____constantsValue.topRightKnob
local ____TextGridClap = require("config.pixel.240x320_kick.ui.Clap.TextGridClap")
local TextGridClap = ____TextGridClap.TextGridClap
function ____exports.ClapView(____bindingPattern0)
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
            track = ClapTrack
        }),
        React.createElement(KnobValue, {
            value = "Clap DURATION",
            position = topRightKnob,
            encoder_id = 2,
            COLOR = "secondary",
            track = ClapTrack
        }),
        React.createElement(KnobValue, {value = "Clap TONE_CUTOFF", position = bottomLeftKnob, encoder_id = 1, track = ClapTrack}),
        React.createElement(KnobValue, {
            value = "Clap TRANSIENT_SPREAD",
            position = bottomRightKnob,
            encoder_id = 3,
            COLOR = "secondary",
            track = ClapTrack
        }),
        React.createElement(TextGridClap, {selected = 1, viewName = name}),
        React.createElement(Common, {selected = 1, track = ClapTrack})
    )
end
return ____exports
