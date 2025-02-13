--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____DrumEnvelop = require("config.libs.components.DrumEnvelop")
local DrumEnvelop = ____DrumEnvelop.DrumEnvelop
local ____KnobValue = require("config.libs.components.KnobValue")
local KnobValue = ____KnobValue.KnobValue
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.components.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local ColorTrack1 = ____constants.ColorTrack1
local Drum23Track = ____constants.Drum23Track
local encBottomLeft = ____constants.encBottomLeft
local encBottomRight = ____constants.encBottomRight
local encTopLeft = ____constants.encTopLeft
local encTopRight = ____constants.encTopRight
local ____constantsValue = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constantsValue")
local bottomRightKnob = ____constantsValue.bottomRightKnob
local topValues = ____constantsValue.topValues
local ____TextGridDrum23 = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Drum23.TextGridDrum23")
local TextGridDrum23 = ____TextGridDrum23.TextGridDrum23
function ____exports.AmpView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(DrumEnvelop, {
            position = topValues,
            plugin = "Drum23",
            envelop_data_id = "0",
            RENDER_TITLE_ON_TOP = false,
            encoder_time = encTopLeft,
            encoder_modulation = encTopRight,
            encoder_phase = encBottomLeft,
            track = Drum23Track
        }),
        React.createElement(KnobValue, {
            position = bottomRightKnob,
            value = "Drum23 DURATION",
            encoder_id = encBottomRight,
            color = "quaternary",
            track = Drum23Track
        }),
        React.createElement(TextGridDrum23, {selected = "Amp", viewName = name}),
        React.createElement(Common, {selected = "Kick", track = Drum23Track, selectedBackground = ColorTrack1})
    )
end
return ____exports
