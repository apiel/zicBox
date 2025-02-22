--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____pixelController = require("config.libs.controllers.pixelController")
local pixelController = ____pixelController.pixelController
local ____core = require("config.libs.core")
local applyZic = ____core.applyZic
local ____Rect = require("config.libs.nativeComponents.Rect")
local Rect = ____Rect.Rect
local ____Text = require("config.libs.nativeComponents.Text")
local Text = ____Text.Text
local ____Value = require("config.libs.nativeComponents.Value")
local Value = ____Value.Value
local ____View = require("config.libs.nativeComponents.View")
local View = ____View.View
local ____ui = require("config.libs.ui")
local setScreenSize = ____ui.setScreenSize
local setWindowPosition = ____ui.setWindowPosition
local ____Keymap = require("config.libs.nativeComponents.Keymap")
local Keymap = ____Keymap.Keymap
local ____yo = require("config.yo")
local Yo = ____yo.Yo
local ____yo2 = require("config.yo2")
local Yo2 = ____yo2.Yo2
local ScreenWidth = 240
local ScreenHeight = 320
pixelController("rpi3A_4enc_11btn")
setWindowPosition(400, 500)
setScreenSize(ScreenWidth, ScreenHeight)
applyZic(React.createElement(
    React.Fragment,
    nil,
    React.createElement(
        View,
        {name = "Demo"},
        React.createElement(Text, {fontSize = 16, text = "title", bounds = {0, 0, ScreenWidth, 16}, centered = true}),
        React.createElement(Rect, {color = "tertiary", filled = false, bounds = {10, 30, 100, 50}}),
        React.createElement(
            Value,
            {
                audioPlugin = "Tempo",
                param = "BPM",
                bounds = {120, 60, 110, 20},
                encoderId = 0,
                barColor = "quaternary"
            },
            React.createElement(Keymap, {key = "s", action = "setView:Drum23", context = "254:1"})
        ),
        React.createElement(Yo2, nil)
    ),
    React.createElement(Yo, {name = "Yo"})
))
return ____exports
