--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Clips = require("config.libs.components.Clips")
local ClipsComponent = ____Clips.Clips
local ____Keymaps = require("config.libs.components.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____Text = require("config.libs.components.Text")
local Text = ____Text.Text
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local ScreenWidth = ____constants.ScreenWidth
function ____exports.Clips(____bindingPattern0)
    local title
    local group
    local track
    local color
    color = ____bindingPattern0.color
    track = ____bindingPattern0.track
    group = ____bindingPattern0.group
    title = ____bindingPattern0.title
    local w = ScreenWidth / 5
    return React.createElement(
        React.Fragment,
        nil,
        React.createElement(Text, {text = title, position = {(track - 1) * w, 2, w, 8}, centered = true, color = color}),
        React.createElement(
            ClipsComponent,
            {
                position = {(track - 1) * w, 15, w - 2, 200},
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
    )
end
return ____exports
