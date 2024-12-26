local audio = require("config/pixel/libs/audio")
local drum23 = require("config/pixel/240x320/audio/drum23_track")
local drumSample = require("config/pixel/240x320/audio/drumSample_track")
local fm = require("config/pixel/240x320/audio/fm_track")

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
drum23(1)
drum23(2)
drumSample(3)
drumSample(4)

audio.plugin("Mixer4", { NAME = "MixerDrum", TRACK = 9 })
audio.plugin("SerializeTrack", { TRACK = 9, FILEPATH = "serializedPixel2/mixer_drum.cfg" })

-- # FM tracks
drumSample(5)
drumSample(6)
drumSample(7)
drumSample(8)
-- fm(8)

audio.plugin("Mixer4", { NAME = "MixerSynth", TRACK_START = 5, TRACK = 10 })
audio.plugin("SerializeTrack", { TRACK = 10, FILEPATH = "serializedPixel2/mixer_synth.cfg" })

-- # Master

audio.plugin("Mixer2", { NAME = "MixerMaster", TRACK_A = 9, TRACK_B = 10, VALUE = 50, TRACK = 0 })

audio.plugin("EffectFilterMultiMode", { NAME = "MasterFilter" })
audio.plugin("EffectGainVolume", { NAME = "MasterVolume" })
audio.plugin("AudioOutput")
audio.plugin("SerializeTrack", { FILEPATH = "serializedPixel2/master.cfg" })

audio.plugin("Tempo")

audio.autoSave(500)
