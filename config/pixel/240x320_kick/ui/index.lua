--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local ____pixelController = require("config.libs.controllers.pixelController")
local pixelController = ____pixelController.pixelController
local ____core = require("config.libs.core")
local applyZic = ____core.applyZic
local React = require("config.libs.react")
local ____ui = require("config.libs.ui")
local addZoneEncoder = ____ui.addZoneEncoder
local setScreenSize = ____ui.setScreenSize
local setWindowPosition = ____ui.setWindowPosition
local ____BassView = require("config.pixel.240x320_kick.ui.Bass.BassView")
local BassView = ____BassView.BassView
local ____DistortionView = require("config.pixel.240x320_kick.ui.Bass.DistortionView")
local BassDistortionView = ____DistortionView.BassDistortionView
local ____EnvView = require("config.pixel.240x320_kick.ui.Bass.EnvView")
local BassEnvView = ____EnvView.BassEnvView
local ____SeqView = require("config.pixel.240x320_kick.ui.Bass.SeqView")
local BassSeqView = ____SeqView.BassSeqView
local ____WaveformView = require("config.pixel.240x320_kick.ui.Bass.WaveformView")
local BassWaveformView = ____WaveformView.BassWaveformView
local ____ClipsView = require("config.pixel.240x320_kick.ui.clipsView.ClipsView")
local ClipsView = ____ClipsView.ClipsView
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local ScreenHeight = ____constants.ScreenHeight
local ScreenWidth = ____constants.ScreenWidth
local W1_2 = ____constants.W1_2
local ____AmpView = require("config.pixel.240x320_kick.ui.Drum23.AmpView")
local AmpView = ____AmpView.AmpView
local ____ClickView = require("config.pixel.240x320_kick.ui.Drum23.ClickView")
local ClickView = ____ClickView.ClickView
local ____DistortionView = require("config.pixel.240x320_kick.ui.Drum23.DistortionView")
local DistortionView = ____DistortionView.DistortionView
local ____Drum23View = require("config.pixel.240x320_kick.ui.Drum23.Drum23View")
local Drum23View = ____Drum23View.Drum23View
local ____FrequencyView = require("config.pixel.240x320_kick.ui.Drum23.FrequencyView")
local FrequencyView = ____FrequencyView.FrequencyView
local ____SeqView = require("config.pixel.240x320_kick.ui.Drum23.SeqView")
local Drum23SeqView = ____SeqView.Drum23SeqView
local ____WaveformView = require("config.pixel.240x320_kick.ui.Drum23.WaveformView")
local WaveformView = ____WaveformView.WaveformView
local ____CreateWorkspaceView = require("config.pixel.240x320_kick.ui.menu.CreateWorkspaceView")
local CreateWorkspaceView = ____CreateWorkspaceView.CreateWorkspaceView
local ____menuView = require("config.pixel.240x320_kick.ui.menu.menuView")
local MenuView = ____menuView.MenuView
local ____ShutdownView = require("config.pixel.240x320_kick.ui.menu.ShutdownView")
local ShutdownView = ____ShutdownView.ShutdownView
local ____WorkspacesView = require("config.pixel.240x320_kick.ui.menu.WorkspacesView")
local WorkspacesView = ____WorkspacesView.WorkspacesView
local ____SampleEditorView = require("config.pixel.240x320_kick.ui.Sample.SampleEditorView")
local SampleEditorView = ____SampleEditorView.SampleEditorView
local ____SaveTapeView = require("config.pixel.240x320_kick.ui.tapeView.SaveTapeView")
local SaveTapeView = ____SaveTapeView.SaveTapeView
local ____TapeView = require("config.pixel.240x320_kick.ui.tapeView.TapeView")
local TapeView = ____TapeView.TapeView
pixelController("rpi0_4enc_6btn")
setWindowPosition(400, 500)
setScreenSize(ScreenWidth, ScreenHeight)
local halfHeight = ScreenHeight / 2
addZoneEncoder({0, 0, W1_2, halfHeight})
addZoneEncoder({W1_2, 0, W1_2, halfHeight})
addZoneEncoder({0, halfHeight, W1_2, halfHeight})
addZoneEncoder({W1_2, halfHeight, W1_2, halfHeight})
applyZic(React.createElement(Drum23View, {name = "Drum23"}))
applyZic(React.createElement(DistortionView, {name = "Distortion"}))
applyZic(React.createElement(WaveformView, {name = "Waveform"}))
applyZic(React.createElement(FrequencyView, {name = "Frequency"}))
applyZic(React.createElement(AmpView, {name = "Amplitude"}))
applyZic(React.createElement(ClickView, {name = "Click"}))
applyZic(React.createElement(Drum23SeqView, {name = "Sequencer"}))
applyZic(React.createElement(SampleEditorView, {name = "Sample"}))
applyZic(React.createElement(BassView, {name = "Bass"}))
applyZic(React.createElement(BassDistortionView, {name = "BassDistortion"}))
applyZic(React.createElement(BassEnvView, {name = "BassEnv"}))
applyZic(React.createElement(BassSeqView, {name = "BassSeq"}))
applyZic(React.createElement(BassWaveformView, {name = "BassWaveform"}))
applyZic(React.createElement(MenuView, {name = "Menu"}))
applyZic(React.createElement(WorkspacesView, {name = "Workspaces"}))
applyZic(React.createElement(CreateWorkspaceView, {name = "CreateWorkspace"}))
applyZic(React.createElement(ShutdownView, {name = "Shutdown"}))
applyZic(React.createElement(TapeView, {name = "Tape"}))
applyZic(React.createElement(SaveTapeView, {name = "SaveTape"}))
applyZic(React.createElement(ClipsView, {name = "Clips"}))
return ____exports
