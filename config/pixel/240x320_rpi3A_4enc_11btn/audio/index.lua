--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local ____audio = require("config.libs.audio")
local audioPlugin = ____audio.audioPlugin
local autoSave = ____audio.autoSave
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.constants")
local BassTrack = ____constants.BassTrack
local Drum23Track = ____constants.Drum23Track
local Fm1Track = ____constants.Fm1Track
local PercTrack = ____constants.PercTrack
local SampleTrack = ____constants.SampleTrack
local SynthTrack = ____constants.SynthTrack
____exports.cutoffStringFormat = "%d%% %d%%"
____exports.maxVariation = 16
local workspaceFolder = "workspaces/rpi3A_4enc_11btn"
local track = Drum23Track
audioPlugin("SynthDrum23", {alias = "Drum23", track = track})
audioPlugin("Sequencer", {track = track})
audioPlugin("EffectDistortion2", {alias = "Distortion", track = track})
audioPlugin("EffectFilterMultiMode", {alias = "MMFilter", cutoffStringFormat = ____exports.cutoffStringFormat, track = track})
audioPlugin("EffectGainVolume", {alias = "TrackFx", track = track})
audioPlugin("SerializeTrack", {track = track, filename = "drum23", maxVariation = ____exports.maxVariation, workspaceFolder = workspaceFolder})
track = BassTrack
audioPlugin("SynthBass", {alias = "Bass", track = track})
audioPlugin("Sequencer", {track = track})
audioPlugin("EffectGainVolume", {alias = "TrackFx", track = track})
audioPlugin("SerializeTrack", {track = track, filename = "bass", maxVariation = ____exports.maxVariation, workspaceFolder = workspaceFolder})
track = Fm1Track
audioPlugin("SynthFmDrum", {alias = "FmDrum", track = track})
audioPlugin("Sequencer", {track = track})
audioPlugin("EffectGainVolume", {alias = "TrackFx", track = track})
audioPlugin("SerializeTrack", {track = track, filename = "fmDrum", maxVariation = ____exports.maxVariation, workspaceFolder = workspaceFolder})
track = SynthTrack
audioPlugin("SynthHybrid", {alias = "Synth", track = track})
audioPlugin("Sequencer", {track = track})
audioPlugin("EffectGainVolume", {alias = "TrackFx", track = track})
audioPlugin("SerializeTrack", {track = track, filename = "synth", maxVariation = ____exports.maxVariation, workspaceFolder = workspaceFolder})
audioPlugin("SampleSequencer", {track = SampleTrack})
audioPlugin("SerializeTrack", {track = SampleTrack, filename = "sampleSeq", maxVariation = ____exports.maxVariation, workspaceFolder = workspaceFolder})
track = PercTrack
audioPlugin("SynthPerc", {alias = "Perc", track = track})
audioPlugin("Sequencer", {track = track})
audioPlugin("EffectGainVolume", {alias = "TrackFx", track = track})
audioPlugin("SerializeTrack", {track = track, filename = "perc", maxVariation = ____exports.maxVariation, workspaceFolder = workspaceFolder})
audioPlugin("Mixer6", {alias = "Mixer"})
audioPlugin("EffectGainVolume", {alias = "Volume"})
audioPlugin("AudioOutputPulse", {alias = "AudioOutput"})
audioPlugin("SerializeTrack", {filename = "master", maxVariation = ____exports.maxVariation, workspaceFolder = workspaceFolder})
audioPlugin("Tempo")
autoSave(500)
return ____exports
