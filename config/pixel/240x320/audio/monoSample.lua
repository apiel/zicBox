--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local ____audio = require("config.libs.audio")
local plugin = ____audio.plugin
local ____constants = require("config.pixel.240x320.audio.constants")
local MAX_VARIATION = ____constants.MAX_VARIATION
local STRING_CUTOFF_FORMAT = ____constants.STRING_CUTOFF_FORMAT
local WORKSPACE_FOLDER = ____constants.WORKSPACE_FOLDER
function ____exports.monoSample(track)
    plugin("MonoSample", {{track = track}})
    plugin("Sequencer", {{track = track}})
    plugin(
        "SerializeTrack",
        {{
            track = track,
            filename = "track_" .. tostring(track),
            MAX_VARIATION = MAX_VARIATION,
            WORKSPACE_FOLDER = WORKSPACE_FOLDER
        }}
    )
    plugin("MMFilter EffectFilterMultiMode", {{track = track, STRING_CUTOFF_FORMAT = STRING_CUTOFF_FORMAT}})
    plugin("SampleRateReducer EffectSampleRateReducer", {{track = track}})
    plugin("Volume EffectVolumeClipping", {{track = track}})
end
return ____exports
