--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Rect = require("config.libs.components.Rect")
local Rect = ____Rect.Rect
local ____Common = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.components.Common")
local Perc = ____Common.Perc
local ____SideInfo = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.components.SideInfo")
local SideInfo = ____SideInfo.SideInfo
local ____TextGridSel = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.components.TextGridSel")
local TextGridSel = ____TextGridSel.TextGridSel
local ____Title = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.components.Title")
local Title = ____Title.Title
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local btn1 = ____constants.btn1
local btn2 = ____constants.btn2
local btn3 = ____constants.btn3
local btn4 = ____constants.btn4
local btn5 = ____constants.btn5
local btn6 = ____constants.btn6
local btn7 = ____constants.btn7
local btn8 = ____constants.btn8
local btnDown = ____constants.btnDown
local btnShift = ____constants.btnShift
local btnUp = ____constants.btnUp
local ColorTrack6 = ____constants.ColorTrack6
function ____exports.TextGridPerc(____bindingPattern0)
    local selected
    selected = ____bindingPattern0.selected
    local viewName = ____bindingPattern0.viewName
    return React.createElement(
        React.Fragment,
        nil,
        React.createElement(Title, {title = Perc}),
        React.createElement(Rect, {position = {70, 28, 6, 6}, color = ColorTrack6}),
        React.createElement(TextGridSel, {
            selectedBackground = ColorTrack6,
            rows = {"Main Fx Env Noise1", "Seq. &empty Fm Noise2"},
            keys = {
                {key = btn1, action = "setView:Perc"},
                {key = btn2, action = "setView:PercFx"},
                {key = btn3, action = "setView:PercEnv"},
                {key = btn4, action = "setView:PercNoise"},
                {key = btn5, action = "setView:PercSeq"},
                {key = btn6, action = "noteOn:Perc:60"},
                {key = btn7, action = "setView:PercFm"},
                {key = btn8, action = "setView:PercNoise2"},
                {key = btnUp, action = "noteOn:Perc:60"},
                {key = btnDown, action = "noteOn:Perc:60"},
                {key = btnShift, action = "contextToggle:254:1:0"}
            },
            selected = selected,
            contextValue = 0
        }),
        React.createElement(SideInfo, {up = "*", down = "&icon::musicNote::pixelated", ctxValue = 0})
    )
end
return ____exports
