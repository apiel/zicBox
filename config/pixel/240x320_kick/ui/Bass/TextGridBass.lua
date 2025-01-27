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
local ColorTrack3 = ____constants.ColorTrack3
function ____exports.TextGridBass(____bindingPattern0)
    local selected
    selected = ____bindingPattern0.selected
    local viewName = ____bindingPattern0.viewName
    return React.createElement(
        React.Fragment,
        nil,
        React.createElement(Title, {title = "Bass"}),
        React.createElement(Rect, {position = {70, 28, 6, 6}, color = ColorTrack3}),
        React.createElement(TextGridSel, {
            ITEM_BACKGROUND = ColorTrack3,
            items = {
                "Main",
                "Env",
                "...",
                "Seq.",
                "Waveform",
                "&icon::musicNote::pixelated"
            },
            keys = {
                {key = "q", action = "setView:Bass"},
                {key = "w", action = "setView:BassEnv"},
                {key = "e", action = "contextToggle:254:1:0"},
                {key = "a", action = "setView:BassSeq"},
                {key = "s", action = "setView:BassWaveform"},
                {key = "d", action = "noteOn:Bass:60"}
            },
            selected = selected,
            contextValue = 0
        })
    )
end
return ____exports
