--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Rect = require("config.libs.nativeComponents.Rect")
local Rect = ____Rect.Rect
local ____TextGrid = require("config.libs.tsComponents.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____Common = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.components.Common")
local Bass = ____Common.Bass
local ____SideInfo = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.components.SideInfo")
local SideInfo = ____SideInfo.SideInfo
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
local ColorTrack2 = ____constants.ColorTrack2
local KeyInfoPosition = ____constants.KeyInfoPosition
function ____exports.TextGridBass(____bindingPattern0)
    local selected
    selected = ____bindingPattern0.selected
    local viewName = ____bindingPattern0.viewName
    return React.createElement(
        React.Fragment,
        nil,
        React.createElement(Title, {title = Bass}),
        React.createElement(Rect, {bounds = {70, 28, 6, 6}, color = ColorTrack2}),
        React.createElement(TextGrid, {
            bounds = KeyInfoPosition,
            selectedBackground = ColorTrack2,
            rows = {"Main Fx Env Wave", "Seq. &empty &empty &empty"},
            keys = {
                {key = btn1, action = "setView:Bass"},
                {key = btn2, action = "setView:BassFx"},
                {key = btn3, action = "setView:BassEnv"},
                {key = btn4, action = "setView:BassWaveform"},
                {key = btn5, action = "setView:BassSeq"},
                {key = btn6, action = "noteOn:Bass:60"},
                {key = btn7, action = "noteOn:Bass:60"},
                {key = btn8, action = "noteOn:Bass:60"},
                {key = btnUp, action = "noteOn:Bass:60"},
                {key = btnDown, action = "noteOn:Bass:60"},
                {key = btnShift, action = "contextToggle:254:1:0"}
            },
            selected = selected,
            contextValue = 0
        }),
        React.createElement(SideInfo, {up = "*", down = "&icon::musicNote::pixelated", ctxValue = 0})
    )
end
return ____exports
