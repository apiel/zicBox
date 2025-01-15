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
local W2_4 = ____constants.W2_4
local ____TextGridSynth = require("config.pixel.240x320_kick.ui.Synth.TextGridSynth")
local TextGridSynth = ____TextGridSynth.TextGridSynth
function ____exports.MasterView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(KnobValue, {value = "Volume VOLUME", position = {0, 50, W2_4 - 2, 80}, encoder_id = 0, COLOR = "tertiary"}),
        React.createElement(KnobValue, {
            value = "MMFilter CUTOFF",
            position = {W2_4, 50, W2_4 - 2, 80},
            encoder_id = 2,
            COLOR = "quaternary",
            FONT_VALUE_SIZE = 8,
            TYPE = "STRING"
        }),
        React.createElement(KnobValue, {value = "Drum23 GAIN_CLIPPING", position = {0, 150, W2_4 - 2, 80}, encoder_id = 1}),
        React.createElement(KnobValue, {value = "MMFilter RESONANCE", position = {W2_4, 150, W2_4 - 2, 80}, encoder_id = 3, COLOR = "quaternary"}),
        React.createElement(TextGridSynth, {selected = 0, viewName = name}),
        React.createElement(Common, {selected = 1})
    )
end
return ____exports
