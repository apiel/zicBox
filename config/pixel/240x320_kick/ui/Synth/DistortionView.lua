local ____lualib = require("lualib_bundle")
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local ____exports = {}
local React = require("config.libs.react")
local ____Value = require("config.libs.components.Value")
local Value = ____Value.Value
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_kick.ui.components.Common")
local Common = ____Common.Common
local ____constantsValue = require("config.pixel.240x320_kick.ui.constantsValue")
local bottomLeft = ____constantsValue.bottomLeft
local bottomRight = ____constantsValue.bottomRight
local primary = ____constantsValue.primary
local quaternary = ____constantsValue.quaternary
local secondary = ____constantsValue.secondary
local tertiary = ____constantsValue.tertiary
local topLeft = ____constantsValue.topLeft
local topRight = ____constantsValue.topRight
local ____TextGridSynth = require("config.pixel.240x320_kick.ui.Synth.TextGridSynth")
local TextGridSynth = ____TextGridSynth.TextGridSynth
function ____exports.DistortionView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(
            Value,
            __TS__ObjectAssign({value = "Distortion WAVESHAPE", position = topLeft, encoder_id = 0}, tertiary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({value = "Distortion DRIVE", position = bottomLeft, encoder_id = 1}, primary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({value = "Distortion COMPRESS", position = topRight, encoder_id = 2}, secondary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({value = "Distortion BASS", position = bottomRight, encoder_id = 3}, quaternary)
        ),
        React.createElement(TextGridSynth, {selected = 0, viewName = name}),
        React.createElement(Common, {selected = 0})
    )
end
return ____exports
