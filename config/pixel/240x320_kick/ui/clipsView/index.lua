--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____ProgressBar = require("config.pixel.240x320_kick.ui.components.ProgressBar")
local ProgressBar = ____ProgressBar.ProgressBar
local ____ValueBpm = require("config.pixel.240x320_kick.ui.components.ValueBpm")
local ValueBpm = ____ValueBpm.ValueBpm
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local ColorTrack1 = ____constants.ColorTrack1
local ColorTrack2 = ____constants.ColorTrack2
local ColorTrack3 = ____constants.ColorTrack3
local ColorTrack4 = ____constants.ColorTrack4
local ColorTrack5 = ____constants.ColorTrack5
local ColorTrack6 = ____constants.ColorTrack6
local ColorTrack7 = ____constants.ColorTrack7
local ColorTrack8 = ____constants.ColorTrack8
local ____Clips = require("config.pixel.240x320_kick.ui.clipsView.Clips")
local Clips = ____Clips.Clips
local ____TextGrid = require("config.pixel.240x320_kick.ui.clipsView.TextGrid")
local TextGridClips = ____TextGrid.TextGridClips
local TextGridClipsShifted = ____TextGrid.TextGridClipsShifted
function ____exports.ClipsView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(ProgressBar, {y = 5}),
        React.createElement(ValueBpm, nil),
        React.createElement(TextGridClips, nil),
        React.createElement(TextGridClipsShifted, nil),
        React.createElement(Clips, {color = ColorTrack1, track = 1, group = 0}),
        React.createElement(Clips, {color = ColorTrack2, track = 2, group = 1}),
        React.createElement(Clips, {color = ColorTrack3, track = 3, group = 2}),
        React.createElement(Clips, {color = ColorTrack4, track = 4, group = 3}),
        React.createElement(Clips, {color = ColorTrack5, track = 5, group = 4}),
        React.createElement(Clips, {color = ColorTrack6, track = 6, group = 5}),
        React.createElement(Clips, {color = ColorTrack7, track = 7, group = 6}),
        React.createElement(Clips, {color = ColorTrack8, track = 8, group = 7})
    )
end
return ____exports
