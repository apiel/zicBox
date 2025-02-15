local ____lualib = require("lualib_bundle")
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local ____exports = {}
local React = require("config.libs.react")
local ____Value = require("config.libs.nativeComponents.Value")
local Value = ____Value.Value
local ____VisibilityContainer = require("config.libs.nativeComponents.VisibilityContainer")
local VisibilityContainer = ____VisibilityContainer.VisibilityContainer
local ____VisibilityContext = require("config.libs.nativeComponents.VisibilityContext")
local VisibilityContext = ____VisibilityContext.VisibilityContext
local ____constants = require("config.pixel.240x320.ui.seqView.values.constants")
local bottomLeft = ____constants.bottomLeft
local bottomRight = ____constants.bottomRight
local posContainerValues = ____constants.posContainerValues
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
        {bounds = posContainerValues, group = group},
        React.createElement(VisibilityContext, {index = 10, condition = "SHOW_WHEN_OVER", value = 0}),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Sequencer STEP_NOTE",
                bounds = topLeft,
                group = group,
                track = track,
                encoder_id = 0
            }, tertiary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Sequencer STEP_CONDITION",
                bounds = bottomLeft,
                group = group,
                track = track,
                encoder_id = 1
            }, primary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Sequencer STEP_VELOCITY",
                bounds = topRight,
                group = group,
                track = track,
                encoder_id = 2
            }, secondary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "Sequencer STEP_MOTION",
                bounds = bottomRight,
                group = group,
                track = track,
                encoder_id = 3
            }, quaternary)
        )
    )
end
return ____exports
