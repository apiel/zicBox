--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____MacroEnvelop = require("config.libs.components.MacroEnvelop")
local MacroEnvelop = ____MacroEnvelop.MacroEnvelop
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_kick.ui.components.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local ColorTrack1 = ____constants.ColorTrack1
local Drum23Track = ____constants.Drum23Track
local ____constantsValue = require("config.pixel.240x320_kick.ui.constantsValue")
local fullValues = ____constantsValue.fullValues
local ____TextGridBass = require("config.pixel.240x320_kick.ui.Bass.TextGridBass")
local TextGridBass = ____TextGridBass.TextGridBass
function ____exports.FrequencyView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(MacroEnvelop, {position = fullValues, plugin = "Drum23", envelop_data_id = "ENV_FREQ2", track = Drum23Track}),
        React.createElement(TextGridBass, {selected = 1, viewName = name}),
        React.createElement(Common, {selected = 0, track = Drum23Track, selectedBackground = ColorTrack1})
    )
end
return ____exports