--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____MacroEnvelop = require("config.libs.nativeComponents.MacroEnvelop")
local MacroEnvelop = ____MacroEnvelop.MacroEnvelop
local ____View = require("config.libs.nativeComponents.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.components.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local ColorTrack1 = ____constants.ColorTrack1
local Drum23Track = ____constants.Drum23Track
local ____constantsValue = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constantsValue")
local fullValues = ____constantsValue.fullValues
local ____TextGridDrum23 = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Drum23.TextGridDrum23")
local TextGridDrum23 = ____TextGridDrum23.TextGridDrum23
function ____exports.FrequencyView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(MacroEnvelop, {bounds = fullValues, plugin = "Drum23", envelop_data_id = "ENV_FREQ2", track = Drum23Track}),
        React.createElement(TextGridDrum23, {selected = "Freq", viewName = name}),
        React.createElement(Common, {selected = "Kick", track = Drum23Track, selectedBackground = ColorTrack1})
    )
end
return ____exports
