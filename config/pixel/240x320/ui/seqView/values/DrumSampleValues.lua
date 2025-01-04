local ____lualib = require("lualib_bundle")
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local ____exports = {}
local Main, Edit
local React = require("config.libs.react")
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
local ____constants = require("config.pixel.240x320.ui.seqView.values.constants")
local bottomLeft = ____constants.bottomLeft
local bottomRight = ____constants.bottomRight
local height = ____constants.height
local posY = ____constants.posY
local primary = ____constants.primary
local quaternary = ____constants.quaternary
local tertiary = ____constants.tertiary
local topLeft = ____constants.topLeft
local topRight = ____constants.topRight
local valueH = ____constants.valueH
function Main(____bindingPattern0)
    local context
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    context = ____bindingPattern0.context
    return React.createElement(
        VisibilityContainer,
        {position = {0, posY, ScreenWidth, valueH * 2 + 2}, group = group},
        React.createElement(VisibilityContext, {index = 10, condition = "SHOW_WHEN", value = 0}),
        React.createElement(VisibilityContext, {index = context, condition = "SHOW_WHEN", value = 0}),
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
                value = "Volume GAIN_CLIPPING",
                position = bottomLeft,
                group = group,
                track = track,
                encoderId = 1
            }, primary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Volume DRIVE",
                position = topRight,
                group = group,
                track = track,
                encoderId = 2
            }, quaternary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "SampleRateReducer SAMPLE_STEP",
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
        VisibilityContainer,
        {position = {0, posY, ScreenWidth, valueH * 2 + 2}, group = group},
        React.createElement(VisibilityContext, {index = 10, condition = "SHOW_WHEN", value = 0}),
        React.createElement(VisibilityContext, {index = context, condition = "SHOW_WHEN", value = 1}),
        React.createElement(Rect, {position = {0, 0, ScreenWidth, height}}),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "DrumSample BROWSER",
                position = topLeft,
                group = group,
                track = track,
                encoderId = 0
            }, tertiary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "DrumSample START",
                position = bottomLeft,
                group = group,
                track = track,
                encoderId = 1
            }, quaternary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "DrumSample END",
                position = bottomRight,
                group = group,
                track = track,
                encoderId = 3
            }, quaternary)
        )
    )
end
function ____exports.DrumSampleValues(____bindingPattern0)
    local context
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    context = ____bindingPattern0.context
    return React.createElement(
        React.Fragment,
        nil,
        React.createElement(Edit, {group = group, track = track, context = context}),
        React.createElement(Main, {group = group, track = track, context = context})
    )
end
return ____exports
