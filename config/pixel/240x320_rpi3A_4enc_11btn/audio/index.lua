--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local ____audio = require("config.libs.audio")
local autoSave = ____audio.autoSave
local plugin = ____audio.plugin
local pluginAlias = ____audio.pluginAlias
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.constants")
local BassTrack = ____constants.BassTrack
local Drum23Track = ____constants.Drum23Track
local Fm1Track = ____constants.Fm1Track
local PercTrack = ____constants.PercTrack
local SampleTrack = ____constants.SampleTrack
local SynthTrack = ____constants.SynthTrack
pluginAlias("EffectDistortion2", "libzic_EffectDistortion2.so")
pluginAlias("EffectFilterMultiMode", "libzic_EffectFilterMultiMode.so")
pluginAlias("EffectGainVolume", "libzic_EffectGainVolume.so")
pluginAlias("SerializeTrack", "libzic_SerializeTrack.so")
pluginAlias("Tempo", "libzic_Tempo.so")
pluginAlias("Drum23", "libzic_SynthDrum23.so")
pluginAlias("FmDrum", "libzic_SynthFmDrum.so")
pluginAlias("Perc", "libzic_SynthPerc.so")
pluginAlias("Bass", "libzic_SynthBass.so")
pluginAlias("Synth", "libzic_SynthHybrid.so")
pluginAlias("Sample", "libzic_SynthMonoSample.so")
pluginAlias("Sequencer", "libzic_Sequencer.so")
pluginAlias("AudioOutput", "libzic_AudioOutputPulse.so")
pluginAlias("Mixer", "libzic_Mixer6.so")
pluginAlias("Tape", "libzic_TapeRecording.so")
pluginAlias("SampleSequencer", "libzic_SampleSequencer.so")
____exports.STRING_CUTOFF_FORMAT = "%d%% %d%%"
____exports.MAX_VARIATION = 16
local WORKSPACE_FOLDER = "workspaces/rpi3A_4enc_11btn"
local track = Drum23Track
plugin("Drum23", {{track = track}})
plugin("Sequencer", {{track = track}})
plugin("Distortion EffectDistortion2", {{track = track}})
plugin("MMFilter EffectFilterMultiMode", {{STRING_CUTOFF_FORMAT = ____exports.STRING_CUTOFF_FORMAT, track = track}})
plugin("TrackFx EffectGainVolume", {{track = track}})
plugin("SerializeTrack", {{track = track, filename = "drum23", MAX_VARIATION = ____exports.MAX_VARIATION, WORKSPACE_FOLDER = WORKSPACE_FOLDER}})
track = BassTrack
plugin("Bass", {{track = track}})
plugin("Sequencer", {{track = track}})
plugin("TrackFx EffectGainVolume", {{track = track}})
plugin("SerializeTrack", {{track = track, filename = "bass", MAX_VARIATION = ____exports.MAX_VARIATION, WORKSPACE_FOLDER = WORKSPACE_FOLDER}})
track = Fm1Track
plugin("FmDrum", {{track = track}})
plugin("Sequencer", {{track = track}})
plugin("TrackFx EffectGainVolume", {{track = track}})
plugin("SerializeTrack", {{track = track, filename = "fmDrum", MAX_VARIATION = ____exports.MAX_VARIATION, WORKSPACE_FOLDER = WORKSPACE_FOLDER}})
track = SynthTrack
plugin("Synth", {{track = track}})
plugin("Sequencer", {{track = track}})
plugin("TrackFx EffectGainVolume", {{track = track}})
plugin("SerializeTrack", {{track = track, filename = "synth", MAX_VARIATION = ____exports.MAX_VARIATION, WORKSPACE_FOLDER = WORKSPACE_FOLDER}})
plugin("SampleSequencer", {{track = SampleTrack}})
plugin("SerializeTrack", {{track = SampleTrack, filename = "sampleSeq", MAX_VARIATION = ____exports.MAX_VARIATION, WORKSPACE_FOLDER = WORKSPACE_FOLDER}})
track = PercTrack
plugin("Perc", {{track = track}})
plugin("Sequencer", {{track = track}})
plugin("TrackFx EffectGainVolume", {{track = track}})
plugin("SerializeTrack", {{track = track, filename = "perc", MAX_VARIATION = ____exports.MAX_VARIATION, WORKSPACE_FOLDER = WORKSPACE_FOLDER}})
plugin("Mixer")
plugin("Volume EffectGainVolume")
plugin("AudioOutput")
plugin("SerializeTrack", {{filename = "master", MAX_VARIATION = ____exports.MAX_VARIATION, WORKSPACE_FOLDER = WORKSPACE_FOLDER}})
plugin("Tempo")
autoSave(500)
return ____exports
