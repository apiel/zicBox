local audio = require("config/pixel/libs/audio")

local function synth(track)
  audio.plugin("SynthFM", { NAME = "FM", TRACK = track })
  audio.plugin("Sequencer", { TRACK = track })
  audio.plugin("EffectFilterMultiMode", { NAME = "MMFilter", TRACK = track })
  audio.plugin("EffectSampleRateReducer", { NAME = "Bitcrusher", TRACK = track })
  audio.plugin("EffectDelay", { NAME = "Delay", TRACK = track })
  audio.plugin("SerializeTrack", { TRACK = track, FILEPATH = "serializedPixel/track_" .. track .. ".cfg" })
  audio.plugin("EffectGainVolume", { NAME = "Volume", TRACK = track })
end

return synth
