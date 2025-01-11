--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____TextGridSel = require("config.pixel.240x320_kick.ui.TextGridSel")
local TextGridSel = ____TextGridSel.TextGridSel
function ____exports.TextGridCommon(____bindingPattern0)
    local selected
    selected = ____bindingPattern0.selected
    return React.createElement(TextGridSel, {items = {
        "Page1",
        "Page2",
        "^...",
        "Seq.",
        "Menu",
        "&icon::play::filled"
    }, keys = {
        {key = "q", action = "setView:Page1"},
        {key = "w", action = "setView:Page2"},
        {key = "e", action = "contextToggle:254:1:0"},
        {key = "a", action = "setView:Sequencer"},
        {key = "s", action = "setView:Menu"},
        {key = "d", action = "playPause"}
    }, selected = selected, contextValue = 1})
end
return ____exports
