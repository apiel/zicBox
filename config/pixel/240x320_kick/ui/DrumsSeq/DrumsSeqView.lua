local ____lualib = require("lualib_bundle")
local __TS__ArrayFrom = ____lualib.__TS__ArrayFrom
local ____exports = {}
local React = require("config.libs.react")
local ____Keymaps = require("config.libs.components.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____StepEditMono = require("config.libs.components.StepEditMono")
local StepEditMono = ____StepEditMono.StepEditMono
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
local ____Common = require("config.pixel.240x320_kick.ui.components.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local ColorTrack5 = ____constants.ColorTrack5
local KeyInfoPosition = ____constants.KeyInfoPosition
local SampleTrack = ____constants.SampleTrack
local ScreenWidth = ____constants.ScreenWidth
local function Seq(____bindingPattern0)
    local show_playing_step
    local color
    local encoder
    local track
    local w
    local x
    x = ____bindingPattern0.x
    w = ____bindingPattern0.w
    track = ____bindingPattern0.track
    encoder = ____bindingPattern0.encoder
    color = ____bindingPattern0.color
    show_playing_step = ____bindingPattern0.show_playing_step
    local y = 0
    return __TS__ArrayFrom(
        {length = 32},
        function(____, _, i)
            local yy = y + 12
            y = y + (8 + (i % 4 == 3 and 3 or 0))
            return React.createElement(
                StepEditMono,
                {
                    position = {x, yy, w, 8},
                    data = "Sequencer " .. tostring(i),
                    group = i,
                    playing_color = rgb(35, 161, 35),
                    background_color = (i % 8 == 0 or i % 8 == 1 or i % 8 == 2 or i % 8 == 3) and rgb(42, 54, 56) or "background",
                    selected_color = rgb(76, 94, 97),
                    track = track,
                    encoder = encoder,
                    show_playing_step = show_playing_step,
                    on_color = color
                }
            )
        end
    )
end
function ____exports.DrumsSeqView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    local w = ScreenWidth / 3
    local y = 0
    return React.createElement(
        View,
        {name = name},
        React.createElement(Text, {text = " Sample", position = {2 * w, 0, w, 9}, background_color = ColorTrack5, color = "text"}),
        React.createElement(Seq, {
            x = w * 2,
            w = w,
            track = SampleTrack,
            encoder = 2,
            color = ColorTrack5,
            show_playing_step = true
        }),
        React.createElement(
            TextGrid,
            {position = KeyInfoPosition, rows = {"&icon::arrowUp::filled &empty ...", "&icon::arrowDown::filled Sample &icon::musicNote::pixelated"}},
            React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 0}),
            React.createElement(Keymaps, {keys = {
                {key = "q", action = "incGroup:-1"},
                {key = "e", action = "contextToggle:254:1:0"},
                {key = "a", action = "incGroup:+1"},
                {key = "s", action = "setView:Sample"},
                {key = "d", action = "noteOn:Sample:60"}
            }})
        ),
        React.createElement(Common, {selected = 0, hideSequencer = true, track = SampleTrack})
    )
end
return ____exports
