--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____DrumEnvelop = require("config.libs.components.DrumEnvelop")
local DrumEnvelop = ____DrumEnvelop.DrumEnvelop
local ____Encoder3 = require("config.libs.components.Encoder3")
local Encoder3 = ____Encoder3.Encoder3
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_kick.ui.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local W1_4 = ____constants.W1_4
local W3_4 = ____constants.W3_4
local ____constantsValue = require("config.pixel.240x320_kick.ui.constantsValue")
local encoderH = ____constantsValue.encoderH
local height = ____constantsValue.height
local ____TextGridPage1 = require("config.pixel.240x320_kick.ui.Page1.TextGridPage1")
local TextGridPage1 = ____TextGridPage1.TextGridPage1
function ____exports.FrequencyView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(DrumEnvelop, {
            position = {0, 0, W3_4 - 2, height},
            plugin = "Drum23",
            envelop_data_id = "4",
            RENDER_TITLE_ON_TOP = false,
            encoder_time = 0,
            encoder_phase = 1,
            encoder_modulation = 2
        }),
        React.createElement(Encoder3, {position = {W3_4, (height - encoderH) * 0.5, W1_4, encoderH}, value = "Drum23 PITCH", encoder_id = 3, color = "secondary"}),
        React.createElement(TextGridPage1, {selected = 4}),
        React.createElement(Common, {selected = 0, page = "Page2"})
    )
end
return ____exports
