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
local ____VisibilityGroup = require("config.libs.components.VisibilityGroup")
local VisibilityGroup = ____VisibilityGroup.VisibilityGroup
local ____constants = require("config.pixel.240x320.ui.constants")
local ScreenWidth = ____constants.ScreenWidth
local ____constants = require("config.pixel.240x320.ui.seqView.values.constants")
local bottomRight = ____constants.bottomRight
local height = ____constants.height
local posY = ____constants.posY
local quaternary = ____constants.quaternary
local tertiary = ____constants.tertiary
local topLeft = ____constants.topLeft
local topRight = ____constants.topRight
local valueH = ____constants.valueH
local function MasterValues(____bindingPattern0)
    local synth = ____bindingPattern0.synth
    return React.createElement(
        VisibilityContainer,
        {position = {0, posY, ScreenWidth, valueH * 2 + 2}},
        React.createElement(VisibilityGroup, {condition = "SHOW_WHEN", group = 0}),
        React.createElement(Rect, {position = {0, 0, ScreenWidth, height}}),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "MasterVolume VOLUME",
                position = topLeft,
                group = 0,
                track = 0,
                encoderId = 0
            }, tertiary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "MasterFilter CUTOFF",
                position = topRight,
                group = 0,
                track = 0,
                encoderId = 2
            }, quaternary, {USE_STRING_VALUE = true})
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({
                value = "MasterFilter RESONANCE",
                position = bottomRight,
                group = 0,
                track = 0,
                encoderId = 3
            }, quaternary)
        )
    )
end
return ____exports
