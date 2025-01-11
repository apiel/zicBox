--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____DrumEnvelop = require("config.libs.components.DrumEnvelop")
local DrumEnvelop = ____DrumEnvelop.DrumEnvelop
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_kick.ui.Common")
local Common = ____Common.Common
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local W3_4 = ____constants.W3_4
local ____constantsValue = require("config.pixel.240x320_kick.ui.constantsValue")
local height = ____constantsValue.height
local ____TextGridPage2 = require("config.pixel.240x320_kick.ui.Page2.TextGridPage2")
local TextGridPage2 = ____TextGridPage2.TextGridPage2
function ____exports.Page2View(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(DrumEnvelop, {
            position = {0, 0, W3_4 - 2, height},
            plugin = "Drum23",
            envelop_data_id = "0",
            RENDER_TITLE_ON_TOP = false,
            encoder_time = 0,
            encoder_phase = 1,
            encoder_modulation = 2
        }),
        React.createElement(TextGridPage2, {selected = 0}),
        React.createElement(Common, {selected = 1, page = "Page1"})
    )
end
return ____exports
