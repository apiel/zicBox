--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Keymaps = require("config.libs.nativeComponents.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____VisibilityContext = require("config.libs.nativeComponents.VisibilityContext")
local VisibilityContext = ____VisibilityContext.VisibilityContext
local ____TextGrid = require("config.libs.tsComponents.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____constants = require("config.pixel.240x320_drums.ui.constants")
local KeyInfoPosition = ____constants.KeyInfoPosition
function ____exports.TextGridSel(____bindingPattern0)
    local activeBgColor
    local contextValue
    local keys
    local items
    local selected
    selected = ____bindingPattern0.selected
    items = ____bindingPattern0.items
    keys = ____bindingPattern0.keys
    contextValue = ____bindingPattern0.contextValue
    activeBgColor = ____bindingPattern0.activeBgColor
    if selected >= 0 then
        items[selected + 1] = "!" .. items[selected + 1]
    end
    return React.createElement(
        TextGrid,
        {bounds = KeyInfoPosition, rows = {(((items[1] .. " ") .. items[2]) .. " ") .. items[3], (((items[4] .. " ") .. items[5]) .. " ") .. items[6]}, activeBgColor = activeBgColor},
        React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = contextValue}),
        React.createElement(Keymaps, {keys = keys})
    )
end
return ____exports
