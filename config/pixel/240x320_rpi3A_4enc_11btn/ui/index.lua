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
local ____BassView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Bass.BassView")
local BassView = ____BassView.BassView
local ____EnvView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Bass.EnvView")
local BassEnvView = ____EnvView.BassEnvView
local ____FxView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Bass.FxView")
local BassFxView = ____FxView.BassFxView
local ____SeqView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Bass.SeqView")
local BassSeqView = ____SeqView.BassSeqView
local ____WaveformView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Bass.WaveformView")
local BassWaveformView = ____WaveformView.BassWaveformView
local ____ClipsView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.clipsView.ClipsView")
local ClipsView = ____ClipsView.ClipsView
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local ScreenHeight = ____constants.ScreenHeight
local ScreenWidth = ____constants.ScreenWidth
local W1_2 = ____constants.W1_2
local ____AmpView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Drum23.AmpView")
local AmpView = ____AmpView.AmpView
local ____ClickView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Drum23.ClickView")
local ClickView = ____ClickView.ClickView
local ____DistortionView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Drum23.DistortionView")
local DistortionView = ____DistortionView.DistortionView
local ____Drum23View = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Drum23.Drum23View")
local Drum23View = ____Drum23View.Drum23View
local ____FrequencyView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Drum23.FrequencyView")
local FrequencyView = ____FrequencyView.FrequencyView
local ____SeqView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Drum23.SeqView")
local Drum23SeqView = ____SeqView.Drum23SeqView
local ____WaveformView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Drum23.WaveformView")
local WaveformView = ____WaveformView.WaveformView
local ____EnvView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Fm1.EnvView")
local Fm1EnvView = ____EnvView.Fm1EnvView
local ____Fm1View = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Fm1.Fm1View")
local Fm1View = ____Fm1View.Fm1View
local ____FxView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Fm1.FxView")
local Fm1FxView = ____FxView.Fm1FxView
local ____SeqView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Fm1.SeqView")
local Fm1SeqView = ____SeqView.Fm1SeqView
local ____CreateWorkspaceView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.menu.CreateWorkspaceView")
local CreateWorkspaceView = ____CreateWorkspaceView.CreateWorkspaceView
local ____menuView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.menu.menuView")
local MenuView = ____menuView.MenuView
local ____ShutdownView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.menu.ShutdownView")
local ShutdownView = ____ShutdownView.ShutdownView
local ____WorkspacesView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.menu.WorkspacesView")
local WorkspacesView = ____WorkspacesView.WorkspacesView
local ____PercFxView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Perc.PercFxView")
local PercFxView = ____PercFxView.PercFxView
local ____PercModView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Perc.PercModView")
local PercModView = ____PercModView.PercModView
local ____PercNoise2View = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Perc.PercNoise2View")
local PercNoise2View = ____PercNoise2View.PercNoise2View
local ____PercNoiseView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Perc.PercNoiseView")
local PercNoiseView = ____PercNoiseView.PercNoiseView
local ____PercView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Perc.PercView")
local PercView = ____PercView.PercView
local ____SeqView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Perc.SeqView")
local PercSeqView = ____SeqView.PercSeqView
local ____SampleEditorView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Sample.SampleEditorView")
local SampleEditorView = ____SampleEditorView.SampleEditorView
local ____Env1View = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Synth.Env1View")
local SynthEnv1View = ____Env1View.SynthEnv1View
local ____Env2View = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Synth.Env2View")
local SynthEnv2View = ____Env2View.SynthEnv2View
local ____FxhView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Synth.FxhView")
local SynthFxView = ____FxhView.SynthFxView
local ____SynthView = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Synth.SynthView")
local SynthView = ____SynthView.SynthView
local ____Waveform1View = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Synth.Waveform1View")
local SynthWaveform1View = ____Waveform1View.SynthWaveform1View
local ____Waveform2View = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.Synth.Waveform2View")
local SynthWaveform2View = ____Waveform2View.SynthWaveform2View
pixelController("rpi3A_4enc_11btn")
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
applyZic(React.createElement(BassFxView, {name = "BassFx"}))
applyZic(React.createElement(BassEnvView, {name = "BassEnv"}))
applyZic(React.createElement(BassSeqView, {name = "BassSeq"}))
applyZic(React.createElement(BassWaveformView, {name = "BassWaveform"}))
applyZic(React.createElement(Fm1View, {name = "Fm1"}))
applyZic(React.createElement(Fm1FxView, {name = "Fm1Fx"}))
applyZic(React.createElement(Fm1EnvView, {name = "Fm1Env"}))
applyZic(React.createElement(Fm1SeqView, {name = "Fm1Seq"}))
applyZic(React.createElement(PercView, {name = "Perc"}))
applyZic(React.createElement(PercFxView, {name = "PercFx"}))
applyZic(React.createElement(PercSeqView, {name = "PercSeq"}))
applyZic(React.createElement(PercNoiseView, {name = "PercNoise"}))
applyZic(React.createElement(PercNoise2View, {name = "PercNoise2"}))
applyZic(React.createElement(PercModView, {name = "PercMod"}))
applyZic(React.createElement(SynthView, {name = "Synth"}))
applyZic(React.createElement(SynthFxView, {name = "SynthFx"}))
applyZic(React.createElement(SynthEnv1View, {name = "SynthEnv1"}))
applyZic(React.createElement(SynthEnv2View, {name = "SynthEnv2"}))
applyZic(React.createElement(SynthWaveform1View, {name = "SynthWaveform1"}))
applyZic(React.createElement(SynthWaveform2View, {name = "SynthWaveform2"}))
applyZic(React.createElement(MenuView, {name = "Menu"}))
applyZic(React.createElement(WorkspacesView, {name = "Workspaces"}))
applyZic(React.createElement(CreateWorkspaceView, {name = "CreateWorkspace"}))
applyZic(React.createElement(ShutdownView, {name = "Shutdown"}))
applyZic(React.createElement(ClipsView, {name = "Clips"}))
return ____exports
