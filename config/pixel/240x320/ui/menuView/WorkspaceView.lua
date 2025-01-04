--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Keymap = require("config.libs.components.Keymap")
local Keymap = ____Keymap.Keymap
local ____TextGrid = require("config.libs.components.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____VisibilityContext = require("config.libs.components.VisibilityContext")
local VisibilityContext = ____VisibilityContext.VisibilityContext
local ____constants = require("config.pixel.240x320.ui.constants")
local KeyInfoPositionCenter = ____constants.KeyInfoPositionCenter
function ____exports.WorkspaceView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(
            TextGrid,
            {position = KeyInfoPositionCenter, rows = {"&empty &icon::arrowUp::filled ...", "Use &icon::arrowDown::filled Exit"}},
            React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 0}),
            React.createElement(Keymap, {key = "d", action = "setView:Home"}),
            React.createElement(Keymap, {key = "e", action = "contextToggle:254:1:0"})
        ),
        React.createElement(
            TextGrid,
            {position = KeyInfoPositionCenter, rows = {"New &empty ^...", "&empty &empty &icon::trash"}},
            React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 1}),
            React.createElement(Keymap, {key = "q", action = "setView:CreateWorkspace", action2 = "contextToggle:254:1:0"}),
            React.createElement(Keymap, {key = "e", action = "contextToggle:254:1:0"})
        )
    )
end
return ____exports
