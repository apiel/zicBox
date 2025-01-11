local ____lualib = require("lualib_bundle")
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local ____exports = {}
local React = require("config.libs.react")
local ____Value = require("config.libs.components.Value")
local Value = ____Value.Value
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_kick.ui.Common")
local Common = ____Common.Common
local ____constantsValue = require("config.pixel.240x320_kick.ui.constantsValue")
local bottomLeft = ____constantsValue.bottomLeft
local bottomRight = ____constantsValue.bottomRight
local primary = ____constantsValue.primary
local quaternary = ____constantsValue.quaternary
local tertiary = ____constantsValue.tertiary
local topLeft = ____constantsValue.topLeft
local topRight = ____constantsValue.topRight
local ____ProgressBar = require("config.pixel.240x320_kick.ui.ProgressBar")
local ProgressBar = ____ProgressBar.ProgressBar
local ____TextGridPage1 = require("config.pixel.240x320_kick.ui.Page1.TextGridPage1")
local TextGridPage1 = ____TextGridPage1.TextGridPage1
function ____exports.Page1View(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(
            Value,
            __TS__ObjectAssign({value = "Volume VOLUME", position = topLeft, encoder_id = 0}, tertiary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({value = "Drum23 GAIN_CLIPPING", position = bottomLeft, encoder_id = 1}, primary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({value = "MMFilter CUTOFF", position = topRight, encoder_id = 2}, quaternary, {USE_STRING_VALUE = true})
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({value = "MMFilter RESONANCE", position = bottomRight, encoder_id = 3}, quaternary)
        ),
        React.createElement(ProgressBar, nil),
        React.createElement(TextGridPage1, {selected = 0}),
        React.createElement(Common, {selected = 0})
    )
end
return ____exports
