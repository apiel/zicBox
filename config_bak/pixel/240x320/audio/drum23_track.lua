local audio = require("config/pixel/libs/audio")

local function synth(track)
    audio.plugin("Drum23", { TRACK = track })
    audio.plugin("Sequencer", { TRACK = track })
    audio.plugin("EffectDistortion2", { NAME = "Distortion", TRACK = track })
    audio.plugin("EffectFilterMultiMode", { NAME = "MMFilter", TRACK = track, STRING_CUTOFF_FORMAT = StringCuttoffFormat })
    audio.plugin("SerializeTrack", { TRACK = track, FILENAME = "track_" .. track, MAX_VARIATION = MaxVariation })
    audio.plugin("EffectGainVolume", { NAME = "Volume", TRACK = track })
end

return synth
