--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Rect = require("config.libs.components.Rect")
local Rect = ____Rect.Rect
local ____TextGridSel = require("config.pixel.240x320_kick.ui.components.TextGridSel")
local TextGridSel = ____TextGridSel.TextGridSel
local ____Title = require("config.pixel.240x320_kick.ui.components.Title")
local Title = ____Title.Title
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local ColorTrack2 = ____constants.ColorTrack2
function ____exports.TextGridFm(____bindingPattern0)
    local viewName
    local selected
    selected = ____bindingPattern0.selected
    viewName = ____bindingPattern0.viewName
    return React.createElement(
        React.Fragment,
        nil,
        React.createElement(Title, {title = "Fm"}),
        React.createElement(Rect, {position = {85, 28, 6, 6}, color = ColorTrack2}),
        React.createElement(TextGridSel, {items = {
            "Snare",
            "Fm",
            "...",
            "HiHat",
            "Seq.",
            "&icon::musicNote::pixelated"
        }, keys = {
            {key = "q", action = viewName == "Snare" and "setView:Snare2" or "setView:Snare"},
            {key = "w", action = viewName == "Fm" and "setView:Fm2" or "setView:Fm"},
            {key = "e", action = "contextToggle:254:1:0"},
            {key = "a", action = viewName == "HiHat" and "setView:HiHat2" or "setView:HiHat"},
            {key = "d", action = "noteOn:FmDrum:60"}
        }, selected = selected, contextValue = 0})
    )
end
return ____exports
