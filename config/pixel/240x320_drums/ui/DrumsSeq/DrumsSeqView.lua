local ____lualib = require("lualib_bundle")
local __TS__ArrayFrom = ____lualib.__TS__ArrayFrom
local ____exports = {}
local React = require("config.libs.react")
local ____Keymaps = require("config.libs.nativeComponents.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____StepEditMono = require("config.libs.nativeComponents.StepEditMono")
local StepEditMono = ____StepEditMono.StepEditMono
local ____Text = require("config.libs.nativeComponents.Text")
local Text = ____Text.Text
local ____TextGrid = require("config.libs.nativeComponents.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____View = require("config.libs.nativeComponents.View")
local View = ____View.View
local ____VisibilityContext = require("config.libs.nativeComponents.VisibilityContext")
local VisibilityContext = ____VisibilityContext.VisibilityContext
local ____ui = require("config.libs.ui")
local rgb = ____ui.rgb
local ____Common = require("config.pixel.240x320_drums.ui.components.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_drums.ui.constants")
local ColorTrack3 = ____constants.ColorTrack3
local ColorTrack4 = ____constants.ColorTrack4
local ColorTrack5 = ____constants.ColorTrack5
local HiHatTrack = ____constants.HiHatTrack
local KeyInfoPosition = ____constants.KeyInfoPosition
local SampleTrack = ____constants.SampleTrack
local ScreenWidth = ____constants.ScreenWidth
local SnareTrack = ____constants.SnareTrack
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
                    bounds = {x, yy, w, 8},
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
        React.createElement(Text, {text = " Snare", bounds = {0, 0, w, 9}, bgColor = ColorTrack3, color = "text"}),
        React.createElement(Seq, {
            x = 0,
            w = w,
            track = SnareTrack,
            encoder = 0,
            color = ColorTrack3
        }),
        React.createElement(Text, {text = " HiHat", bounds = {w, 0, w, 9}, bgColor = ColorTrack4, color = "text"}),
        React.createElement(Seq, {
            x = w,
            w = w,
            track = HiHatTrack,
            encoder = 1,
            color = ColorTrack4
        }),
        React.createElement(Text, {text = " Sample", bounds = {2 * w, 0, w, 9}, bgColor = ColorTrack5, color = "text"}),
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
            {bounds = KeyInfoPosition, rows = {"&icon::arrowUp::filled &empty ...", "&icon::arrowDown::filled Drums &icon::musicNote::pixelated"}},
            React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 0}),
            React.createElement(Keymaps, {keys = {
                {key = "q", action = "incGroup:-1"},
                {key = "e", action = "contextToggle:254:1:0"},
                {key = "a", action = "incGroup:+1"},
                {key = "s", action = "setView:Snare"},
                {key = "d", action = "noteOn:Snare:60"}
            }})
        ),
        React.createElement(Common, {selected = 0, hideSequencer = true, track = SnareTrack})
    )
end
return ____exports
