local ____lualib = require("lualib_bundle")
local __TS__ArrayFrom = ____lualib.__TS__ArrayFrom
local ____exports = {}
local React = require("config.libs.react")
local ____Keymaps = require("config.libs.components.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____StepEditSmall = require("config.libs.components.StepEditSmall")
local StepEditSmall = ____StepEditSmall.StepEditSmall
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
local Drum23Track = ____constants.Drum23Track
local KeyInfoPosition = ____constants.KeyInfoPosition
local ScreenWidth = ____constants.ScreenWidth
function ____exports.DrumsSeqView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    local w = ScreenWidth / 3
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
                    StepEditSmall,
                    {
                        position = {0, yy, w, 8},
                        data = "Sequencer " .. tostring(i),
                        group = i,
                        playing_color = rgb(35, 161, 35),
                        background_color = (i % 8 == 0 or i % 8 == 1 or i % 8 == 2 or i % 8 == 3) and rgb(42, 54, 56) or "background",
                        selected_color = rgb(76, 94, 97)
                    }
                )
            end
        ),
        React.createElement(
            TextGrid,
            {position = KeyInfoPosition, rows = {"&icon::arrowUp::filled &icon::toggle::rect ...", "&icon::arrowDown::filled Synth &icon::musicNote::pixelated"}},
            React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 0}),
            React.createElement(Keymaps, {keys = {
                {key = "q", action = "incGroup:-1"},
                {key = "e", action = "contextToggle:254:1:0"},
                {key = "a", action = "incGroup:+1"},
                {key = "s", action = "setView:Drum23"},
                {key = "d", action = "noteOn:Drum23:60"}
            }})
        ),
        React.createElement(Common, {selected = 0, hideSequencer = true, track = Drum23Track})
    )
end
return ____exports
