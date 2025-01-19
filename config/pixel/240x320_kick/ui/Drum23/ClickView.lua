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
local Drum23Track = ____constants.Drum23Track
local ____constantsValue = require("config.pixel.240x320_kick.ui.constantsValue")
local bottomLeftKnob = ____constantsValue.bottomLeftKnob
local bottomRightKnob = ____constantsValue.bottomRightKnob
local topLeftKnob = ____constantsValue.topLeftKnob
local topRightKnob = ____constantsValue.topRightKnob
local ____TextGridDrum23 = require("config.pixel.240x320_kick.ui.Drum23.TextGridDrum23")
local TextGridDrum23 = ____TextGridDrum23.TextGridDrum23
function ____exports.ClickView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(KnobValue, {
            value = "Drum23 CLICK",
            position = topLeftKnob,
            encoder_id = 0,
            COLOR = "tertiary",
            track = Drum23Track
        }),
        React.createElement(KnobValue, {
            value = "Drum23 CLICK_CUTOFF",
            position = topRightKnob,
            encoder_id = 2,
            COLOR = "primary",
            TYPE = "STRING",
            FONT_VALUE_SIZE = 12,
            track = Drum23Track
        }),
        React.createElement(KnobValue, {
            value = "Drum23 CLICK_DURATION",
            position = bottomLeftKnob,
            encoder_id = 1,
            COLOR = "quaternary",
            track = Drum23Track
        }),
        React.createElement(KnobValue, {
            value = "Drum23 CLICK_RESONANCE",
            position = bottomRightKnob,
            encoder_id = 3,
            COLOR = "secondary",
            track = Drum23Track
        }),
        React.createElement(TextGridDrum23, {selected = 3, viewName = name}),
        React.createElement(Common, {selected = 1, track = Drum23Track})
    )
end
return ____exports
