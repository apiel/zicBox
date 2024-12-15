local function fmTrack(track)
    zic("AUDIO_PLUGIN", "FM SynthFM")
    zic("TRACK", track)

    zic("AUDIO_PLUGIN", "Sequencer")
    zic("TRACK", track)

    zic("AUDIO_PLUGIN", "MMFilter EffectFilterMultiMode")
    zic("TRACK", track)

    zic("AUDIO_PLUGIN", "Bitcrusher EffectSampleRateReducer")
    zic("TRACK", track)

    zic("AUDIO_PLUGIN", "Delay EffectDelay")
    zic("TRACK", track)

    zic("AUDIO_PLUGIN", "SerializeTrack")
    zic("TRACK", track)
    zic("FILEPATH", "serializedPixel/track_" .. track .. ".cfg")

    zic("AUDIO_PLUGIN", "Volume EffectGainVolume")
    zic("TRACK", track)
end

return fmTrack