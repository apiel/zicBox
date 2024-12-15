zic("DEBUG", "true")

local buildDir = IS_RPI and "build/arm" or "build/x86"

zic("AUDIO_PLUGIN_ALIAS", "AudioOutput ./plugins/audio/" .. buildDir .. "/libzic_AudioOutputPulse.so")
zic("AUDIO_PLUGIN_ALIAS", "SynthDrum23 ./plugins/audio/" .. buildDir .. "/libzic_SynthDrum23.so")
zic("AUDIO_PLUGIN_ALIAS", "Sequencer ./plugins/audio/" .. buildDir .. "/libzic_Sequencer.so")
zic("AUDIO_PLUGIN_ALIAS", "EffectDistortion ./plugins/audio/" .. buildDir .. "/libzic_EffectDistortion.so")
zic("AUDIO_PLUGIN_ALIAS", "EffectDistortion2 ./plugins/audio/" .. buildDir .. "/libzic_EffectDistortion2.so")
zic("AUDIO_PLUGIN_ALIAS", "EffectFilterMultiMode ./plugins/audio/" .. buildDir .. "/libzic_EffectFilterMultiMode.so")
zic("AUDIO_PLUGIN_ALIAS", "EffectSampleRateReducer ./plugins/audio/" .. buildDir .. "/libzic_EffectSampleRateReducer.so")
zic("AUDIO_PLUGIN_ALIAS", "EffectGainVolume ./plugins/audio/" .. buildDir .. "/libzic_EffectGainVolume.so")
zic("AUDIO_PLUGIN_ALIAS", "EffectDelay ./plugins/audio/" .. buildDir .. "/libzic_EffectDelay.so")
zic("AUDIO_PLUGIN_ALIAS", "Mixer2 ./plugins/audio/" .. buildDir .. "/libzic_Mixer2.so")
zic("AUDIO_PLUGIN_ALIAS", "Mixer4 ./plugins/audio/" .. buildDir .. "/libzic_Mixer4.so")
zic("AUDIO_PLUGIN_ALIAS", "SerializeTrack ./plugins/audio/" .. buildDir .. "/libzic_SerializeTrack.so")
zic("AUDIO_PLUGIN_ALIAS", "SynthFM ./plugins/audio/" .. buildDir .. "/libzic_SynthFM2.so")
zic("AUDIO_PLUGIN_ALIAS", "Tempo ./plugins/audio/" .. buildDir .. "/libzic_Tempo.so")

zic("AUDIO_PLUGIN", "Tempo")

-- Tracks

local drum23Track = require("config/pixel/audio/drum23_track")
local fmTrack = require("config/pixel/audio/fm_track")

drum23Track(1)
drum23Track(2)
drum23Track(3)
fmTrack(4)
-- fmTrack(5)

-- Mixers

zic("AUDIO_PLUGIN", "MixerDrum Mixer4")
zic("TRACK_TARGET", "9")

zic("AUDIO_PLUGIN", "SerializeTrack")
zic("TRACK", "9")
zic("FILEPATH", "serializedPixel/mixer_drum.cfg")

zic("AUDIO_PLUGIN", "MixerSynth Mixer4")
zic("TRACK_START", "5")
zic("TRACK_TARGET", "10")

zic("AUDIO_PLUGIN", "SerializeTrack")
zic("TRACK", "10")
zic("FILEPATH", "serializedPixel/mixer_synth.cfg")

zic("AUDIO_PLUGIN", "MixerMaster Mixer2")
zic("TRACK_A", "9")
zic("TRACK_B", "10")
zic("VALUE", "50")
zic("TRACK_TARGET", "0")

zic("AUDIO_PLUGIN", "MasterVolume EffectGainVolume")
zic("AUDIO_PLUGIN", "AudioOutput")

zic("AUDIO_PLUGIN", "SerializeTrack_Master SerializeTrack")
zic("TRACK", "0")
zic("FILEPATH", "serializedPixel/master.cfg")

zic("AUTO_SAVE", "500")