--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Keymap = require("config.libs.components.Keymap")
local Keymap = ____Keymap.Keymap
local ____List = require("config.libs.components.List")
local List = ____List.List
local ____TextGrid = require("config.libs.components.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____constants = require("config.pixel.240x320.ui.constants")
local KeyInfoPositionCenter = ____constants.KeyInfoPositionCenter
local ScreenWidth = ____constants.ScreenWidth
function ____exports.MenuView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(
            List,
            {position = {0, 0, ScreenWidth, 280}, items = {"Workspaces", "Daw", "More", "Shutdown"}},
            React.createElement(Keymap, {key = "a", action = ".setView"}),
            React.createElement(Keymap, {key = "w", action = ".up"}),
            React.createElement(Keymap, {key = "s", action = ".down"})
        ),
        React.createElement(
            TextGrid,
            {position = KeyInfoPositionCenter, rows = {"  &icon::arrowUp::filled  ", "Select &icon::arrowDown::filled Exit"}},
            React.createElement(Keymap, {key = "d", action = "setView:Home"})
        )
    )
end
return ____exports