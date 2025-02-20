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
____exports.STRING_CUTOFF_FORMAT = "%d%% %d%%"
____exports.MAX_VARIATION = 16
local WORKSPACE_FOLDER = "workspaces/rpi3A_4enc_11btn"
local track = Drum23Track
audioPlugin({plugin = "SynthDrum23", aliasName = "Drum23", track = track})
audioPlugin({plugin = "Sequencer", track = track})
audioPlugin({plugin = "EffectDistortion2", aliasName = "Distortion", track = track})
audioPlugin({plugin = "EffectFilterMultiMode", aliasName = "MMFilter", STRING_CUTOFF_FORMAT = ____exports.STRING_CUTOFF_FORMAT, track = track})
audioPlugin({plugin = "EffectGainVolume", aliasName = "TrackFx", track = track})
audioPlugin({
    plugin = "SerializeTrack",
    track = track,
    filename = "drum23",
    MAX_VARIATION = ____exports.MAX_VARIATION,
    WORKSPACE_FOLDER = WORKSPACE_FOLDER
})
track = BassTrack
audioPlugin({plugin = "SynthBass", aliasName = "Bass", track = track})
audioPlugin({plugin = "Sequencer", track = track})
audioPlugin({plugin = "EffectGainVolume", aliasName = "TrackFx", track = track})
audioPlugin({
    plugin = "SerializeTrack",
    track = track,
    filename = "bass",
    MAX_VARIATION = ____exports.MAX_VARIATION,
    WORKSPACE_FOLDER = WORKSPACE_FOLDER
})
track = Fm1Track
audioPlugin({plugin = "SynthFmDrum", aliasName = "FmDrum", track = track})
audioPlugin({plugin = "Sequencer", track = track})
audioPlugin({plugin = "EffectGainVolume", aliasName = "TrackFx", track = track})
audioPlugin({
    plugin = "SerializeTrack",
    track = track,
    filename = "fmDrum",
    MAX_VARIATION = ____exports.MAX_VARIATION,
    WORKSPACE_FOLDER = WORKSPACE_FOLDER
})
track = SynthTrack
audioPlugin({plugin = "SynthHybrid", aliasName = "Synth", track = track})
audioPlugin({plugin = "Sequencer", track = track})
audioPlugin({plugin = "EffectGainVolume", aliasName = "TrackFx", track = track})
audioPlugin({
    plugin = "SerializeTrack",
    track = track,
    filename = "synth",
    MAX_VARIATION = ____exports.MAX_VARIATION,
    WORKSPACE_FOLDER = WORKSPACE_FOLDER
})
audioPlugin({plugin = "SampleSequencer", track = SampleTrack})
audioPlugin({
    plugin = "SerializeTrack",
    track = SampleTrack,
    filename = "sampleSeq",
    MAX_VARIATION = ____exports.MAX_VARIATION,
    WORKSPACE_FOLDER = WORKSPACE_FOLDER
})
track = PercTrack
audioPlugin({plugin = "SynthPerc", aliasName = "Perc", track = track})
audioPlugin({plugin = "Sequencer", track = track})
audioPlugin({plugin = "EffectGainVolume", aliasName = "TrackFx", track = track})
audioPlugin({
    plugin = "SerializeTrack",
    track = track,
    filename = "perc",
    MAX_VARIATION = ____exports.MAX_VARIATION,
    WORKSPACE_FOLDER = WORKSPACE_FOLDER
})
audioPlugin({plugin = "Mixer6", aliasName = "Mixer"})
audioPlugin({plugin = "EffectGainVolume", aliasName = "Volume"})
audioPlugin({plugin = "AudioOutputPulse", aliasName = "AudioOutput"})
audioPlugin({plugin = "SerializeTrack", filename = "master", MAX_VARIATION = ____exports.MAX_VARIATION, WORKSPACE_FOLDER = WORKSPACE_FOLDER})
audioPlugin({plugin = "Tempo"})
autoSave(500)
return ____exports
