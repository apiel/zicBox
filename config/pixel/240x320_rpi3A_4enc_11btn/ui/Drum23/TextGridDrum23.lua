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
local btn1 = ____constants.btn1
local btn2 = ____constants.btn2
local btn5 = ____constants.btn5
local btn6 = ____constants.btn6
local btn7 = ____constants.btn7
local btnShift = ____constants.btnShift
local ColorTrack1 = ____constants.ColorTrack1
function ____exports.TextGridDrum23(____bindingPattern0)
    local viewName
    local selected
    selected = ____bindingPattern0.selected
    viewName = ____bindingPattern0.viewName
    return React.createElement(
        React.Fragment,
        nil,
        React.createElement(Title, {title = "Kick"}),
        React.createElement(Rect, {position = {70, 28, 6, 6}, color = ColorTrack1}),
        React.createElement(TextGridSel, {
            ITEM_BACKGROUND = ColorTrack1,
            items = {
                "Fx1/Fx2",
                "Wave/Freq",
                "...",
                "Seq.",
                "Amp/Click",
                "&icon::musicNote::pixelated"
            },
            keys = {
                {key = btn1, action = viewName == "Drum23" and "setView:Distortion" or "setView:Drum23"},
                {key = btn2, action = viewName == "Waveform" and "setView:Frequency" or "setView:Waveform"},
                {key = btnShift, action = "contextToggle:254:1:0"},
                {key = btn5, action = "setView:Sequencer"},
                {key = btn6, action = viewName == "Amplitude" and "setView:Click" or "setView:Amplitude"},
                {key = btn7, action = "noteOn:Drum23:60"}
            },
            selected = selected,
            contextValue = 0
        })
    )
end
return ____exports
