--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Clips = require("config.libs.components.Clips")
local ClipsComponent = ____Clips.Clips
local ____Keymaps = require("config.libs.components.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____constants = require("config.pixel.240x320.ui.constants")
local W1_8 = ____constants.W1_8
function ____exports.Clips(____bindingPattern0)
    local group
    local track
    local color
    color = ____bindingPattern0.color
    track = ____bindingPattern0.track
    group = ____bindingPattern0.group
    return React.createElement(
        ClipsComponent,
        {
            bounds = {(track - 1) * W1_8, 15, W1_8 - 2, 200},
            plugin = "SerializeTrack",
            track = track,
            group = group,
            color = color,
            seq_plugin = "Sequencer"
        },
        React.createElement(Keymaps, {keys = {
            {key = "q", action = ".toggle", context = "254:0"},
            {key = "w", action = ".up", context = "254:0"},
            {key = "s", action = ".down", context = "254:0"},
            {key = "q", action = ".next", context = "254:1"},
            {key = "s", action = ".delete", context = "254:1"},
            {key = "d", action = ".save", context = "254:1"}
        }})
    )
end
return ____exports
