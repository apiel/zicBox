--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____TextGridSel = require("config.pixel.240x320_kick.ui.TextGridSel")
local TextGridSel = ____TextGridSel.TextGridSel
function ____exports.TextGridPage2(____bindingPattern0)
    local selected
    selected = ____bindingPattern0.selected
    return React.createElement(TextGridSel, {items = {
        "Amp.",
        "Click",
        "...",
        "&empty",
        "&empty",
        "&icon::musicNote::pixelated"
    }, keys = {{key = "q", action = "setView:Page2"}, {key = "w", action = "setView:Click"}, {key = "e", action = "contextToggle:254:1:0"}, {key = "d", action = "noteOn:Drum23:60"}}, selected = selected, contextValue = 0})
end
return ____exports
