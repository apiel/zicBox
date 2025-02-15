local ____lualib = require("lualib_bundle")
local __TS__ArrayFrom = ____lualib.__TS__ArrayFrom
local ____exports = {}
local React = require("config.libs.react")
local ____StepEditDrum = require("config.libs.components.StepEditDrum")
local StepEditDrum = ____StepEditDrum.StepEditDrum
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____ui = require("config.libs.ui")
local rgb = ____ui.rgb
local ____Common = require("config.pixel.240x320_drums.ui.components.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_drums.ui.constants")
local Drum23Track = ____constants.Drum23Track
local ScreenWidth = ____constants.ScreenWidth
local ____TextGridSeq = require("config.pixel.240x320_drums.ui.Drum23.TextGridSeq")
local TextGridSeq = ____TextGridSeq.TextGridSeq
function ____exports.Drum23SeqView(____bindingPattern0)
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
                        group = i,
                        playing_color = rgb(35, 161, 35),
                        background_color = (i % 8 == 0 or i % 8 == 1 or i % 8 == 2 or i % 8 == 3) and rgb(42, 54, 56) or "background",
                        selected_color = rgb(76, 94, 97)
                    }
                )
            end
        ),
        React.createElement(TextGridSeq, nil),
        React.createElement(Common, {selected = 0, hideSequencer = true, track = Drum23Track})
    )
end
return ____exports
