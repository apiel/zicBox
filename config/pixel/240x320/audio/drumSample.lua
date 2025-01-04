--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local ____audio = require("config.libs.audio")
local plugin = ____audio.plugin
local ____constants = require("config.pixel.240x320.audio.constants")
local MAX_VARIATION = ____constants.MAX_VARIATION
function ____exports.drumSample(track)
    plugin("DrumSample", {{track = track}})
    plugin("Sequencer", {{track = track}})
    plugin(
        "SerializeTrack",
        {{
            track = track,
            filename = "track_" .. tostring(track),
            MAX_VARIATION = MAX_VARIATION
        }}
    )
    plugin("EffectSampleRateReducer", {{name = "SampleRateReducer", track = track}})
    plugin("EffectVolumeClipping", {{name = "Volume", track = track}})
end
return ____exports
