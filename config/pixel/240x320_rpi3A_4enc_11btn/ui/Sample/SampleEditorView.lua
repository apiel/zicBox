local ____lualib = require("lualib_bundle")
local __TS__ArrayFrom = ____lualib.__TS__ArrayFrom
local ____exports = {}
local React = require("config.libs.react")
local ____Keymaps = require("config.libs.components.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____StepEditSample = require("config.libs.components.StepEditSample")
local StepEditSample = ____StepEditSample.StepEditSample
local ____Text = require("config.libs.components.Text")
local Text = ____Text.Text
local ____TextGrid = require("config.libs.components.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____VisibilityContext = require("config.libs.components.VisibilityContext")
local VisibilityContext = ____VisibilityContext.VisibilityContext
local ____ui = require("config.libs.ui")
local rgb = ____ui.rgb
local ____Common = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.components.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local btnShift = ____constants.btnShift
local ColorTrack3 = ____constants.ColorTrack3
local KeyInfoPosition = ____constants.KeyInfoPosition
local SampleTrack = ____constants.SampleTrack
local ScreenWidth = ____constants.ScreenWidth
function ____exports.SampleEditorView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    local y = 0
    return React.createElement(
        View,
        {name = name},
        React.createElement(Text, {text = "Vel.", position = {0, 0, 25, 9}}),
        React.createElement(Text, {text = "Sample", position = {60, 0, 100, 9}}),
        React.createElement(Text, {text = "Start", position = {ScreenWidth - 84, 0, 50, 9}}),
        React.createElement(Text, {text = "End", position = {ScreenWidth - 38, 0, 50, 9}}),
        __TS__ArrayFrom(
            {length = 32},
            function(____, _, i)
                local yy = y + 12
                y = y + (8 + (i % 4 == 3 and 3 or 0))
                return React.createElement(
                    StepEditSample,
                    {
                        position = {0, yy, ScreenWidth, 8},
                        data = "SampleSequencer " .. tostring(i),
                        track = SampleTrack,
                        group = i,
                        playing_color = rgb(35, 161, 35),
                        background_color = (i % 8 == 0 or i % 8 == 1 or i % 8 == 2 or i % 8 == 3) and rgb(42, 54, 56) or "background",
                        selected_color = rgb(76, 94, 97)
                    },
                    React.createElement(Keymaps, {keys = {{key = "q", action = ".toggle"}, {key = "d", action = ".play"}}})
                )
            end
        ),
        React.createElement(
            TextGrid,
            {position = KeyInfoPosition, rows = {"&icon::toggle::rect &icon::arrowUp::filled ...", "&empty &icon::arrowDown::filled &icon::musicNote::pixelated"}},
            React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 0}),
            React.createElement(Keymaps, {keys = {{key = btnShift, action = "contextToggle:254:1:0"}, {key = "w", action = "incGroup:-1"}, {key = "s", action = "incGroup:+1"}}})
        ),
        React.createElement(Common, {selected = 1, track = SampleTrack, hideSequencer = true, selectedBackground = ColorTrack3})
    )
end
return ____exports
