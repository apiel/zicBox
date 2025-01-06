local ____lualib = require("lualib_bundle")
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local ____exports = {}
local React = require("config.libs.react")
local ____Log = require("config.libs.components.Log")
local Log = ____Log.Log
local ____Rect = require("config.libs.components.Rect")
local Rect = ____Rect.Rect
local ____Spectrogram = require("config.libs.components.Spectrogram")
local Spectrogram = ____Spectrogram.Spectrogram
local ____Value = require("config.libs.components.Value")
local Value = ____Value.Value
local ____VisibilityContainer = require("config.libs.components.VisibilityContainer")
local VisibilityContainer = ____VisibilityContainer.VisibilityContainer
local ____constants = require("config.pixel.240x320.ui.constants")
local ScreenWidth = ____constants.ScreenWidth
local ____constants = require("config.pixel.240x320.ui.seqView.values.constants")
local bottomRight = ____constants.bottomRight
local height = ____constants.height
local posContainerValues = ____constants.posContainerValues
local posTopContainer = ____constants.posTopContainer
local quaternary = ____constants.quaternary
local tertiary = ____constants.tertiary
local topLeft = ____constants.topLeft
local topRight = ____constants.topRight
function ____exports.MasterValues()
    return React.createElement(
        React.Fragment,
        nil,
        React.createElement(
            VisibilityContainer,
            {position = posTopContainer, group = 0},
            React.createElement(Spectrogram, {position = {0, 0, ScreenWidth, 70}, track = 0, data = "Spectrogram BUFFER", text = "Pixel"})
        ),
        React.createElement(
            VisibilityContainer,
            {position = posContainerValues, group = 0},
            React.createElement(Log, {text = "MasterValues"}),
            React.createElement(Rect, {position = {0, 0, ScreenWidth, height}}),
            React.createElement(
                Value,
                __TS__ObjectAssign({
                    value = "MasterVolume VOLUME",
                    position = topLeft,
                    group = 0,
                    track = 0,
                    encoder_id = 0
                }, tertiary)
            ),
            React.createElement(
                Value,
                __TS__ObjectAssign({
                    value = "MasterFilter CUTOFF",
                    position = topRight,
                    group = 0,
                    track = 0,
                    encoder_id = 2
                }, quaternary, {USE_STRING_VALUE = true})
            ),
            React.createElement(
                Value,
                __TS__ObjectAssign({
                    value = "MasterFilter RESONANCE",
                    position = bottomRight,
                    group = 0,
                    track = 0,
                    encoder_id = 3
                }, quaternary)
            )
        )
    )
end
return ____exports
