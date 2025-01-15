--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____KnobValue = require("config.libs.components.KnobValue")
local KnobValue = ____KnobValue.KnobValue
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_kick.ui.components.Common")
local Common = ____Common.Common
local ____constantsValue = require("config.pixel.240x320_kick.ui.constantsValue")
local bottomLeftKnob = ____constantsValue.bottomLeftKnob
local bottomRightKnob = ____constantsValue.bottomRightKnob
local topLeftKnob = ____constantsValue.topLeftKnob
local topRightKnob = ____constantsValue.topRightKnob
local ____TextGridSynth = require("config.pixel.240x320_kick.ui.Synth.TextGridSynth")
local TextGridSynth = ____TextGridSynth.TextGridSynth
function ____exports.DistortionView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(KnobValue, {value = "Distortion WAVESHAPE", position = topLeftKnob, encoder_id = 0, COLOR = "tertiary"}),
        React.createElement(KnobValue, {value = "Distortion COMPRESS", position = topRightKnob, encoder_id = 2, COLOR = "primary"}),
        React.createElement(KnobValue, {value = "Distortion DRIVE", position = bottomLeftKnob, encoder_id = 1, COLOR = "quaternary"}),
        React.createElement(KnobValue, {value = "Distortion BASS", position = bottomRightKnob, encoder_id = 3, COLOR = "secondary"}),
        React.createElement(TextGridSynth, {selected = 0, viewName = name}),
        React.createElement(Common, {selected = 0})
    )
end
return ____exports
