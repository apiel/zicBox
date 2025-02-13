local ____lualib = require("lualib_bundle")
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local ____exports = {}
local Main, Edit, EditFx, EditSample, EditLoop, Container
local React = require("config.libs.react")
local ____Log = require("config.libs.components.Log")
local Log = ____Log.Log
local ____Rect = require("config.libs.components.Rect")
local Rect = ____Rect.Rect
local ____Sample = require("config.libs.components.Sample")
local Sample = ____Sample.Sample
local ____Value = require("config.libs.components.Value")
local Value = ____Value.Value
local ____VisibilityContainer = require("config.libs.components.VisibilityContainer")
local VisibilityContainer = ____VisibilityContainer.VisibilityContainer
local ____VisibilityContext = require("config.libs.components.VisibilityContext")
local VisibilityContext = ____VisibilityContext.VisibilityContext
local ____constants = require("config.pixel.240x320.ui.constants")
local ScreenWidth = ____constants.ScreenWidth
local ____constants = require("config.pixel.240x320.ui.seqView.values.constants")
local bottomLeft = ____constants.bottomLeft
local bottomRight = ____constants.bottomRight
local height = ____constants.height
local posContainerValues = ____constants.posContainerValues
local posTopContainer = ____constants.posTopContainer
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
                encoder_id = 0
            }, tertiary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Volume DRIVE",
                position = bottomLeft,
                group = group,
                track = track,
                encoder_id = 1
            }, primary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "MMFilter CUTOFF",
                position = topRight,
                group = group,
                track = track,
                encoder_id = 2
            }, quaternary, {USE_STRING_VALUE = true})
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "MMFilter RESONANCE",
                position = bottomRight,
                group = group,
                track = track,
                encoder_id = 3
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
        React.createElement(EditFx, {group = group, track = track, context = context}),
        React.createElement(EditSample, {group = group, track = track, context = context}),
        React.createElement(EditLoop, {group = group, track = track, context = context})
    )
end
function EditFx(____bindingPattern0)
    local context
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    context = ____bindingPattern0.context
    local topFull = {table.unpack(topLeft)}
    topFull[3] = ScreenWidth
    return React.createElement(
        Container,
        {group = group, context = context, values = {seq = 0, menu = 1}},
        React.createElement(Rect, {position = {0, 0, ScreenWidth, height}}),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Volume GAIN_CLIPPING",
                position = topLeft,
                group = group,
                track = track,
                encoder_id = 0
            }, secondary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "SampleRateReducer SAMPLE_STEP",
                position = topRight,
                group = group,
                track = track,
                encoder_id = 2
            }, quaternary)
        )
    )
end
function EditSample(____bindingPattern0)
    local context
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    context = ____bindingPattern0.context
    local topFull = {table.unpack(topLeft)}
    topFull[3] = ScreenWidth
    return React.createElement(
        Container,
        {group = group, context = context, values = {seq = 0, menu = 2}},
        React.createElement(Rect, {position = {0, 0, ScreenWidth, height}}),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "MonoSample BROWSER",
                position = topFull,
                group = group,
                track = track,
                encoder_id = 0
            }, tertiary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "MonoSample START",
                position = bottomLeft,
                group = group,
                track = track,
                encoder_id = 1
            }, quaternary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "MonoSample END",
                position = bottomRight,
                group = group,
                track = track,
                encoder_id = 3
            }, quaternary)
        )
    )
end
function EditLoop(____bindingPattern0)
    local context
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    context = ____bindingPattern0.context
    return React.createElement(
        Container,
        {group = group, context = context, values = {seq = 0, menu = 3}},
        React.createElement(Rect, {position = {0, 0, ScreenWidth, height}}),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "MonoSample LOOP_POSITION",
                position = topLeft,
                group = group,
                track = track,
                encoder_id = 0,
                label = "Position"
            }, tertiary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "MonoSample LOOP_LENGTH",
                position = topRight,
                group = group,
                track = track,
                encoder_id = 2,
                label = "Length"
            }, tertiary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "MonoSample LOOP_RELEASE",
                position = bottomLeft,
                group = group,
                track = track,
                encoder_id = 1
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
        {position = posContainerValues, group = group},
        React.createElement(VisibilityContext, {index = 10, condition = "SHOW_WHEN", value = values.seq}),
        React.createElement(VisibilityContext, {index = context, condition = "SHOW_WHEN", value = values.menu})
    )
end
function ____exports.MonoSampleValues(____bindingPattern0)
    local context
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    context = ____bindingPattern0.context
    return React.createElement(
        React.Fragment,
        nil,
        React.createElement(Log, {text = "MonoSampleValues"}),
        React.createElement(
            VisibilityContainer,
            {position = posTopContainer, group = group},
            React.createElement(Sample, {position = {0, 0, ScreenWidth, 70}, track = track, plugin = "MonoSample SAMPLE_BUFFER SAMPLE_INDEX", loop_points_color = "tertiary"})
        ),
        React.createElement(Edit, {group = group, track = track, context = context}),
        React.createElement(Main, {group = group, track = track, context = context}),
        React.createElement(SeqValues, {group = group, track = track})
    )
end
return ____exports
