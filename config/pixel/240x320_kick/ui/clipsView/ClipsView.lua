--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local ColorTrack1 = ____constants.ColorTrack1
local ColorTrack2 = ____constants.ColorTrack2
local ColorTrack3 = ____constants.ColorTrack3
local ColorTrack4 = ____constants.ColorTrack4
local ColorTrack5 = ____constants.ColorTrack5
local Drum23Track = ____constants.Drum23Track
local FmTrack = ____constants.FmTrack
local HiHatTrack = ____constants.HiHatTrack
local SampleTrack = ____constants.SampleTrack
local SnareTrack = ____constants.SnareTrack
local ____Clips = require("config.pixel.240x320_kick.ui.clipsView.Clips")
local Clips = ____Clips.Clips
local ____TextGrid = require("config.pixel.240x320_kick.ui.clipsView.TextGrid")
local TextGridClips = ____TextGrid.TextGridClips
local TextGridClipsShifted = ____TextGrid.TextGridClipsShifted
local ____Values = require("config.pixel.240x320_kick.ui.clipsView.Values")
local Drum23Values = ____Values.Drum23Values
local SampleValues = ____Values.SampleValues
function ____exports.ClipsView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(TextGridClips, nil),
        React.createElement(TextGridClipsShifted, nil),
        React.createElement(Clips, {color = ColorTrack1, track = Drum23Track, group = 0, title = "Kick"}),
        React.createElement(Clips, {color = ColorTrack2, track = FmTrack, group = 1, title = "Fm"}),
        React.createElement(Clips, {color = ColorTrack3, track = SnareTrack, group = 2, title = "Snare"}),
        React.createElement(Clips, {color = ColorTrack4, track = HiHatTrack, group = 3, title = "Hihat"}),
        React.createElement(Clips, {color = ColorTrack5, track = SampleTrack, group = 4, title = "Sample"}),
        React.createElement(Drum23Values, {group = 0, track = Drum23Track}),
        React.createElement(SampleValues, {group = 4, track = SampleTrack})
    )
end
return ____exports
