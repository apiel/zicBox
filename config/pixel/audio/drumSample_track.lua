local audio = require("config/pixel/libs/audio")

local function synth(track)
    audio.plugin("DrumSample", { TRACK = track })
    audio.plugin("SerializeTrack", { TRACK = track, FILEPATH = "serializedPixel/track_" .. track .. ".cfg" })
    audio.plugin("EffectGainVolume", { NAME = "Volume", TRACK = track })
end

return synth
