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
local drum23Track = {
    id = Drum23Track,
    plugins = {
        audioPlugin("SynthDrum23", {alias = "Drum23"}),
        audioPlugin("Sequencer"),
        audioPlugin("EffectDistortion2", {alias = "Distortion"}),
        audioPlugin("EffectFilterMultiMode", {alias = "MMFilter", cutoffStringFormat = ____exports.cutoffStringFormat}),
        audioPlugin("EffectGainVolume", {alias = "TrackFx"}),
        audioPlugin("SerializeTrack", {filename = "drum23", maxVariation = ____exports.maxVariation, workspaceFolder = workspaceFolder})
    }
}
local bassTrack = {
    id = BassTrack,
    plugins = {
        audioPlugin("SynthBass", {alias = "Bass"}),
        audioPlugin("Sequencer"),
        audioPlugin("EffectGainVolume", {alias = "TrackFx"}),
        audioPlugin("SerializeTrack", {filename = "bass", maxVariation = ____exports.maxVariation, workspaceFolder = workspaceFolder})
    }
}
local fm1Track = {
    id = Fm1Track,
    plugins = {
        audioPlugin("SynthFmDrum", {alias = "FmDrum"}),
        audioPlugin("Sequencer"),
        audioPlugin("EffectGainVolume", {alias = "TrackFx"}),
        audioPlugin("SerializeTrack", {filename = "fmDrum", maxVariation = ____exports.maxVariation, workspaceFolder = workspaceFolder})
    }
}
local synthTrack = {
    id = SynthTrack,
    plugins = {
        audioPlugin("SynthHybrid", {alias = "Synth"}),
        audioPlugin("Sequencer"),
        audioPlugin("EffectGainVolume", {alias = "TrackFx"}),
        audioPlugin("SerializeTrack", {filename = "synth", maxVariation = ____exports.maxVariation, workspaceFolder = workspaceFolder})
    }
}
local sampleTrack = {
    id = SampleTrack,
    plugins = {
        audioPlugin("SampleSequencer"),
        audioPlugin("Sequencer"),
        audioPlugin("EffectGainVolume", {alias = "TrackFx"}),
        audioPlugin("SerializeTrack", {filename = "sampleSeq", maxVariation = ____exports.maxVariation, workspaceFolder = workspaceFolder})
    }
}
local percTrack = {
    id = PercTrack,
    plugins = {
        audioPlugin("SynthPerc", {alias = "Perc"}),
        audioPlugin("Sequencer"),
        audioPlugin("EffectGainVolume", {alias = "TrackFx"}),
        audioPlugin("SerializeTrack", {filename = "perc", maxVariation = ____exports.maxVariation, workspaceFolder = workspaceFolder})
    }
}
local masterTrack = {
    id = 0,
    plugins = {
        audioPlugin("Mixer6", {alias = "Mixer"}),
        audioPlugin("EffectGainVolume", {alias = "Volume"}),
        audioPlugin("AudioOutputPulse", {alias = "AudioOutput"}),
        audioPlugin("SerializeTrack", {filename = "master", maxVariation = ____exports.maxVariation, workspaceFolder = workspaceFolder}),
        audioPlugin("Tempo")
    }
}
applyZic({{AUDIO = jsonStringify({tracks = {
    drum23Track,
    bassTrack,
    fm1Track,
    synthTrack,
    sampleTrack,
    percTrack,
    masterTrack
}, autoSave = 500})}})
return ____exports
