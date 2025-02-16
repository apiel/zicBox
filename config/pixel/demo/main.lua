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
    React.createElement(Text, {text = "hello world", bounds = {5, 5, ScreenWidth, 16}, color = "secondary"}),
    React.createElement(Text, {fontSize = 16, text = "title", bounds = {0, 20, ScreenWidth, 16}, centered = true}),
    React.createElement(Rect, {color = "tertiary", filled = false, bounds = {10, 100, 100, 100}}),
    React.createElement(Rect, {color = "primary", bounds = {120, 100, 100, 50}}),
    React.createElement(Value, {
        audioPlugin = "Tempo",
        param = "BPM",
        bounds = {120, 160, 100, 20},
        encoderId = 0,
        barColor = "quaternary"
    }),
    React.createElement(KnobValue, {
        audioPlugin = "Tempo",
        param = "BPM",
        bounds = {10, 210, 80, 80},
        encoderId = 1,
        color = "secondary"
    }),
    React.createElement(List, {items = {"item1", "item2", "item3"}, bounds = {120, 210, 110, 80}})
))
return ____exports
