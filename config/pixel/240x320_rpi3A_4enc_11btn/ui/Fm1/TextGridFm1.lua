--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Rect = require("config.libs.components.Rect")
local Rect = ____Rect.Rect
local ____Common = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.components.Common")
local Fm1 = ____Common.Fm1
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
local btn5 = ____constants.btn5
local btn6 = ____constants.btn6
local btn7 = ____constants.btn7
local btn8 = ____constants.btn8
local btnDown = ____constants.btnDown
local btnShift = ____constants.btnShift
local btnUp = ____constants.btnUp
local ColorTrack2 = ____constants.ColorTrack2
function ____exports.TextGridFm1(____bindingPattern0)
    local selected
    selected = ____bindingPattern0.selected
    local viewName = ____bindingPattern0.viewName
    return React.createElement(
        React.Fragment,
        nil,
        React.createElement(Title, {title = Fm1}),
        React.createElement(Rect, {bounds = {70, 28, 6, 6}, color = ColorTrack2}),
        React.createElement(TextGridSel, {
            selectedBackground = ColorTrack2,
            rows = {"Main Fx Env &empty", "Seq. &empty &empty &empty"},
            keys = {
                {key = btn1, action = "setView:Fm1"},
                {key = btn2, action = "setView:Fm1Fx"},
                {key = btn3, action = "setView:Fm1Env"},
                {key = btn5, action = "setView:Fm1Seq"},
                {key = btn6, action = "noteOn:FmDrum:60"},
                {key = btn7, action = "noteOn:FmDrum:60"},
                {key = btn8, action = "noteOn:FmDrum:60"},
                {key = btnUp, action = "noteOn:FmDrum:60"},
                {key = btnDown, action = "noteOn:FmDrum:60"},
                {key = btnShift, action = "contextToggle:254:1:0"}
            },
            selected = selected,
            contextValue = 0
        }),
        React.createElement(SideInfo, {up = "*", down = "&icon::musicNote::pixelated", ctxValue = 0})
    )
end
return ____exports
