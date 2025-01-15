--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____DrumEnvelop = require("config.libs.components.DrumEnvelop")
local DrumEnvelop = ____DrumEnvelop.DrumEnvelop
local ____KnobValue = require("config.libs.components.KnobValue")
local KnobValue = ____KnobValue.KnobValue
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_kick.ui.components.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local W1_4 = ____constants.W1_4
local W3_4 = ____constants.W3_4
local ____constantsValue = require("config.pixel.240x320_kick.ui.constantsValue")
local encoderH = ____constantsValue.encoderH
local height = ____constantsValue.height
local ____TextGridSynth = require("config.pixel.240x320_kick.ui.Synth.TextGridSynth")
local TextGridSynth = ____TextGridSynth.TextGridSynth
function ____exports.AmpView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(DrumEnvelop, {
            position = {0, 0, W3_4 - 2, height},
            plugin = "Drum23",
            envelop_data_id = "0",
            RENDER_TITLE_ON_TOP = false,
            encoder_time = 0,
            encoder_phase = 1,
            encoder_modulation = 2
        }),
        React.createElement(KnobValue, {position = {W3_4, (height - encoderH) * 0.5, W1_4, encoderH}, value = "Drum23 DURATION", encoder_id = 3, color = "quaternary"}),
        React.createElement(TextGridSynth, {selected = 3, viewName = name}),
        React.createElement(Common, {selected = 1})
    )
end
return ____exports
