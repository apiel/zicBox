local audio = require("config/pixel/libs/audio")

local function synth(track)
    audio.plugin("SynthDrum23", { TRACK = track })
    audio.plugin("Sequencer", { TRACK = track })
    audio.plugin("EffectDistortion2", { NAME = "Distortion", TRACK = track })
    audio.plugin("EffectFilterMultiMode", { NAME = "MMFilter", TRACK = track })
    audio.plugin("SerializeTrack", { TRACK = track, FILEPATH = "serializedPixel/track_" .. track .. ".cfg" })
    audio.plugin("EffectGainVolume", { NAME = "Volume", TRACK = track })
end

return synth
