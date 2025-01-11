local ____lualib = require("lualib_bundle")
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local ____exports = {}
local React = require("config.libs.react")
local ____Value = require("config.libs.components.Value")
local Value = ____Value.Value
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____constantsValue = require("config.pixel.240x320_kick.ui.constantsValue")
local bottomLeft = ____constantsValue.bottomLeft
local bottomRight = ____constantsValue.bottomRight
local primary = ____constantsValue.primary
local quaternary = ____constantsValue.quaternary
local secondary = ____constantsValue.secondary
local tertiary = ____constantsValue.tertiary
local topLeft = ____constantsValue.topLeft
local topRight = ____constantsValue.topRight
local ____TextGridCommon = require("config.pixel.240x320_kick.ui.TextGridCommon")
local TextGridCommon = ____TextGridCommon.TextGridCommon
local ____TextGridPage2 = require("config.pixel.240x320_kick.ui.Page2.TextGridPage2")
local TextGridPage2 = ____TextGridPage2.TextGridPage2
function ____exports.ClickView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(
            Value,
            __TS__ObjectAssign({value = "Drum23 CLICK", position = topLeft, encoder_id = 0}, tertiary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({value = "Drum23 CLICK_DURATION", position = bottomLeft, encoder_id = 1}, primary)
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({value = "Drum23 CLICK_CUTOFF", position = topRight, encoder_id = 2}, secondary, {USE_STRING_VALUE = true})
        ),
        React.createElement(
            Value,
            __TS__ObjectAssign({value = "Drum23 CLICK_RESONANCE", position = bottomRight, encoder_id = 3}, quaternary)
        ),
        React.createElement(TextGridPage2, nil),
        React.createElement(TextGridCommon, {selected = 1})
    )
end
return ____exports
