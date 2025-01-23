--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Rect = require("config.libs.components.Rect")
local Rect = ____Rect.Rect
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local ColorTrack3 = ____constants.ColorTrack3
local ColorTrack4 = ____constants.ColorTrack4
local ColorTrack5 = ____constants.ColorTrack5
local ____TextGridSel = require("config.pixel.240x320_kick.ui.components.TextGridSel")
local TextGridSel = ____TextGridSel.TextGridSel
local ____Title = require("config.pixel.240x320_kick.ui.components.Title")
local Title = ____Title.Title
function ____exports.TextGridDrums(____bindingPattern0)
    local target
    local viewName
    local selected
    selected = ____bindingPattern0.selected
    viewName = ____bindingPattern0.viewName
    target = ____bindingPattern0.target
    local color = ColorTrack3
    local rectX = 62
    if target == "HiHat" then
        color = ColorTrack4
        rectX = 58
    elseif target == "Sample" then
        color = ColorTrack5
        rectX = 55
    end
    return React.createElement(
        React.Fragment,
        nil,
        React.createElement(Title, {title = target}),
        React.createElement(Rect, {position = {rectX, 28, 6, 6}, color = color}),
        React.createElement(TextGridSel, {
            items = {
                "&empty",
                "Sample",
                "...",
                "&empty",
                "Seq.",
                "&icon::musicNote::pixelated"
            },
            keys = {{key = "w", action = viewName == "Sample" and "setView:Sample2" or "setView:Sample"}, {key = "e", action = "contextToggle:254:1:0"}, {key = "s", action = "setView:DrumsSeq"}, {key = "d", action = ("noteOn:" .. target) .. ":60"}},
            selected = selected,
            contextValue = 0,
            ITEM_BACKGROUND = color
        })
    )
end
return ____exports
