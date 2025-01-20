--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____TextGridSel = require("config.pixel.240x320_kick.ui.components.TextGridSel")
local TextGridSel = ____TextGridSel.TextGridSel
local ____Title = require("config.pixel.240x320_kick.ui.components.Title")
local Title = ____Title.Title
function ____exports.TextGridDrums(____bindingPattern0)
    local title
    local viewName
    local selected
    selected = ____bindingPattern0.selected
    viewName = ____bindingPattern0.viewName
    title = ____bindingPattern0.title
    return React.createElement(
        React.Fragment,
        nil,
        React.createElement(Title, {title = title}),
        React.createElement(TextGridSel, {items = {
            "Snare",
            "Perc",
            "...",
            "HiHat",
            "Seq.",
            "&icon::musicNote::pixelated"
        }, keys = {
            {key = "q", action = viewName == "Snare" and "setView:Snare2" or "setView:Snare"},
            {key = "w", action = viewName == "Perc" and "setView:Perc2" or "setView:Perc"},
            {key = "e", action = "contextToggle:254:1:0"},
            {key = "a", action = viewName == "HiHat" and "setView:HiHat2" or "setView:HiHat"},
            {key = "d", action = "noteOn:HiHat:60"}
        }, selected = selected, contextValue = 0})
    )
end
return ____exports
