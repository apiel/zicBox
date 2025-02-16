--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Adsr = require("config.libs.nativeComponents.Adsr")
local Adsr = ____Adsr.Adsr
local ____View = require("config.libs.nativeComponents.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.components.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local ColorTrack4 = ____constants.ColorTrack4
local SynthTrack = ____constants.SynthTrack
local ____constantsValue = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constantsValue")
local halfFullValues = ____constantsValue.halfFullValues
local ____TextGridSynth = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Synth.TextGridSynth")
local TextGridSynth = ____TextGridSynth.TextGridSynth
function ____exports.SynthEnv2View(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(Adsr, {bounds = halfFullValues, audioPlugin = "Synth", values = {"ATTACK_2", "DECAY_2", "SUSTAIN_2", "RELEASE_2"}}),
        React.createElement(TextGridSynth, {selected = "Env2", viewName = name}),
        React.createElement(Common, {selected = "Synth", track = SynthTrack, selectedBackground = ColorTrack4})
    )
end
return ____exports
