--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____SampleEditor = require("config.libs.components.SampleEditor")
local SampleEditor = ____SampleEditor.SampleEditor
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_kick.ui.components.Common")
local Common = ____Common.Common
local ____TextGridDrums = require("config.pixel.240x320_kick.ui.components.TextGridDrums")
local TextGridDrums = ____TextGridDrums.TextGridDrums
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local SampleTrack = ____constants.SampleTrack
local ScreenWidth = ____constants.ScreenWidth
function ____exports.SampleEditorView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(SampleEditor, {position = {0, 80, ScreenWidth, 100}, track = SampleTrack}),
        React.createElement(TextGridDrums, {selected = 1, viewName = name, target = "Sample"}),
        React.createElement(Common, {selected = 1, track = SampleTrack})
    )
end
return ____exports
