--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____MacroEnvelop = require("config.libs.components.MacroEnvelop")
local MacroEnvelop = ____MacroEnvelop.MacroEnvelop
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_kick.ui.components.Common")
local Common = ____Common.Common
local ____constantsValue = require("config.pixel.240x320_kick.ui.constantsValue")
local fullValues = ____constantsValue.fullValues
local ____TextGridSynth = require("config.pixel.240x320_kick.ui.Synth.TextGridSynth")
local TextGridSynth = ____TextGridSynth.TextGridSynth
function ____exports.FrequencyView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(MacroEnvelop, {position = fullValues, plugin = "Drum23", envelop_data_id = "ENV_FREQ2"}),
        React.createElement(TextGridSynth, {selected = 4, viewName = name}),
        React.createElement(Common, {selected = 0})
    )
end
return ____exports
