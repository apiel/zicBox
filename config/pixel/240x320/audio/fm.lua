--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local ____audio = require("config.libs.audio")
local plugin = ____audio.plugin
local ____constants = require("config.pixel.240x320.audio.constants")
local MAX_VARIATION = ____constants.MAX_VARIATION
local STRING_CUTOFF_FORMAT = ____constants.STRING_CUTOFF_FORMAT
function ____exports.fm(track)
    plugin("SynthFM", {{name = "FM", track = track}})
    plugin("Sequencer", {{track = track}})
    plugin("MMFilter EffectFilterMultiMode", {{track = track, STRING_CUTOFF_FORMAT = STRING_CUTOFF_FORMAT}})
    plugin("Bitcrusher EffectSampleRateReducer", {{track = track}})
    plugin("Delay EffectDelay", {{track = track}})
    plugin(
        "SerializeTrack",
        {{
            track = track,
            filename = "track_" .. tostring(track),
            MAX_VARIATION = MAX_VARIATION
        }}
    )
    plugin("Volume EffectGainVolume", {{track = track}})
end
return ____exports
