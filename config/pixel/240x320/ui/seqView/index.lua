--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____HiddenValue = require("config.libs.components.HiddenValue")
local HiddenValue = ____HiddenValue.HiddenValue
local ____Keymap = require("config.libs.components.Keymap")
local Keymap = ____Keymap.Keymap
local ____Sample = require("config.libs.components.Sample")
local Sample = ____Sample.Sample
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____constants = require("config.pixel.240x320.ui.constants")
local ColorTrack1 = ____constants.ColorTrack1
local ColorTrack2 = ____constants.ColorTrack2
local ColorTrack3 = ____constants.ColorTrack3
local ColorTrack4 = ____constants.ColorTrack4
local ColorTrack5 = ____constants.ColorTrack5
local ColorTrack6 = ____constants.ColorTrack6
local ColorTrack7 = ____constants.ColorTrack7
local ColorTrack8 = ____constants.ColorTrack8
local ScreenWidth = ____constants.ScreenWidth
local ____ProgressBar = require("config.pixel.240x320.ui.components.ProgressBar")
local ProgressBar = ____ProgressBar.ProgressBar
local ____ValueBpm = require("config.pixel.240x320.ui.components.ValueBpm")
local ValueBpm = ____ValueBpm.ValueBpm
local ____Row = require("config.pixel.240x320.ui.seqView.Row")
local RowDrum23 = ____Row.RowDrum23
local RowMonoSample = ____Row.RowMonoSample
local ____TextGrid = require("config.pixel.240x320.ui.seqView.TextGrid")
local TextGridProgressBar = ____TextGrid.TextGridProgressBar
local TextGridProgressBarShifted = ____TextGrid.TextGridProgressBarShifted
local TextGridTrack = ____TextGrid.TextGridTrack
local TextGridTrackShifted = ____TextGrid.TextGridTrackShifted
local TextGridTrackStepEdit = ____TextGrid.TextGridTrackStepEdit
local ____Drum23Values = require("config.pixel.240x320.ui.seqView.values.Drum23Values")
local Drum23Values = ____Drum23Values.Drum23Values
local ____MasterValues = require("config.pixel.240x320.ui.seqView.values.MasterValues")
local MasterValues = ____MasterValues.MasterValues
local ____MonoSampleValues = require("config.pixel.240x320.ui.seqView.values.MonoSampleValues")
local MonoSampleValues = ____MonoSampleValues.MonoSampleValues
function ____exports.SeqView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(
            HiddenValue,
            nil,
            React.createElement(Keymap, {key = "w", action = "incGroup:-1", context = "254:0"}),
            React.createElement(Keymap, {key = "s", action = "incGroup:+1", context = "254:0"})
        ),
        React.createElement(TextGridProgressBar, nil),
        React.createElement(TextGridProgressBarShifted, nil),
        React.createElement(TextGridTrack, nil),
        React.createElement(TextGridTrackStepEdit, nil),
        React.createElement(TextGridTrackShifted, nil),
        React.createElement(ValueBpm, nil),
        React.createElement(Sample, {position = {0, 4, ScreenWidth, 70}, track = 4, plugin = "MonoSample SAMPLE_BUFFER", loop_line_color = "tertiary"}),
        React.createElement(ProgressBar, {y = 160, volume_plugin = "MasterVolume VOLUME", group = 0}),
        React.createElement(RowDrum23, {group = 1, track = 1, name_color = ColorTrack1, select_menu_context = "11"}),
        React.createElement(RowDrum23, {group = 2, track = 2, name_color = ColorTrack2, select_menu_context = "12"}),
        React.createElement(RowMonoSample, {group = 3, track = 3, name_color = ColorTrack3, select_menu_context = "13"}),
        React.createElement(RowMonoSample, {group = 4, track = 4, name_color = ColorTrack4, select_menu_context = "14"}),
        React.createElement(RowMonoSample, {group = 5, track = 5, name_color = ColorTrack5, select_menu_context = "15"}),
        React.createElement(RowMonoSample, {group = 6, track = 6, name_color = ColorTrack6, select_menu_context = "16"}),
        React.createElement(RowMonoSample, {group = 7, track = 7, name_color = ColorTrack7, select_menu_context = "17"}),
        React.createElement(RowMonoSample, {group = 8, track = 8, name_color = ColorTrack8, select_menu_context = "18"}),
        React.createElement(MasterValues, nil),
        React.createElement(Drum23Values, {group = 1, track = 1, context = 11}),
        React.createElement(Drum23Values, {group = 2, track = 2, context = 12}),
        React.createElement(MonoSampleValues, {group = 3, track = 3, context = 13}),
        React.createElement(MonoSampleValues, {group = 4, track = 4, context = 14}),
        React.createElement(MonoSampleValues, {group = 5, track = 5, context = 15}),
        React.createElement(MonoSampleValues, {group = 6, track = 6, context = 16}),
        React.createElement(MonoSampleValues, {group = 7, track = 7, context = 17}),
        React.createElement(MonoSampleValues, {group = 8, track = 8, context = 18})
    )
end
return ____exports
