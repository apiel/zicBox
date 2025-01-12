local ____lualib = require("lualib_bundle")
local __TS__ArrayFrom = ____lualib.__TS__ArrayFrom
local ____exports = {}
local React = require("config.libs.react")
local ____StepEdit2 = require("config.libs.components.StepEdit2")
local StepEdit2 = ____StepEdit2.StepEdit2
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_kick.ui.components.Common")
local Common = ____Common.Common
local ____TextGridSeq = require("config.pixel.240x320_kick.ui.Seq.TextGridSeq")
local TextGridSeq = ____TextGridSeq.TextGridSeq
function ____exports.SeqView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        __TS__ArrayFrom(
            {length = 32},
            function(____, _, i) return React.createElement(
                StepEdit2,
                {
                    position = {0, i * 8, 0, 0},
                    data = "Sequencer GET_STEP " .. tostring(i),
                    group = i
                }
            ) end
        ),
        React.createElement(TextGridSeq, nil),
        React.createElement(Common, {selected = 0, hideSpectrogram = true, hideSequencer = true})
    )
end
return ____exports
