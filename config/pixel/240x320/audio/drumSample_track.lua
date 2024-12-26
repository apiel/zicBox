local audio = require("config/pixel/libs/audio")

local function synth(track)
    audio.plugin("DrumSample", { TRACK = track })
    audio.plugin("Sequencer", { TRACK = track })
    audio.plugin("SerializeTrack", { TRACK = track, FILEPATH = "serializedPixel2/track_" .. track .. ".cfg" })
    audio.plugin("EffectSampleRateReducer", { NAME = "SampleRateReducer", TRACK = track })
    audio.plugin("EffectVolumeClipping", { NAME = "Volume", TRACK = track })
end

return synth
