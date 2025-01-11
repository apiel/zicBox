--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local ____core = require("config.libs.core")
local applyZic = ____core.applyZic
local React = require("config.libs.react")
local ____ui = require("config.libs.ui")
local addZoneEncoder = ____ui.addZoneEncoder
local setScreenSize = ____ui.setScreenSize
local setWindowPosition = ____ui.setWindowPosition
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local ScreenHeight = ____constants.ScreenHeight
local ScreenWidth = ____constants.ScreenWidth
local W1_2 = ____constants.W1_2
local W1_4 = ____constants.W1_4
local W3_4 = ____constants.W3_4
local ____Page1View = require("config.pixel.240x320_kick.ui.Page1View.index")
local Page1View = ____Page1View.Page1View
local ____DistortionView = require("config.pixel.240x320_kick.ui.Page1View.DistortionView")
local DistortionView = ____DistortionView.DistortionView
local ____WaveformView = require("config.pixel.240x320_kick.ui.Page1View.WaveformView")
local WaveformView = ____WaveformView.WaveformView
setWindowPosition(400, 500)
setScreenSize(ScreenWidth, ScreenHeight)
addZoneEncoder({0, 0, W1_4, ScreenHeight})
addZoneEncoder({W1_4, 0, W1_4, ScreenHeight})
addZoneEncoder({W1_2, 0, W1_4, ScreenHeight})
addZoneEncoder({W3_4, 0, W1_4, ScreenHeight})
applyZic(React.createElement(Page1View, {name = "Page1"}))
applyZic(React.createElement(DistortionView, {name = "Distortion"}))
applyZic(React.createElement(WaveformView, {name = "Waveform"}))
return ____exports
