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
local btn5 = ____constants.btn5
local btn6 = ____constants.btn6
local btnDown = ____constants.btnDown
local btnShift = ____constants.btnShift
local ScreenWidth = ____constants.ScreenWidth
local ____TextGridSel = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.components.TextGridSel")
local TextGridSel = ____TextGridSel.TextGridSel
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
            __TS__ObjectAssign({items = {
                "Kick",
                "Sample",
                "^...",
                "Bass",
                "Clips",
                "&icon::play::filled"
            }, keys = {
                {key = btn1, action = "setView:Drum23"},
                {key = btn2, action = "setView:Sample"},
                {key = btn5, action = "setView:Bass"},
                {key = btn6, action = "setView:Clips"},
                {key = btnShift, action = "contextToggle:254:1:0"},
                {key = btnDown, action = "playPause"}
            }, selected = selected, contextValue = 1}, selectedBackground and ({ITEM_BACKGROUND = selectedBackground}))
        )
    )
end
return ____exports
