local ____lualib = require("lualib_bundle")
local __TS__ArrayFrom = ____lualib.__TS__ArrayFrom
local ____exports = {}
local React = require("config.libs.react")
local ____Keymaps = require("config.libs.nativeComponents.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____StepEditDrum = require("config.libs.nativeComponents.StepEditDrum")
local StepEditDrum = ____StepEditDrum.StepEditDrum
local ____View = require("config.libs.nativeComponents.View")
local View = ____View.View
local ____TextGrid = require("config.libs.tsComponents.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____ui = require("config.libs.ui")
local rgb = ____ui.rgb
local ____Common = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.components.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local btn1 = ____constants.btn1
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
function ____exports.PercSeqView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    local y = 0
    return React.createElement(
        View,
        {name = name},
        __TS__ArrayFrom(
            {length = 32},
            function(____, _, i)
                local yy = y + 5
                y = y + (8 + (i % 4 == 3 and 4 or 0))
                return React.createElement(
                    StepEditDrum,
                    {
                        bounds = {0, yy, ScreenWidth, 8},
                        data = "Sequencer " .. tostring(i),
                        track = PercTrack,
                        group = i,
                        playing_color = rgb(35, 161, 35),
                        background_color = (i % 8 == 0 or i % 8 == 1 or i % 8 == 2 or i % 8 == 3) and rgb(42, 54, 56) or "background",
                        selected_color = rgb(76, 94, 97)
                    },
                    React.createElement(Keymaps, {keys = {{key = btn1, action = ".toggle"}}})
                )
            end
        ),
        React.createElement(TextGrid, {
            bounds = KeyInfoPosition,
            rows = {"&icon::toggle::rect &icon::arrowUp::filled ...", "Seq./Perc &icon::arrowDown::filled &icon::musicNote::pixelated"},
            selectedBackground = ColorTrack3,
            contextValue = 0,
            keys = {
                {key = btn2, action = "incGroup:-1"},
                {key = btnShift, action = "contextToggle:254:1:0"},
                {key = btn5, action = "setView:Perc"},
                {key = btn6, action = "incGroup:+1"},
                {key = btn7, action = "noteOn:Perc:60"}
            }
        }),
        React.createElement(Common, {selected = "Perc", hideSequencer = true, track = PercTrack, selectedBackground = ColorTrack6})
    )
end
return ____exports
