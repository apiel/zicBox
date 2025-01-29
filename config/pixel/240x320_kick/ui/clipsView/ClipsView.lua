--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local BassTrack = ____constants.BassTrack
local ColorTrack1 = ____constants.ColorTrack1
local ColorTrack2 = ____constants.ColorTrack2
local Drum23Track = ____constants.Drum23Track
local ____Clips = require("config.pixel.240x320_kick.ui.clipsView.Clips")
local Clips = ____Clips.Clips
local ____TextGrid = require("config.pixel.240x320_kick.ui.clipsView.TextGrid")
local TextGridClips = ____TextGrid.TextGridClips
local TextGridClipsShifted = ____TextGrid.TextGridClipsShifted
local ____Values = require("config.pixel.240x320_kick.ui.clipsView.Values")
local Drum23Values = ____Values.Drum23Values
local MasterValues = ____Values.MasterValues
function ____exports.ClipsView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(TextGridClips, nil),
        React.createElement(TextGridClipsShifted, nil),
        React.createElement(Clips, {
            color = ColorTrack1,
            track = Drum23Track,
            group = 0,
            title = "Kick",
            group_all = 2
        }),
        React.createElement(Clips, {
            color = ColorTrack2,
            track = BassTrack,
            group = 1,
            title = "Bass",
            group_all = 2
        }),
        React.createElement(Drum23Values, {group = 0, track = Drum23Track}),
        React.createElement(MasterValues, {group = 2, track = 0})
    )
end
return ____exports
