local ____lualib = require("lualib_bundle")
local __TS__ArrayFrom = ____lualib.__TS__ArrayFrom
local ____exports = {}
local React = require("config.libs.react")
local ____StepEditSample = require("config.libs.components.StepEditSample")
local StepEditSample = ____StepEditSample.StepEditSample
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____ui = require("config.libs.ui")
local rgb = ____ui.rgb
local ____Common = require("config.pixel.240x320_kick.ui.components.Common")
local Common = ____Common.Common
local ____TextGridSel = require("config.pixel.240x320_kick.ui.components.TextGridSel")
local TextGridSel = ____TextGridSel.TextGridSel
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local SampleTrack = ____constants.SampleTrack
local ScreenWidth = ____constants.ScreenWidth
function ____exports.SampleEditorView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    local y = 0
    return React.createElement(
        View,
        {name = name},
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
                    }
                )
            end
        ),
        React.createElement(TextGridSel, {items = {
            "Editor",
            "Sample",
            "...",
            "&empty",
            "Seq.",
            "&icon::musicNote::pixelated"
        }, keys = {}, selected = 0, contextValue = 0}),
        React.createElement(Common, {selected = 1, track = SampleTrack, hideSequencer = true})
    )
end
return ____exports
