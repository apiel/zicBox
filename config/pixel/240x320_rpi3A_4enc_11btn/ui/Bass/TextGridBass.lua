--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Rect = require("config.libs.components.Rect")
local Rect = ____Rect.Rect
local ____TextGridSel = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.components.TextGridSel")
local TextGridSel = ____TextGridSel.TextGridSel
local ____Title = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.components.Title")
local Title = ____Title.Title
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local ColorTrack2 = ____constants.ColorTrack2
function ____exports.TextGridBass(____bindingPattern0)
    local viewName
    local selected
    selected = ____bindingPattern0.selected
    viewName = ____bindingPattern0.viewName
    return React.createElement(
        React.Fragment,
        nil,
        React.createElement(Title, {title = "Bass"}),
        React.createElement(Rect, {position = {70, 28, 6, 6}, color = ColorTrack2}),
        React.createElement(TextGridSel, {
            ITEM_BACKGROUND = ColorTrack2,
            items = {
                "Fx1/Fx2",
                "Env",
                "...",
                "Seq.",
                "Waveform",
                "&icon::musicNote::pixelated"
            },
            keys = {
                {key = "q", action = viewName == "Bass" and "setView:BassDistortion" or "setView:Bass"},
                {key = "w", action = "setView:BassEnv"},
                {key = "shift", action = "contextToggle:254:1:0"},
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
