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
local ____VisibilityContext = require("config.libs.components.VisibilityContext")
local VisibilityContext = ____VisibilityContext.VisibilityContext
local ____VisibilityGroup = require("config.libs.components.VisibilityGroup")
local VisibilityGroup = ____VisibilityGroup.VisibilityGroup
local ____constants = require("config.pixel.240x320.ui.constants")
local ScreenWidth = ____constants.ScreenWidth
local ____constants = require("config.pixel.240x320.ui.seqView.values.constants")
local bottomLeft = ____constants.bottomLeft
local bottomRight = ____constants.bottomRight
local height = ____constants.height
local posY = ____constants.posY
local quaternary = ____constants.quaternary
local tertiary = ____constants.tertiary
local topLeft = ____constants.topLeft
local valueH = ____constants.valueH
function ____exports.DrumSampleValues(____bindingPattern0)
    local context
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    context = ____bindingPattern0.context
    return React.createElement(
        VisibilityContainer,
        {position = {0, posY, ScreenWidth, valueH * 2 + 2}},
        React.createElement(VisibilityGroup, {condition = "SHOW_WHEN", group = group}),
        React.createElement(VisibilityContext, {index = 10, condition = "SHOW_WHEN", value = 0}),
        React.createElement(VisibilityContext, {index = context, condition = "SHOW_WHEN", value = 0}),
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
return ____exports
