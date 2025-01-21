--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local ____audio = require("config.libs.audio")
local autoSave = ____audio.autoSave
local plugin = ____audio.plugin
local pluginAlias = ____audio.pluginAlias
local ____constants = require("config.pixel.240x320_kick.constants")
local Drum23Track = ____constants.Drum23Track
local FmTrack = ____constants.FmTrack
local HiHatTrack = ____constants.HiHatTrack
local SampleTrack = ____constants.SampleTrack
local SnareTrack = ____constants.SnareTrack
pluginAlias("EffectDistortion2", "libzic_EffectDistortion2.so")
pluginAlias("EffectFilterMultiMode", "libzic_EffectFilterMultiMode.so")
pluginAlias("EffectVolumeClipping", "libzic_EffectVolumeClipping.so")
pluginAlias("EffectGainVolume", "libzic_EffectGainVolume.so")
pluginAlias("SerializeTrack", "libzic_SerializeTrack.so")
pluginAlias("Tempo", "libzic_Tempo.so")
pluginAlias("Drum23", "libzic_SynthDrum23.so")
pluginAlias("Snare", "libzic_SynthSnare.so")
pluginAlias("HiHat", "libzic_SynthHiHat.so")
pluginAlias("FmDrum", "libzic_SynthFmDrum.so")
pluginAlias("Sample", "libzic_SynthMonoSample.so")
pluginAlias("Sequencer", "libzic_Sequencer.so")
pluginAlias("AudioOutput", "libzic_AudioOutputPulse.so")
pluginAlias("Mixer", "libzic_Mixer5.so")
pluginAlias("Tape", "libzic_TapeRecording.so")
____exports.STRING_CUTOFF_FORMAT = "%d%% %d%%"
____exports.MAX_VARIATION = 16
local WORKSPACE_FOLDER = "workspaces_kick"
local track = Drum23Track
plugin("Drum23", {{track = track}})
plugin("Sequencer", {{track = track}})
plugin("Distortion EffectDistortion2", {{track = track}})
plugin("MMFilter EffectFilterMultiMode", {{STRING_CUTOFF_FORMAT = ____exports.STRING_CUTOFF_FORMAT, track = track}})
plugin("Volume EffectGainVolume", {{track = track}})
plugin("SerializeTrack", {{track = track, filename = "drum23", MAX_VARIATION = ____exports.MAX_VARIATION, WORKSPACE_FOLDER = WORKSPACE_FOLDER}})
track = FmTrack
plugin("FmDrum", {{track = track}})
plugin("Sequencer", {{track = track}})
plugin("Volume EffectGainVolume", {{track = track}})
plugin("SerializeTrack", {{track = track, filename = "fm", MAX_VARIATION = ____exports.MAX_VARIATION, WORKSPACE_FOLDER = WORKSPACE_FOLDER}})
track = SnareTrack
plugin("Snare", {{track = track}})
plugin("Sequencer", {{track = track}})
plugin("Volume EffectGainVolume", {{track = track}})
plugin("SerializeTrack", {{track = track, filename = "snare", MAX_VARIATION = ____exports.MAX_VARIATION, WORKSPACE_FOLDER = WORKSPACE_FOLDER}})
track = HiHatTrack
plugin("HiHat", {{track = track}})
plugin("Sequencer", {{track = track}})
plugin("Volume EffectGainVolume", {{track = track}})
plugin("SerializeTrack", {{track = track, filename = "hihat", MAX_VARIATION = ____exports.MAX_VARIATION, WORKSPACE_FOLDER = WORKSPACE_FOLDER}})
track = SampleTrack
plugin("Sample", {{track = track}})
plugin("Sequencer", {{track = track}})
plugin("Volume EffectVolumeClipping", {{track = track}})
plugin("SerializeTrack", {{track = track, filename = "sample", MAX_VARIATION = ____exports.MAX_VARIATION, WORKSPACE_FOLDER = WORKSPACE_FOLDER}})
plugin("Mixer")
plugin("SerializeTrack", {{filename = "mixer", MAX_VARIATION = ____exports.MAX_VARIATION, WORKSPACE_FOLDER = WORKSPACE_FOLDER}})
plugin("AudioOutput")
plugin("SerializeTrack", {{filename = "master", MAX_VARIATION = ____exports.MAX_VARIATION, WORKSPACE_FOLDER = WORKSPACE_FOLDER}})
plugin("Tempo")
autoSave(500)
return ____exports
