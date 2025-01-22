local ____lualib = require("lualib_bundle")
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local ____exports = {}
local React = require("config.libs.react")
local ____Rect = require("config.libs.components.Rect")
local Rect = ____Rect.Rect
local ____Value = require("config.libs.components.Value")
local Value = ____Value.Value
local ____VisibilityContainer = require("config.libs.components.VisibilityContainer")
local VisibilityContainer = ____VisibilityContainer.VisibilityContainer
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local PrimaryBar = ____constants.PrimaryBar
local QuaternaryBar = ____constants.QuaternaryBar
local ScreenWidth = ____constants.ScreenWidth
local SecondaryBar = ____constants.SecondaryBar
local TertiaryBar = ____constants.TertiaryBar
local W2_4 = ____constants.W2_4
local valueH = 29
local rowSpacing = 8
local topLeft = {0, 0, W2_4 - 2, valueH}
local bottomLeft = {0, valueH + rowSpacing, W2_4 - 2, valueH}
local topRight = {W2_4, 0, W2_4 - 2, valueH}
local bottomRight = {W2_4, valueH + rowSpacing, W2_4 - 2, valueH}
local posContainer = {0, 222, ScreenWidth, valueH * 2 + rowSpacing}
local base = {SHOW_LABEL_OVER_VALUE = 6, LABEL_FONT_SIZE = 8}
____exports.primary = __TS__ObjectAssign({LABEL_COLOR = "primary", BAR_COLOR = PrimaryBar}, base)
____exports.secondary = __TS__ObjectAssign({LABEL_COLOR = "secondary", BAR_COLOR = SecondaryBar}, base)
____exports.tertiary = __TS__ObjectAssign({LABEL_COLOR = "tertiary", BAR_COLOR = TertiaryBar}, base)
____exports.quaternary = __TS__ObjectAssign({LABEL_COLOR = "quaternary", BAR_COLOR = QuaternaryBar}, base)
function ____exports.MasterValues(____bindingPattern0)
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    return React.createElement(
        VisibilityContainer,
        {position = posContainer, group = group},
        React.createElement(Rect, {position = {0, 0, ScreenWidth, posContainer[4]}}),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Volume VOLUME",
                position = topLeft,
                group = group,
                track = track,
                encoder_id = 0
            }, ____exports.tertiary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Volume GAIN_CLIPPING",
                position = bottomLeft,
                group = group,
                track = track,
                encoder_id = 1
            }, ____exports.primary)
        )
    )
end
function ____exports.Drum23Values(____bindingPattern0)
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    return React.createElement(
        VisibilityContainer,
        {position = posContainer, group = group},
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "TrackFx VOLUME",
                position = topLeft,
                group = group,
                track = track,
                encoder_id = 0
            }, ____exports.tertiary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Drum23 GAIN_CLIPPING",
                position = bottomLeft,
                group = group,
                track = track,
                encoder_id = 1
            }, ____exports.primary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "MMFilter CUTOFF",
                position = topRight,
                group = group,
                track = track,
                encoder_id = 2
            }, ____exports.quaternary, {USE_STRING_VALUE = true})
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "MMFilter RESONANCE",
                position = bottomRight,
                group = group,
                track = track,
                encoder_id = 3
            }, ____exports.quaternary)
        )
    )
end
function ____exports.FmValues(____bindingPattern0)
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    return React.createElement(
        VisibilityContainer,
        {position = posContainer, group = group},
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "TrackFx VOLUME",
                position = topLeft,
                group = group,
                track = track,
                encoder_id = 0
            }, ____exports.tertiary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "FmDrum DISTORTION",
                position = bottomLeft,
                group = group,
                track = track,
                encoder_id = 1
            }, ____exports.primary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "FmDrum REVERB",
                position = topRight,
                group = group,
                track = track,
                encoder_id = 2
            }, ____exports.quaternary, {USE_STRING_VALUE = true})
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "FmDrum MOD_INDEX",
                position = bottomRight,
                group = group,
                track = track,
                encoder_id = 3
            }, ____exports.secondary)
        )
    )
end
function ____exports.SnareValues(____bindingPattern0)
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    return React.createElement(
        VisibilityContainer,
        {position = posContainer, group = group},
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "TrackFx VOLUME_DRIVE",
                position = topLeft,
                group = group,
                track = track,
                encoder_id = 0
            }, ____exports.tertiary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "TrackFx REVERB",
                position = bottomLeft,
                group = group,
                track = track,
                encoder_id = 1
            }, ____exports.primary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "TrackFx CUTOFF",
                position = topRight,
                group = group,
                track = track,
                encoder_id = 2
            }, ____exports.quaternary, {USE_STRING_VALUE = true})
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "TrackFx RESONANCE",
                position = bottomRight,
                group = group,
                track = track,
                encoder_id = 3
            }, ____exports.quaternary)
        )
    )
end
function ____exports.HiHatValues(____bindingPattern0)
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    return React.createElement(
        VisibilityContainer,
        {position = posContainer, group = group},
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "TrackFx VOLUME_DRIVE",
                position = topLeft,
                group = group,
                track = track,
                encoder_id = 0
            }, ____exports.tertiary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "TrackFx REVERB",
                position = bottomLeft,
                group = group,
                track = track,
                encoder_id = 1
            }, ____exports.primary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "TrackFx CUTOFF",
                position = topRight,
                group = group,
                track = track,
                encoder_id = 2
            }, ____exports.quaternary, {USE_STRING_VALUE = true})
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "TrackFx RESONANCE",
                position = bottomRight,
                group = group,
                track = track,
                encoder_id = 3
            }, ____exports.quaternary)
        )
    )
end
function ____exports.SampleValues(____bindingPattern0)
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    return React.createElement(
        VisibilityContainer,
        {position = posContainer, group = group},
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "TrackFx VOLUME_DRIVE",
                position = topLeft,
                group = group,
                track = track,
                encoder_id = 0
            }, ____exports.tertiary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "TrackFx REVERB",
                position = bottomLeft,
                group = group,
                track = track,
                encoder_id = 1
            }, ____exports.primary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "TrackFx CUTOFF",
                position = topRight,
                group = group,
                track = track,
                encoder_id = 2
            }, ____exports.quaternary, {USE_STRING_VALUE = true})
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "TrackFx RESONANCE",
                position = bottomRight,
                group = group,
                track = track,
                encoder_id = 3
            }, ____exports.quaternary)
        )
    )
end
return ____exports
