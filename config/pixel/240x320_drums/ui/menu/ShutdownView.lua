--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Keymap = require("config.libs.nativeComponents.Keymap")
local Keymap = ____Keymap.Keymap
local ____Text = require("config.libs.nativeComponents.Text")
local Text = ____Text.Text
local ____View = require("config.libs.nativeComponents.View")
local View = ____View.View
local ____TextGrid = require("config.libs.tsComponents.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____constants = require("config.pixel.240x320_drums.ui.constants")
local KeyInfoPosition = ____constants.KeyInfoPosition
local ScreenWidth = ____constants.ScreenWidth
function ____exports.ShutdownView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(Text, {bounds = {0, 0, ScreenWidth, 280}, text = "Are you sure to shutdown?", color = "#ffacac", centered = true}),
        React.createElement(
            TextGrid,
            {bounds = KeyInfoPosition, rows = {"&empty &empty &empty", "Yes &empty No"}},
            React.createElement(Keymap, {key = "q", action = "shutdown"}),
            React.createElement(Keymap, {key = "a", action = "shutdown"}),
            React.createElement(Keymap, {key = "d", action = "setView:Clips"}),
            React.createElement(Keymap, {key = "e", action = "setView:Clips"})
        )
    )
end
return ____exports
