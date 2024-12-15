local function drum23Track(track)
    zic("AUDIO_PLUGIN", "SynthDrum23")
    zic("TRACK", track)

    zic("AUDIO_PLUGIN", "Sequencer")
    zic("TRACK", track)

    zic("AUDIO_PLUGIN", "Distortion EffectDistortion2")
    zic("TRACK", track)

    zic("AUDIO_PLUGIN", "MMFilter EffectFilterMultiMode")
    zic("TRACK", track)

    zic("AUDIO_PLUGIN", "SerializeTrack")
    zic("TRACK", track)
    zic("FILEPATH", "serializedPixel/track_" .. track .. ".cfg")

    zic("AUDIO_PLUGIN", "Volume EffectGainVolume")
    zic("TRACK", track)
end

return drum23Track