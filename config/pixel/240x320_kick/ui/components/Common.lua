local ____lualib = require("lualib_bundle")
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local ____exports = {}
local React = require("config.libs.react")
local ____SeqProgressBar = require("config.libs.components.SeqProgressBar")
local SeqProgressBar = ____SeqProgressBar.SeqProgressBar
local ____ui = require("config.libs.ui")
local rgb = ____ui.rgb
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local ScreenWidth = ____constants.ScreenWidth
local ____TextGridSel = require("config.pixel.240x320_kick.ui.components.TextGridSel")
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
                "Sequencer",
                "Clips",
                "&icon::play::filled"
            }, keys = {
                {key = "q", action = "setView:Drum23"},
                {key = "w", action = "setView:Sample"},
                {key = "e", action = "contextToggle:254:1:0"},
                {key = "a", action = "setView:Sequencer"},
                {key = "s", action = "setView:Clips"},
                {key = "d", action = "playPause"}
            }, selected = selected, contextValue = 1}, selectedBackground and ({ITEM_BACKGROUND = selectedBackground}))
        )
    )
end
return ____exports
