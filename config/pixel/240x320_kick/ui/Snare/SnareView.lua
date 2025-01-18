--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____KnobValue = require("config.libs.components.KnobValue")
local KnobValue = ____KnobValue.KnobValue
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_kick.ui.components.Common")
local Common = ____Common.Common
local ____constantsValue = require("config.pixel.240x320_kick.ui.constantsValue")
local topLeftKnob = ____constantsValue.topLeftKnob
local ____TextGridSnare = require("config.pixel.240x320_kick.ui.Snare.TextGridSnare")
local TextGridSnare = ____TextGridSnare.TextGridSnare
function ____exports.SnareView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(KnobValue, {value = "Volume VOLUME", position = topLeftKnob, encoder_id = 0, COLOR = "tertiary"}),
        React.createElement(TextGridSnare, {selected = 0, viewName = name}),
        React.createElement(Common, {selected = 1})
    )
end
return ____exports
