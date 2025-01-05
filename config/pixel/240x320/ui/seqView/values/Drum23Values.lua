local ____lualib = require("lualib_bundle")
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local ____exports = {}
local Main, Edit, EditDistortion, EditWaveform, EditEnvAmp, EditEnvFreq, EditClick, Container
local React = require("config.libs.react")
local ____DrumEnvelop = require("config.libs.components.DrumEnvelop")
local DrumEnvelop = ____DrumEnvelop.DrumEnvelop
local ____Encoder3 = require("config.libs.components.Encoder3")
local Encoder3 = ____Encoder3.Encoder3
local ____GraphEncoder = require("config.libs.components.GraphEncoder")
local GraphEncoder = ____GraphEncoder.GraphEncoder
local ____Log = require("config.libs.components.Log")
local Log = ____Log.Log
local ____Rect = require("config.libs.components.Rect")
local Rect = ____Rect.Rect
local ____Value = require("config.libs.components.Value")
local Value = ____Value.Value
local ____VisibilityContainer = require("config.libs.components.VisibilityContainer")
local VisibilityContainer = ____VisibilityContainer.VisibilityContainer
local ____VisibilityContext = require("config.libs.components.VisibilityContext")
local VisibilityContext = ____VisibilityContext.VisibilityContext
local ____constants = require("config.pixel.240x320.ui.constants")
local ScreenWidth = ____constants.ScreenWidth
local W1_4 = ____constants.W1_4
local W3_4 = ____constants.W3_4
local ____constants = require("config.pixel.240x320.ui.seqView.values.constants")
local bottomLeft = ____constants.bottomLeft
local bottomRight = ____constants.bottomRight
local encoderH = ____constants.encoderH
local height = ____constants.height
local posContainer = ____constants.posContainer
local primary = ____constants.primary
local quaternary = ____constants.quaternary
local secondary = ____constants.secondary
local tertiary = ____constants.tertiary
local topLeft = ____constants.topLeft
local topRight = ____constants.topRight
local ____SeqValues = require("config.pixel.240x320.ui.seqView.values.SeqValues")
local SeqValues = ____SeqValues.SeqValues
function Main(____bindingPattern0)
    local context
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    context = ____bindingPattern0.context
    return React.createElement(
        Container,
        {group = group, context = context, values = {seq = 0, menu = 0}},
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Volume VOLUME",
                position = topLeft,
                group = group,
                track = track,
                encoderId = 0
            }, tertiary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Drum23 GAIN_CLIPPING",
                position = bottomLeft,
                group = group,
                track = track,
                encoderId = 1
            }, primary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "MMFilter CUTOFF",
                position = topRight,
                group = group,
                track = track,
                encoderId = 2
            }, quaternary, {USE_STRING_VALUE = true})
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "MMFilter RESONANCE",
                position = bottomRight,
                group = group,
                track = track,
                encoderId = 3
            }, quaternary)
        )
    )
end
function Edit(____bindingPattern0)
    local context
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    context = ____bindingPattern0.context
    return React.createElement(
        React.Fragment,
        nil,
        React.createElement(EditDistortion, {group = group, track = track, context = context, menu = 1}),
        React.createElement(EditWaveform, {group = group, track = track, context = context, menu = 2}),
        React.createElement(EditEnvAmp, {group = group, track = track, context = context, menu = 3}),
        React.createElement(EditEnvFreq, {group = group, track = track, context = context, menu = 4}),
        React.createElement(EditClick, {group = group, track = track, context = context, menu = 5})
    )
end
function EditDistortion(____bindingPattern0)
    local menu
    local context
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    context = ____bindingPattern0.context
    menu = ____bindingPattern0.menu
    return React.createElement(
        Container,
        {group = group, context = context, values = {seq = 0, menu = menu}},
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Distortion WAVESHAPE",
                position = topLeft,
                group = group,
                track = track,
                encoderId = 0
            }, tertiary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Distortion DRIVE",
                position = bottomLeft,
                group = group,
                track = track,
                encoderId = 1
            }, primary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Distortion COMPRESS",
                position = topRight,
                group = group,
                track = track,
                encoderId = 2
            }, secondary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Distortion BASS",
                position = bottomRight,
                group = group,
                track = track,
                encoderId = 3
            }, quaternary)
        )
    )
end
function EditWaveform(____bindingPattern0)
    local menu
    local context
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    context = ____bindingPattern0.context
    menu = ____bindingPattern0.menu
    return React.createElement(
        Container,
        {group = group, context = context, values = {seq = 0, menu = menu}},
        React.createElement(GraphEncoder, {
            position = {0, 0, ScreenWidth, height},
            track = track,
            plugin = "Drum23",
            data_id = "WAVEFORM",
            RENDER_TITLE_ON_TOP = false,
            encoders = {"0 WAVEFORM_TYPE", "2 MACRO", "1 SHAPE"}
        })
    )
end
function EditEnvAmp(____bindingPattern0)
    local menu
    local context
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    context = ____bindingPattern0.context
    menu = ____bindingPattern0.menu
    return React.createElement(
        Container,
        {group = group, context = context, values = {seq = 0, menu = menu}},
        React.createElement(Rect, {position = {0, 0, ScreenWidth, height}}),
        React.createElement(DrumEnvelop, {
            position = {0, 0, W3_4 - 2, height},
            track = track,
            plugin = "Drum23",
            envelop_data_id = "0",
            RENDER_TITLE_ON_TOP = false,
            encoder_time = 0,
            encoder_phase = 1,
            encoder_modulation = 2
        }),
        React.createElement(Encoder3, {
            position = {W3_4, (height - encoderH) * 0.5, W1_4, encoderH},
            track = track,
            value = "Drum23 DURATION",
            encoder_id = 3,
            color = "quaternary"
        })
    )
end
function EditEnvFreq(____bindingPattern0)
    local menu
    local context
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    context = ____bindingPattern0.context
    menu = ____bindingPattern0.menu
    return React.createElement(
        Container,
        {group = group, context = context, values = {seq = 0, menu = menu}},
        React.createElement(Rect, {position = {0, 0, ScreenWidth, height}}),
        React.createElement(DrumEnvelop, {
            position = {0, 0, W3_4 - 2, height},
            track = track,
            plugin = "Drum23",
            envelop_data_id = "4",
            RENDER_TITLE_ON_TOP = false,
            encoder_time = 0,
            encoder_phase = 1,
            encoder_modulation = 2
        }),
        React.createElement(Encoder3, {
            position = {W3_4, (height - encoderH) * 0.5, W1_4, encoderH},
            track = track,
            value = "Drum23 PITCH",
            encoder_id = 3,
            color = "secondary"
        })
    )
end
function EditClick(____bindingPattern0)
    local menu
    local context
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    context = ____bindingPattern0.context
    menu = ____bindingPattern0.menu
    return React.createElement(
        Container,
        {group = group, context = context, values = {seq = 0, menu = menu}},
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Drum23 CLICK",
                position = topLeft,
                group = group,
                track = track,
                encoderId = 0
            }, tertiary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Drum23 CLICK_DURATION",
                position = bottomLeft,
                group = group,
                track = track,
                encoderId = 1
            }, primary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Drum23 CLICK_CUTOFF",
                position = topRight,
                group = group,
                track = track,
                encoderId = 2
            }, secondary, {USE_STRING_VALUE = true})
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Drum23 CLICK_RESONANCE",
                position = bottomRight,
                group = group,
                track = track,
                encoderId = 3
            }, quaternary)
        )
    )
end
function Container(____bindingPattern0)
    local values
    local context
    local group
    group = ____bindingPattern0.group
    context = ____bindingPattern0.context
    values = ____bindingPattern0.values
    return React.createElement(
        VisibilityContainer,
        {position = posContainer, group = group},
        React.createElement(VisibilityContext, {index = 10, condition = "SHOW_WHEN", value = values.seq}),
        React.createElement(VisibilityContext, {index = context, condition = "SHOW_WHEN", value = values.menu})
    )
end
function ____exports.Drum23Values(____bindingPattern0)
    local context
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    context = ____bindingPattern0.context
    return React.createElement(
        React.Fragment,
        nil,
        React.createElement(Log, {text = "Drum23Values"}),
        React.createElement(Edit, {group = group, track = track, context = context}),
        React.createElement(Main, {group = group, track = track, context = context}),
        React.createElement(SeqValues, {group = group, track = track})
    )
end
return ____exports
