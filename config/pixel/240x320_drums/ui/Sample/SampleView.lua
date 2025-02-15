--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____KnobValue = require("config.libs.nativeComponents.KnobValue")
local KnobValue = ____KnobValue.KnobValue
local ____Sample = require("config.libs.nativeComponents.Sample")
local Sample = ____Sample.Sample
local ____View = require("config.libs.nativeComponents.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_drums.ui.components.Common")
local Common = ____Common.Common
local ____TextGridDrums = require("config.pixel.240x320_drums.ui.components.TextGridDrums")
local TextGridDrums = ____TextGridDrums.TextGridDrums
local ____constants = require("config.pixel.240x320_drums.ui.constants")
local SampleTrack = ____constants.SampleTrack
local ScreenWidth = ____constants.ScreenWidth
local ____constantsValue = require("config.pixel.240x320_drums.ui.constantsValue")
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
            value = "TrackFx VOLUME",
            bounds = topLeftKnob,
            encoderId = 0,
            COLOR = "tertiary",
            track = SampleTrack
        }),
        React.createElement(KnobValue, {
            value = "Sample BROWSER",
            bounds = topRightKnob,
            encoderId = 2,
            track = SampleTrack,
            STRING_VALUE_REPLACE_TITLE = true
        }),
        React.createElement(KnobValue, {
            value = "Sample START",
            bounds = bottomLeftKnob,
            encoderId = 1,
            COLOR = "secondary",
            track = SampleTrack
        }),
        React.createElement(KnobValue, {
            value = "Sample END",
            bounds = bottomRightKnob,
            encoderId = 3,
            COLOR = "secondary",
            track = SampleTrack
        }),
        React.createElement(Sample, {
            bounds = {0, 245, ScreenWidth, 50},
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
