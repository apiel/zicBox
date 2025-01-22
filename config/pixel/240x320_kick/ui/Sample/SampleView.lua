--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____KnobValue = require("config.libs.components.KnobValue")
local KnobValue = ____KnobValue.KnobValue
local ____Sample = require("config.libs.components.Sample")
local Sample = ____Sample.Sample
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_kick.ui.components.Common")
local Common = ____Common.Common
local ____TextGridDrums = require("config.pixel.240x320_kick.ui.components.TextGridDrums")
local TextGridDrums = ____TextGridDrums.TextGridDrums
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local SampleTrack = ____constants.SampleTrack
local ScreenWidth = ____constants.ScreenWidth
local ____constantsValue = require("config.pixel.240x320_kick.ui.constantsValue")
local bottomLeftKnob = ____constantsValue.bottomLeftKnob
local bottomRightKnob = ____constantsValue.bottomRightKnob
local topLeftKnob = ____constantsValue.topLeftKnob
local topRightKnob = ____constantsValue.topRightKnob
function ____exports.SampleView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(KnobValue, {
            value = "TrackFx VOLUME_DRIVE",
            position = topLeftKnob,
            encoder_id = 0,
            COLOR = "tertiary",
            track = SampleTrack
        }),
        React.createElement(KnobValue, {
            value = "Sample BROWSER",
            position = topRightKnob,
            encoder_id = 2,
            track = SampleTrack,
            STRING_VALUE_REPLACE_TITLE = true
        }),
        React.createElement(KnobValue, {
            value = "Sample START",
            position = bottomLeftKnob,
            encoder_id = 1,
            COLOR = "secondary",
            track = SampleTrack
        }),
        React.createElement(KnobValue, {
            value = "Sample END",
            position = bottomRightKnob,
            encoder_id = 3,
            COLOR = "secondary",
            track = SampleTrack
        }),
        React.createElement(Sample, {
            position = {0, 245, ScreenWidth, 50},
            track = SampleTrack,
            plugin = "Sample SAMPLE_BUFFER SAMPLE_INDEX",
            loop_points_color = "tertiary",
            BACKGROUND_COLOR = "background"
        }),
        React.createElement(TextGridDrums, {selected = 1, viewName = name, target = "Sample"}),
        React.createElement(Common, {selected = 1, track = SampleTrack})
    )
end
return ____exports
