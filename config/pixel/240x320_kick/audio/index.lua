--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local ____audio = require("config.libs.audio")
local autoSave = ____audio.autoSave
local plugin = ____audio.plugin
local pluginAlias = ____audio.pluginAlias
local ____constants = require("config.pixel.240x320_kick.audio.constants")
local MAX_VARIATION = ____constants.MAX_VARIATION
local STRING_CUTOFF_FORMAT = ____constants.STRING_CUTOFF_FORMAT
pluginAlias("EffectDistortion2", "libzic_EffectDistortion2.so")
pluginAlias("EffectFilterMultiMode", "libzic_EffectFilterMultiMode.so")
pluginAlias("EffectGainVolume", "libzic_EffectGainVolume.so")
pluginAlias("SerializeTrack", "libzic_SerializeTrack.so")
pluginAlias("Tempo", "libzic_Tempo.so")
pluginAlias("Drum23", "libzic_SynthDrum23.so")
pluginAlias("Sequencer", "libzic_Sequencer.so")
pluginAlias("AudioOutput", "libzic_AudioOutputPulse.so")
pluginAlias("Spectrogram", "libzic_AudioSpectrogram.so")
pluginAlias("Tape", "libzic_TapeRecording.so")
plugin("Drum23")
plugin("Sequencer")
plugin("Distortion EffectDistortion2")
plugin("MMFilter EffectFilterMultiMode", {{STRING_CUTOFF_FORMAT = STRING_CUTOFF_FORMAT}})
plugin("Volume EffectGainVolume")
plugin("Spectrogram")
plugin("Tape", {{filename = "kick", max_track = 0}})
plugin("AudioOutput")
plugin("SerializeTrack", {{filename = "master", MAX_VARIATION = MAX_VARIATION, WORKSPACE_FOLDER = "workspaces_kick"}})
plugin("Tempo")
autoSave(500)
return ____exports
