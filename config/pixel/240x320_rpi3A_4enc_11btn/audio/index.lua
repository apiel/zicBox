--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local ____audio = require("config.libs.audio")
local audioPlugin = ____audio.audioPlugin
local ____core = require("config.libs.core")
local applyZic = ____core.applyZic
local jsonStringify = ____core.jsonStringify
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
local plugins = {}
local track = Drum23Track
plugins[#plugins + 1] = audioPlugin("SynthDrum23", {alias = "Drum23", track = track})
plugins[#plugins + 1] = audioPlugin("Sequencer", {track = track})
plugins[#plugins + 1] = audioPlugin("EffectDistortion2", {alias = "Distortion", track = track})
plugins[#plugins + 1] = audioPlugin("EffectFilterMultiMode", {alias = "MMFilter", cutoffStringFormat = ____exports.cutoffStringFormat, track = track})
plugins[#plugins + 1] = audioPlugin("EffectGainVolume", {alias = "TrackFx", track = track})
plugins[#plugins + 1] = audioPlugin("SerializeTrack", {track = track, filename = "drum23", maxVariation = ____exports.maxVariation, workspaceFolder = workspaceFolder})
track = BassTrack
plugins[#plugins + 1] = audioPlugin("SynthBass", {alias = "Bass", track = track})
plugins[#plugins + 1] = audioPlugin("Sequencer", {track = track})
plugins[#plugins + 1] = audioPlugin("EffectGainVolume", {alias = "TrackFx", track = track})
plugins[#plugins + 1] = audioPlugin("SerializeTrack", {track = track, filename = "bass", maxVariation = ____exports.maxVariation, workspaceFolder = workspaceFolder})
track = Fm1Track
plugins[#plugins + 1] = audioPlugin("SynthFmDrum", {alias = "FmDrum", track = track})
plugins[#plugins + 1] = audioPlugin("Sequencer", {track = track})
plugins[#plugins + 1] = audioPlugin("EffectGainVolume", {alias = "TrackFx", track = track})
plugins[#plugins + 1] = audioPlugin("SerializeTrack", {track = track, filename = "fmDrum", maxVariation = ____exports.maxVariation, workspaceFolder = workspaceFolder})
track = SynthTrack
plugins[#plugins + 1] = audioPlugin("SynthHybrid", {alias = "Synth", track = track})
plugins[#plugins + 1] = audioPlugin("Sequencer", {track = track})
plugins[#plugins + 1] = audioPlugin("EffectGainVolume", {alias = "TrackFx", track = track})
plugins[#plugins + 1] = audioPlugin("SerializeTrack", {track = track, filename = "synth", maxVariation = ____exports.maxVariation, workspaceFolder = workspaceFolder})
plugins[#plugins + 1] = audioPlugin("SampleSequencer", {track = SampleTrack})
plugins[#plugins + 1] = audioPlugin("SerializeTrack", {track = SampleTrack, filename = "sampleSeq", maxVariation = ____exports.maxVariation, workspaceFolder = workspaceFolder})
track = PercTrack
plugins[#plugins + 1] = audioPlugin("SynthPerc", {alias = "Perc", track = track})
plugins[#plugins + 1] = audioPlugin("Sequencer", {track = track})
plugins[#plugins + 1] = audioPlugin("EffectGainVolume", {alias = "TrackFx", track = track})
plugins[#plugins + 1] = audioPlugin("SerializeTrack", {track = track, filename = "perc", maxVariation = ____exports.maxVariation, workspaceFolder = workspaceFolder})
plugins[#plugins + 1] = audioPlugin("Mixer6", {alias = "Mixer"})
plugins[#plugins + 1] = audioPlugin("EffectGainVolume", {alias = "Volume"})
plugins[#plugins + 1] = audioPlugin("AudioOutputPulse", {alias = "AudioOutput"})
plugins[#plugins + 1] = audioPlugin("SerializeTrack", {filename = "master", maxVariation = ____exports.maxVariation, workspaceFolder = workspaceFolder})
plugins[#plugins + 1] = audioPlugin("Tempo")
applyZic({{AUDIO = jsonStringify({plugins = plugins, autoSave = 500})}})
return ____exports
