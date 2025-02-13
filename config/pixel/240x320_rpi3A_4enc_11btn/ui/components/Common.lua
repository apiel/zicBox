local ____lualib = require("lualib_bundle")
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local ____exports = {}
local React = require("config.libs.react")
local ____SeqProgressBar = require("config.libs.components.SeqProgressBar")
local SeqProgressBar = ____SeqProgressBar.SeqProgressBar
local ____ui = require("config.libs.ui")
local rgb = ____ui.rgb
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local btn1 = ____constants.btn1
local btn2 = ____constants.btn2
local btn3 = ____constants.btn3
local btn4 = ____constants.btn4
local btn5 = ____constants.btn5
local btn6 = ____constants.btn6
local btn8 = ____constants.btn8
local btnDown = ____constants.btnDown
local btnShift = ____constants.btnShift
local btnUp = ____constants.btnUp
local ScreenWidth = ____constants.ScreenWidth
local ____SideInfo = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.components.SideInfo")
local SideInfo = ____SideInfo.SideInfo
local ____TextGridSel = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.components.TextGridSel")
local TextGridSel = ____TextGridSel.TextGridSel
____exports.Kick = "Kick"
____exports.Bass = "Bass"
____exports.Fm1 = "Fm1"
____exports.Synth = "Synth"
____exports.Sample1 = "Sampl1"
____exports.Sample2 = "Sampl2"
____exports.Sample3 = "Sampl3"
____exports.Clips = "Clips"
____exports.Perc = "Perc"
function ____exports.Common(____bindingPattern0)
    local selectedBackground
    local track
    local hideSequencer
    local selected
    selected = ____bindingPattern0.selected
    hideSequencer = ____bindingPattern0.hideSequencer
    track = ____bindingPattern0.track
    selectedBackground = ____bindingPattern0.selectedBackground
    return React.createElement(
        React.Fragment,
        nil,
        not hideSequencer and React.createElement(
            SeqProgressBar,
            {
                position = {0, 0, ScreenWidth, 5},
                seq_plugin = "Sequencer",
                active_color = rgb(35, 161, 35),
                selection_color = rgb(35, 161, 35),
                foreground_color = rgb(34, 110, 34),
                volume_plugin = "TrackFx VOLUME",
                show_steps = true,
                track = track
            }
        ),
        React.createElement(
            TextGridSel,
            __TS__ObjectAssign({rows = {(((((____exports.Kick .. " ") .. ____exports.Bass) .. " ") .. ____exports.Fm1) .. " ") .. ____exports.Synth, (((((____exports.Sample1 .. " ") .. ____exports.Perc) .. " ") .. ____exports.Sample3) .. " ") .. ____exports.Clips}, keys = {
                {key = btn1, action = "setView:Drum23"},
                {key = btn2, action = "setView:Bass"},
                {key = btn3, action = "setView:Fm1"},
                {key = btn4, action = "setView:Synth"},
                {key = btn5, action = "setView:Sample"},
                {key = btn6, action = "setView:Perc"},
                {key = btn8, action = "setView:Clips"},
                {key = btnUp, action = "setView:Menu"},
                {key = btnDown, action = "playPause"},
                {key = btnShift, action = "contextToggle:254:1:0"}
            }, selected = selected, contextValue = 1}, selectedBackground and ({selectedBackground = selectedBackground}))
        ),
        React.createElement(SideInfo, {up = "*", down = "&icon::play::filled", ctxValue = 1})
    )
end
return ____exports
