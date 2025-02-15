--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____audio = require("config.libs.audio")
local plugin = ____audio.plugin
local pluginAlias = ____audio.pluginAlias
local ____core = require("config.libs.core")
local applyZic = ____core.applyZic
local ____KnobValue = require("config.libs.nativeComponents.KnobValue")
local KnobValue = ____KnobValue.KnobValue
local ____Rect = require("config.libs.nativeComponents.Rect")
local Rect = ____Rect.Rect
local ____Text = require("config.libs.nativeComponents.Text")
local Text = ____Text.Text
local ____Value = require("config.libs.nativeComponents.Value")
local Value = ____Value.Value
local ____View = require("config.libs.nativeComponents.View")
local View = ____View.View
local ____TextGrid = require("config.libs.tsComponents.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____ui = require("config.libs.ui")
local setScreenSize = ____ui.setScreenSize
local setWindowPosition = ____ui.setWindowPosition
pluginAlias("AudioOutput", "libzic_AudioOutputPulse.so")
pluginAlias("Bass", "libzic_SynthBass.so")
pluginAlias("Tempo", "libzic_Tempo.so")
plugin("Bass")
plugin("AudioOutput")
plugin("Tempo")
local ScreenWidth = 240
local ScreenHeight = 320
setWindowPosition(400, 500)
setScreenSize(ScreenWidth, ScreenHeight)
applyZic(React.createElement(
    View,
    {name = "Demo"},
    React.createElement(Text, {fontSize = 16, text = "title", bounds = {0, 20, ScreenWidth, 16}, centered = true}),
    React.createElement(Text, {text = "hello world", bounds = {5, 5, ScreenWidth, 16}, color = "secondary"}),
    React.createElement(TextGrid, {bounds = {0, 50, ScreenWidth, 20}, rows = {"Hello !world &empty", "^123 abc &icon::arrowRight::filled"}}),
    React.createElement(Rect, {color = "secondary", filled = false, bounds = {10, 100, 100, 100}}),
    React.createElement(Rect, {color = "primary", bounds = {120, 100, 100, 50}}),
    React.createElement(KnobValue, {
        audioPlugin = "Tempo",
        param = "BPM",
        bounds = {0, 150, 50, 50},
        encoderId = 0,
        color = "secondary",
        track = 0
    }),
    React.createElement(Value, {
        audioPlugin = "Tempo",
        param = "BPM",
        bounds = {50, 150, 50, 20},
        encoderId = 0,
        track = 0
    })
))
return ____exports
