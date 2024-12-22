local audio = require("config/pixel/libs/audio")
local drum23 = require("config/pixel/audio/drum23_track")
local drumSample = require("config/pixel/audio/drumSample_track")
local fm = require("config/pixel/audio/fm_track")

audio.pluginAlias("EffectDistortion", "libzic_EffectDistortion.so")
audio.pluginAlias("EffectDistortion2", "libzic_EffectDistortion2.so")
audio.pluginAlias("EffectFilterMultiMode", "libzic_EffectFilterMultiMode.so")
audio.pluginAlias("EffectSampleRateReducer", "libzic_EffectSampleRateReducer.so")
audio.pluginAlias("EffectGainVolume", "libzic_EffectGainVolume.so")
audio.pluginAlias("EffectVolumeDrive", "libzic_EffectVolumeDrive.so")
audio.pluginAlias("EffectDelay", "libzic_EffectDelay.so")
audio.pluginAlias("Mixer2", "libzic_Mixer2.so")
audio.pluginAlias("Mixer4", "libzic_Mixer4.so")
audio.pluginAlias("Mixer8", "libzic_Mixer8.so")
audio.pluginAlias("SerializeTrack", "libzic_SerializeTrack.so")
audio.pluginAlias("Tempo", "libzic_Tempo.so")
audio.pluginAlias("SynthFM", "libzic_SynthFM2.so")
audio.pluginAlias("Drum23", "libzic_SynthDrum23.so")
audio.pluginAlias("DrumSample", "libzic_SynthDrumSample.so")
audio.pluginAlias("Sequencer", "libzic_Sequencer.so")
audio.pluginAlias("AudioOutput", "libzic_AudioOutputPulse.so")

-- # Drum tracks
drumSample(11)
drumSample(12)
drumSample(13)
drumSample(14)
drumSample(15)
drumSample(16)
drumSample(17)
drumSample(18)
audio.plugin("Mixer8", { NAME = "MixerDrumSample",  TRACK_START = 11, TRACK = 1 })
audio.plugin("EffectVolumeDrive", { NAME = "VolumeDrive", TRACK = 1 })
audio.plugin("EffectFilterMultiMode", { NAME = "MMFilter", TRACK = 1 })
audio.plugin("SerializeTrack", { TRACK = 1, FILEPATH = "serializedPixel/track_1.cfg" })

drum23(2)
drum23(3)
drum23(4)

audio.plugin("Mixer4", { NAME = "MixerDrum", TRACK = 9 })
audio.plugin("SerializeTrack", { TRACK = 9, FILEPATH = "serializedPixel/mixer_drum.cfg" })

-- # FM tracks
fm(5)
fm(6)

audio.plugin("Mixer4", { NAME = "MixerSynth", TRACK_START = 5, TRACK = 10 })
audio.plugin("SerializeTrack", { TRACK = 10, FILEPATH = "serializedPixel/mixer_synth.cfg" })

-- # Master

audio.plugin("Mixer2", { NAME = "MixerMaster", TRACK_A = 9, TRACK_B = 10, VALUE = 50, TRACK = 0 })

audio.plugin("EffectGainVolume", { NAME = "MasterVolume" })
audio.plugin("AudioOutput")
audio.plugin("SerializeTrack", { FILEPATH = "serializedPixel/master.cfg" })

audio.plugin("Tempo")

audio.autoSave(500)
