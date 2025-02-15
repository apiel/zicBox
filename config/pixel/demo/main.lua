--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____audio = require("config.libs.audio")
local plugin = ____audio.plugin
local pluginAlias = ____audio.pluginAlias
local ____Text = require("config.libs.components.Text")
local Text = ____Text.Text
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____core = require("config.libs.core")
local applyZic = ____core.applyZic
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
    React.createElement(Text, {fontSize = 16, text = "title", position = {0, 20, ScreenWidth, 16}, centered = true}),
    React.createElement(Text, {text = "hello world", position = {5, 5, ScreenWidth, 16}, color = "secondary"})
))
return ____exports
