--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____audio = require("config.libs.audio")
local plugin = ____audio.plugin
local pluginAlias = ____audio.pluginAlias
local ____pixelController = require("config.libs.controllers.pixelController")
local pixelController = ____pixelController.pixelController
local ____core = require("config.libs.core")
local applyZic = ____core.applyZic
local ____Keyboard = require("config.libs.nativeComponents.Keyboard")
local Keyboard = ____Keyboard.Keyboard
local ____KnobValue = require("config.libs.nativeComponents.KnobValue")
local KnobValue = ____KnobValue.KnobValue
local ____List = require("config.libs.nativeComponents.List")
local List = ____List.List
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
pluginAlias("Tempo", "libzic_Tempo.so")
plugin("Tempo")
local ScreenWidth = 240
local ScreenHeight = 320
pixelController("rpi3A_4enc_11btn")
setWindowPosition(400, 500)
setScreenSize(ScreenWidth, ScreenHeight)
applyZic(React.createElement(
    View,
    {name = "Demo"},
    React.createElement(Text, {fontSize = 16, text = "title", bounds = {0, 0, ScreenWidth, 16}, centered = true}),
    React.createElement(Rect, {color = "tertiary", filled = false, bounds = {10, 30, 100, 50}}),
    React.createElement(Rect, {color = "primary", bounds = {120, 30, 110, 20}}),
    React.createElement(Value, {
        audioPlugin = "Tempo",
        param = "BPM",
        bounds = {120, 60, 110, 20},
        encoderId = 0,
        barColor = "quaternary"
    }),
    React.createElement(KnobValue, {
        audioPlugin = "Tempo",
        param = "BPM",
        bounds = {10, 90, 70, 70},
        encoderId = 1,
        color = "secondary"
    }),
    React.createElement(List, {items = {"item1", "item2", "item3"}, bounds = {120, 100, 110, 60}}),
    React.createElement(Keyboard, {bounds = {0, 175, ScreenWidth, 120}})
))
return ____exports
