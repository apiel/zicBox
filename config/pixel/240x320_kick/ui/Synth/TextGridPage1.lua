--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____TextGridSel = require("config.pixel.240x320_kick.ui.TextGridSel")
local TextGridSel = ____TextGridSel.TextGridSel
function ____exports.TextGridPage1(____bindingPattern0)
    local selected
    selected = ____bindingPattern0.selected
    return React.createElement(TextGridSel, {items = {
        "Fx1/Fx2",
        "Waveform",
        "...",
        "Amp/Click",
        "Freq.",
        "&icon::musicNote::pixelated"
    }, keys = {
        {key = "q", action = "setView:Page1"},
        {key = "w", action = "setView:Distortion"},
        {key = "e", action = "contextToggle:254:1:0"},
        {key = "a", action = "setView:Waveform"},
        {key = "s", action = "setView:Frequency"},
        {key = "d", action = "noteOn:Drum23:60"}
    }, selected = selected, contextValue = 0})
end
return ____exports
