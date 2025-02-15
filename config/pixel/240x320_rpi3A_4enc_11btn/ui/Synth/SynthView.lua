--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____KnobValue = require("config.libs.components.KnobValue")
local KnobValue = ____KnobValue.KnobValue
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.components.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local ColorTrack4 = ____constants.ColorTrack4
local encBottomLeft = ____constants.encBottomLeft
local encTopLeft = ____constants.encTopLeft
local encTopRight = ____constants.encTopRight
local SynthTrack = ____constants.SynthTrack
local ____constantsValue = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constantsValue")
local bottomLeftKnob = ____constantsValue.bottomLeftKnob
local topLeftKnob = ____constantsValue.topLeftKnob
local topRightKnob = ____constantsValue.topRightKnob
local ____TextGridSynth = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Synth.TextGridSynth")
local TextGridSynth = ____TextGridSynth.TextGridSynth
function ____exports.SynthView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(KnobValue, {
            value = "TrackFx VOLUME",
            bounds = topLeftKnob,
            encoder_id = encTopLeft,
            COLOR = "tertiary",
            track = SynthTrack
        }),
        React.createElement(KnobValue, {
            value = "Synth OSC_MIX",
            bounds = topRightKnob,
            encoder_id = encTopRight,
            COLOR = "primary",
            track = SynthTrack
        }),
        React.createElement(KnobValue, {
            value = "Synth FM_AMOUNT",
            bounds = bottomLeftKnob,
            encoder_id = encBottomLeft,
            COLOR = "quaternary",
            track = SynthTrack
        }),
        React.createElement(TextGridSynth, {selected = "Main", viewName = name}),
        React.createElement(Common, {selected = "Synth", track = SynthTrack, selectedBackground = ColorTrack4})
    )
end
return ____exports
