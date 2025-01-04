local ____lualib = require("lualib_bundle")
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local ____exports = {}
local React = require("config.libs.react")
local ____Value = require("config.libs.components.Value")
local Value = ____Value.Value
local ____VisibilityContainer = require("config.libs.components.VisibilityContainer")
local VisibilityContainer = ____VisibilityContainer.VisibilityContainer
local ____VisibilityContext = require("config.libs.components.VisibilityContext")
local VisibilityContext = ____VisibilityContext.VisibilityContext
local ____constants = require("config.pixel.240x320.ui.seqView.values.constants")
local bottomLeft = ____constants.bottomLeft
local bottomRight = ____constants.bottomRight
local posContainer = ____constants.posContainer
local primary = ____constants.primary
local quaternary = ____constants.quaternary
local secondary = ____constants.secondary
local tertiary = ____constants.tertiary
local topLeft = ____constants.topLeft
local topRight = ____constants.topRight
function ____exports.SeqValues(____bindingPattern0)
    local track
    local group
    group = ____bindingPattern0.group
    track = ____bindingPattern0.track
    return React.createElement(
        VisibilityContainer,
        {position = posContainer, group = group},
        React.createElement(VisibilityContext, {index = 10, condition = "SHOW_WHEN_OVER", value = 0}),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Sequencer STEP_NOTE",
                position = topLeft,
                group = group,
                track = track,
                encoderId = 0
            }, tertiary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Sequencer STEP_CONDITION",
                position = bottomLeft,
                group = group,
                track = track,
                encoderId = 1
            }, primary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Sequencer STEP_VELOCITY",
                position = topRight,
                group = group,
                track = track,
                encoderId = 2
            }, secondary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Sequencer STEP_MOTION",
                position = bottomRight,
                group = group,
                track = track,
                encoderId = 3
            }, quaternary)
        )
    )
end
return ____exports
