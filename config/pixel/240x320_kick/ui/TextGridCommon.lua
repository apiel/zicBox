--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Keymaps = require("config.libs.components.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____TextGrid = require("config.libs.components.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____VisibilityContext = require("config.libs.components.VisibilityContext")
local VisibilityContext = ____VisibilityContext.VisibilityContext
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local KeyInfoPosition = ____constants.KeyInfoPosition
do
end
function ____exports.TextGridCommon(____bindingPattern0)
    local selected
    selected = ____bindingPattern0.selected
    local items = {
        "Page1",
        "Page2",
        "^...",
        "Seq.",
        "Menu",
        "&icon::play::filled"
    }
    if selected >= 0 then
        items[selected + 1] = "^" .. items[selected + 1]
    end
    return React.createElement(
        TextGrid,
        {position = KeyInfoPosition, rows = {(((items[1] .. " ") .. items[2]) .. " ") .. items[3], (((items[4] .. " ") .. items[5]) .. " ") .. items[6]}},
        React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 1}),
        React.createElement(Keymaps, {keys = {
            {key = "q", action = "setView:Page1"},
            {key = "w", action = "setView:Page2"},
            {key = "e", action = "contextToggle:254:1:0"},
            {key = "a", action = "setView:Seq"},
            {key = "s", action = "setView:Menu"},
            {key = "d", action = "playPause"}
        }})
    )
end
return ____exports
