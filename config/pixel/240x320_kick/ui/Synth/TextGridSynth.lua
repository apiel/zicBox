--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____TextGridSel = require("config.pixel.240x320_kick.ui.components.TextGridSel")
local TextGridSel = ____TextGridSel.TextGridSel
function ____exports.TextGridSynth(____bindingPattern0)
    local viewName
    local selected
    selected = ____bindingPattern0.selected
    viewName = ____bindingPattern0.viewName
    return React.createElement(TextGridSel, {items = {
        "Fx1/Fx2",
        "Waveform",
        "...",
        "Amp/Click",
        "Freq.",
        "&icon::musicNote::pixelated"
    }, keys = {
        {key = "q", action = viewName == "Master" and "setView:Distortion" or "setView:Master"},
        {key = "w", action = "setView:Waveform"},
        {key = "e", action = "contextToggle:254:1:0"},
        {key = "a", action = viewName == "Amplitude" and "setView:Click" or "setView:Amplitude"},
        {key = "s", action = "setView:Frequency"},
        {key = "d", action = "noteOn:Drum23:60"}
    }, selected = selected, contextValue = 0})
end
return ____exports
