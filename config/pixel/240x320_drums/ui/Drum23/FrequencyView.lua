--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____MacroEnvelop = require("config.libs.nativeComponents.MacroEnvelop")
local MacroEnvelop = ____MacroEnvelop.MacroEnvelop
local ____View = require("config.libs.nativeComponents.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_drums.ui.components.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_drums.ui.constants")
local Drum23Track = ____constants.Drum23Track
local ____constantsValue = require("config.pixel.240x320_drums.ui.constantsValue")
local fullValues = ____constantsValue.fullValues
local ____TextGridDrum23 = require("config.pixel.240x320_drums.ui.Drum23.TextGridDrum23")
local TextGridDrum23 = ____TextGridDrum23.TextGridDrum23
function ____exports.FrequencyView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(MacroEnvelop, {bounds = fullValues, plugin = "Drum23", envelop_data_id = "ENV_FREQ2", track = Drum23Track}),
        React.createElement(TextGridDrum23, {selected = 4, viewName = name}),
        React.createElement(Common, {selected = 0, track = Drum23Track})
    )
end
return ____exports
