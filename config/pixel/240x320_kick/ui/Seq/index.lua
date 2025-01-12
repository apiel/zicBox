local ____lualib = require("lualib_bundle")
local __TS__ArrayFrom = ____lualib.__TS__ArrayFrom
local ____exports = {}
local React = require("config.libs.react")
local ____StepEdit2 = require("config.libs.components.StepEdit2")
local StepEdit2 = ____StepEdit2.StepEdit2
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____ui = require("config.libs.ui")
local rgb = ____ui.rgb
local ____Common = require("config.pixel.240x320_kick.ui.components.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local ScreenWidth = ____constants.ScreenWidth
local ____TextGridSeq = require("config.pixel.240x320_kick.ui.Seq.TextGridSeq")
local TextGridSeq = ____TextGridSeq.TextGridSeq
function ____exports.SeqView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    local y = 0
    return React.createElement(
        View,
        {name = name},
        __TS__ArrayFrom(
            {length = 32},
            function(____, _, i)
                local yy = y
                y = y + (8 + (i % 4 == 3 and 3 or 0))
                return React.createElement(
                    StepEdit2,
                    {
                        position = {0, yy, ScreenWidth, 8},
                        data = "Sequencer " .. tostring(i),
                        group = i,
                        playing_color = rgb(35, 161, 35),
                        background_color = (i % 8 == 0 or i % 8 == 1 or i % 8 == 2 or i % 8 == 3) and rgb(42, 54, 56) or "background",
                        SELECTED_COLOR = rgb(68, 85, 88)
                    }
                )
            end
        ),
        React.createElement(TextGridSeq, nil),
        React.createElement(Common, {selected = 0, hideSpectrogram = true, hideSequencer = true})
    )
end
return ____exports
