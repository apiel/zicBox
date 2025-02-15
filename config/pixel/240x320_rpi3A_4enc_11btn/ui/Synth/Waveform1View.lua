--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____GraphEncoder = require("config.libs.nativeComponents.GraphEncoder")
local GraphEncoder = ____GraphEncoder.GraphEncoder
local ____KnobValue = require("config.libs.nativeComponents.KnobValue")
local KnobValue = ____KnobValue.KnobValue
local ____View = require("config.libs.nativeComponents.View")
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
local topValues = ____constantsValue.topValues
local ____TextGridSynth = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Synth.TextGridSynth")
local TextGridSynth = ____TextGridSynth.TextGridSynth
function ____exports.SynthWaveform1View(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(
            GraphEncoder,
            {
                bounds = topValues,
                plugin = "Synth",
                data_id = "WAVEFORM1",
                RENDER_TITLE_ON_TOP = true,
                encoders = {
                    tostring(encTopLeft) .. " SHAPE_1",
                    tostring(encTopRight) .. " MORPH_1"
                },
                track = SynthTrack
            }
        ),
        React.createElement(KnobValue, {
            audioPlugin = "Synth",
            param = "FREQ_1",
            bounds = bottomLeftKnob,
            encoderId = encBottomLeft,
            color = "quaternary",
            track = SynthTrack
        }),
        React.createElement(TextGridSynth, {selected = "Osc1", viewName = name}),
        React.createElement(Common, {selected = "Synth", track = SynthTrack, selectedBackground = ColorTrack4})
    )
end
return ____exports
