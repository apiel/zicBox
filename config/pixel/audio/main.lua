local core = require("config/pixel/libs/core")
local audio = require("config/pixel/libs/audio")
local drum23 = require("config/pixel/audio/drum23_track")

audio.pluginAlias("AudioOutput", "libzic_AudioOutputPulse.so")
audio.pluginAlias("SynthDrum23", "libzic_SynthDrum23.so")
audio.pluginAlias("Sequencer", "libzic_Sequencer.so")
audio.pluginAlias("EffectDistortion", "libzic_EffectDistortion.so")
audio.pluginAlias("EffectDistortion2", "libzic_EffectDistortion2.so")
audio.pluginAlias("EffectFilterMultiMode", "libzic_EffectFilterMultiMode.so")
audio.pluginAlias("EffectSampleRateReducer", "libzic_EffectSampleRateReducer.so")
audio.pluginAlias("EffectGainVolume", "libzic_EffectGainVolume.so")
audio.pluginAlias("EffectDelay", "libzic_EffectDelay.so")
audio.pluginAlias("Mixer2", "libzic_Mixer2.so")
audio.pluginAlias("Mixer4", "libzic_Mixer4.so")
audio.pluginAlias("SerializeTrack", "libzic_SerializeTrack.so")
audio.pluginAlias("Tempo", "libzic_Tempo.so")
audio.pluginAlias("SynthFM", "libzic_SynthFM2.so")

audio.plugin("Tempo")

-- # Drum tracks
drum23(1)
drum23(2)
drum23(3)

-- require("config/pixel/audio/main")
core.loadDustConfig("config/pixel/audio/main.cfg")
