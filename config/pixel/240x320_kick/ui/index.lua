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
local ____Page1 = require("config.pixel.240x320_kick.ui.Page1.index")
local Page1View = ____Page1.Page1View
local ____DistortionView = require("config.pixel.240x320_kick.ui.Page1.DistortionView")
local DistortionView = ____DistortionView.DistortionView
local ____FrequencyView = require("config.pixel.240x320_kick.ui.Page1.FrequencyView")
local FrequencyView = ____FrequencyView.FrequencyView
local ____WaveformView = require("config.pixel.240x320_kick.ui.Page1.WaveformView")
local WaveformView = ____WaveformView.WaveformView
local ____Page2 = require("config.pixel.240x320_kick.ui.Page2.index")
local Page2View = ____Page2.Page2View
local ____ClickView = require("config.pixel.240x320_kick.ui.Page2.ClickView")
local ClickView = ____ClickView.ClickView
local ____Seq = require("config.pixel.240x320_kick.ui.Seq.index")
local SeqView = ____Seq.SeqView
setWindowPosition(400, 500)
setScreenSize(ScreenWidth, ScreenHeight)
addZoneEncoder({0, 0, W1_4, ScreenHeight})
addZoneEncoder({W1_4, 0, W1_4, ScreenHeight})
addZoneEncoder({W1_2, 0, W1_4, ScreenHeight})
addZoneEncoder({W3_4, 0, W1_4, ScreenHeight})
applyZic(React.createElement(Page1View, {name = "Page1"}))
applyZic(React.createElement(DistortionView, {name = "Distortion"}))
applyZic(React.createElement(WaveformView, {name = "Waveform"}))
applyZic(React.createElement(FrequencyView, {name = "Frequency"}))
applyZic(React.createElement(Page2View, {name = "Page2"}))
applyZic(React.createElement(ClickView, {name = "Click"}))
applyZic(React.createElement(SeqView, {name = "Sequencer"}))
return ____exports
