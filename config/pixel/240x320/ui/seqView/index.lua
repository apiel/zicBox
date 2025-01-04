--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Keymap = require("config.libs.components.Keymap")
local Keymap = ____Keymap.Keymap
local ____Keymaps = require("config.libs.components.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____TextGrid = require("config.libs.components.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____Value = require("config.libs.components.Value")
local Value = ____Value.Value
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____VisibilityContext = require("config.libs.components.VisibilityContext")
local VisibilityContext = ____VisibilityContext.VisibilityContext
local ____VisibilityGroup = require("config.libs.components.VisibilityGroup")
local VisibilityGroup = ____VisibilityGroup.VisibilityGroup
local ____constants = require("config.pixel.240x320.ui.constants")
local KeyInfoPosition = ____constants.KeyInfoPosition
local W1_4 = ____constants.W1_4
local W3_4 = ____constants.W3_4
function ____exports.SeqView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(
            TextGrid,
            {position = KeyInfoPosition, rows = {"&icon::play::filled &icon::arrowUp::filled ...", "&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled"}},
            React.createElement(VisibilityGroup, {condition = "SHOW_WHEN", group = 0}),
            React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 0}),
            React.createElement(Keymap, {key = "q", action = "playPause"}),
            React.createElement(Keymap, {key = "e", action = "contextToggle:254:1:0"})
        ),
        React.createElement(
            TextGrid,
            {position = KeyInfoPosition, rows = {"Menu &icon::play::filled ^...", "Clips ? Save"}},
            React.createElement(VisibilityGroup, {condition = "SHOW_WHEN", group = 0}),
            React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 1}),
            React.createElement(Keymaps, {keys = {{key = "q", action = "setView:Menu", action2 = "contextToggle:254:1:0"}, {key = "w", action = "playPause"}, {key = "a", action = "setView:Clips"}, {key = "e", action = "contextToggle:254:1:0"}}})
        ),
        React.createElement(Value, {
            value = "Tempo BPM",
            position = {W3_4, KeyInfoPosition[2], W1_4, 22},
            SHOW_LABEL_OVER_VALUE = 0,
            BAR_HEIGHT = 0,
            VALUE_FONT_SIZE = 16
        })
    )
end
return ____exports
