--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Keymap = require("config.libs.components.Keymap")
local Keymap = ____Keymap.Keymap
local ____Text = require("config.libs.components.Text")
local Text = ____Text.Text
local ____TextGrid = require("config.libs.components.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local KeyInfoPosition = ____constants.KeyInfoPosition
local ScreenWidth = ____constants.ScreenWidth
function ____exports.ShutdownView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(Text, {position = {0, 0, ScreenWidth, 280}, text = "Are you sure to shutdown?", color = "#ffacac", centered = true}),
        React.createElement(
            TextGrid,
            {position = KeyInfoPosition, rows = {"&empty &empty &empty", "Yes &empty No"}},
            React.createElement(Keymap, {key = "q", action = "shutdown"}),
            React.createElement(Keymap, {key = "a", action = "shutdown"}),
            React.createElement(Keymap, {key = "d", action = "setView:Clips"}),
            React.createElement(Keymap, {key = "e", action = "setView:Clips"})
        )
    )
end
return ____exports
