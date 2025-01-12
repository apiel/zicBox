--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____SeqProgressBar = require("config.libs.components.SeqProgressBar")
local SeqProgressBar = ____SeqProgressBar.SeqProgressBar
local ____Spectrogram = require("config.libs.components.Spectrogram")
local Spectrogram = ____Spectrogram.Spectrogram
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local ScreenWidth = ____constants.ScreenWidth
local ____TextGridCommon = require("config.pixel.240x320_kick.ui.components.TextGridCommon")
local TextGridCommon = ____TextGridCommon.TextGridCommon
function ____exports.Common(____bindingPattern0)
    local selected
    selected = ____bindingPattern0.selected
    return React.createElement(
        React.Fragment,
        nil,
        React.createElement(Spectrogram, {
            position = {0, 245, ScreenWidth, 35},
            data = "Spectrogram BUFFER",
            text = "Pixel",
            wave_color = "#23a123",
            raw_buffer = true
        }),
        React.createElement(SeqProgressBar, {
            position = {0, 285, ScreenWidth, 5},
            seq_plugin = "Sequencer 0",
            active_color = "#23a123",
            selection_color = "#23a123",
            volume_plugin = "Volume VOLUME"
        }),
        React.createElement(TextGridCommon, {selected = selected})
    )
end
return ____exports
