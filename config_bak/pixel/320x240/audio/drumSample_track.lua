local audio = require("config/pixel/libs/audio")

local function synth(track)
    audio.plugin("DrumSample", { TRACK = track })
    audio.plugin("Sequencer", { TRACK = track })
    audio.plugin("SerializeTrack", { TRACK = track, FILENAME = "track_" .. track })
    audio.plugin("EffectGainVolume", { NAME = "Volume", TRACK = track })
end

return synth