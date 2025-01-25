--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____StepEditSample = require("config.libs.components.StepEditSample")
local StepEditSample = ____StepEditSample.StepEditSample
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_kick.ui.components.Common")
local Common = ____Common.Common
local ____TextGridSel = require("config.pixel.240x320_kick.ui.components.TextGridSel")
local TextGridSel = ____TextGridSel.TextGridSel
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local SampleTrack = ____constants.SampleTrack
local ScreenWidth = ____constants.ScreenWidth
function ____exports.SampleEditorView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(StepEditSample, {position = {0, 0, ScreenWidth, 8}, data = "SampleSequencer"}),
        React.createElement(TextGridSel, {items = {
            "Editor",
            "Sample",
            "...",
            "&empty",
            "Seq.",
            "&icon::musicNote::pixelated"
        }, keys = {}, selected = 0, contextValue = 0}),
        React.createElement(Common, {selected = 1, track = SampleTrack, hideSequencer = true})
    )
end
return ____exports
