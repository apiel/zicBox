--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____View = require("config.libs.nativeComponents.View")
local View = ____View.View
local ____TextGrid = require("config.libs.tsComponents.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____Common = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.components.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local BassTrack = ____constants.BassTrack
local btn2 = ____constants.btn2
local btn5 = ____constants.btn5
local btn6 = ____constants.btn6
local btn7 = ____constants.btn7
local btnShift = ____constants.btnShift
local ColorTrack2 = ____constants.ColorTrack2
local KeyInfoPosition = ____constants.KeyInfoPosition
function ____exports.BassSeqView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    local y = 0
    return React.createElement(
        View,
        {name = name},
        React.createElement(TextGrid, {
            bounds = KeyInfoPosition,
            rows = {"&icon::toggle::rect &icon::arrowUp::filled ...", "Seq./Bass &icon::arrowDown::filled &icon::musicNote::pixelated"},
            selectedBackground = ColorTrack2,
            contextValue = 0,
            keys = {
                {key = btn2, action = "incGroup:-1"},
                {key = btnShift, action = "contextToggle:254:1:0"},
                {key = btn5, action = "setView:Bass"},
                {key = btn6, action = "incGroup:+1"},
                {key = btn7, action = "noteOn:Bass:60"}
            }
        }),
        React.createElement(Common, {selected = "Bass", hideSequencer = true, track = BassTrack, selectedBackground = ColorTrack2})
    )
end
return ____exports
