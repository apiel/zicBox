--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Spectrogram = require("config.libs.components.Spectrogram")
local Spectrogram = ____Spectrogram.Spectrogram
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local ScreenWidth = ____constants.ScreenWidth
local ____ProgressBar = require("config.pixel.240x320_kick.ui.components.ProgressBar")
local ProgressBar = ____ProgressBar.ProgressBar
local ____TextGridCommon = require("config.pixel.240x320_kick.ui.components.TextGridCommon")
local TextGridCommon = ____TextGridCommon.TextGridCommon
function ____exports.Common(____bindingPattern0)
    local selected
    selected = ____bindingPattern0.selected
    return React.createElement(
        React.Fragment,
        nil,
        React.createElement(Spectrogram, {position = {0, 100, ScreenWidth, 70}, data = "Spectrogram BUFFER", text = "Pixel", raw_buffer = true}),
        React.createElement(ProgressBar, nil),
        React.createElement(TextGridCommon, {selected = selected})
    )
end
return ____exports
