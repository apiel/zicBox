--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____TextGridSel = require("config.pixel.240x320_kick.ui.components.TextGridSel")
local TextGridSel = ____TextGridSel.TextGridSel
local ____Title = require("config.pixel.240x320_kick.ui.components.Title")
local Title = ____Title.Title
function ____exports.TextGridSnare(____bindingPattern0)
    local viewName
    local selected
    selected = ____bindingPattern0.selected
    viewName = ____bindingPattern0.viewName
    return React.createElement(
        React.Fragment,
        nil,
        React.createElement(Title, {title = "Snare"}),
        React.createElement(TextGridSel, {items = {
            "Snare",
            "Seq.",
            "...",
            "Amp/Click",
            "Freq.",
            "&icon::musicNote::pixelated"
        }, keys = {
            {key = "q", action = viewName == "Snare" and "setView:Snare2" or "setView:Snare"},
            {key = "w", action = "setView:Snare2"},
            {key = "e", action = "contextToggle:254:1:0"},
            {key = "a", action = viewName == "Amplitude" and "setView:Click" or "setView:Amplitude"},
            {key = "s", action = "setView:Frequency"},
            {key = "d", action = "noteOn:Snare:60"}
        }, selected = selected, contextValue = 0})
    )
end
return ____exports