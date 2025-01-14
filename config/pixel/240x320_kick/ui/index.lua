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
local ____Seq = require("config.pixel.240x320_kick.ui.Seq.index")
local SeqView = ____Seq.SeqView
local ____AmpView = require("config.pixel.240x320_kick.ui.Synth.AmpView")
local AmpView = ____AmpView.AmpView
local ____ClickView = require("config.pixel.240x320_kick.ui.Synth.ClickView")
local ClickView = ____ClickView.ClickView
local ____DistortionView = require("config.pixel.240x320_kick.ui.Synth.DistortionView")
local DistortionView = ____DistortionView.DistortionView
local ____FrequencyView = require("config.pixel.240x320_kick.ui.Synth.FrequencyView")
local FrequencyView = ____FrequencyView.FrequencyView
local ____MasterView = require("config.pixel.240x320_kick.ui.Synth.MasterView")
local MasterView = ____MasterView.MasterView
local ____WaveformView = require("config.pixel.240x320_kick.ui.Synth.WaveformView")
local WaveformView = ____WaveformView.WaveformView
setWindowPosition(400, 500)
setScreenSize(ScreenWidth, ScreenHeight)
addZoneEncoder({0, 0, W1_4, ScreenHeight})
addZoneEncoder({W1_4, 0, W1_4, ScreenHeight})
addZoneEncoder({W1_2, 0, W1_4, ScreenHeight})
addZoneEncoder({W3_4, 0, W1_4, ScreenHeight})
applyZic(React.createElement(MasterView, {name = "Master"}))
applyZic(React.createElement(DistortionView, {name = "Distortion"}))
applyZic(React.createElement(WaveformView, {name = "Waveform"}))
applyZic(React.createElement(FrequencyView, {name = "Frequency"}))
applyZic(React.createElement(AmpView, {name = "Amplitude"}))
applyZic(React.createElement(ClickView, {name = "Click"}))
applyZic(React.createElement(SeqView, {name = "Sequencer"}))
return ____exports
