--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Sequencer = require("config.libs.nativeComponents.Sequencer")
local Sequencer = ____Sequencer.Sequencer
local ____View = require("config.libs.nativeComponents.View")
local View = ____View.View
local ____TextGrid = require("config.libs.tsComponents.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____Common = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.components.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local btn2 = ____constants.btn2
local btn5 = ____constants.btn5
local btn6 = ____constants.btn6
local btn7 = ____constants.btn7
local btnShift = ____constants.btnShift
local ColorTrack3 = ____constants.ColorTrack3
local ColorTrack6 = ____constants.ColorTrack6
local KeyInfoPosition = ____constants.KeyInfoPosition
local PercTrack = ____constants.PercTrack
local ScreenWidth = ____constants.ScreenWidth
function ____exports.PercSeq2View(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(Sequencer, {bounds = {0, 0, ScreenWidth, 250}, track = PercTrack, audioPlugin = "Sequencer"}),
        React.createElement(TextGrid, {
            bounds = KeyInfoPosition,
            rows = {"&icon::toggle::rect &icon::arrowUp::filled ...", "Seq./Perc &icon::arrowDown::filled &icon::musicNote::pixelated"},
            selectedBackground = ColorTrack3,
            keys = {
                {key = btn2, action = "incGroup:-1"},
                {key = btnShift, action = "contextToggle:254:1:0"},
                {key = btn5, action = "setView:Perc"},
                {key = btn6, action = "incGroup:+1"},
                {key = btn7, action = "noteOn:Perc:60"}
            },
            contextValue = 0
        }),
        React.createElement(Common, {selected = "Perc", hideSequencer = true, track = PercTrack, selectedBackground = ColorTrack6})
    )
end
return ____exports
