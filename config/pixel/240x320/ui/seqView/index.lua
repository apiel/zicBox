--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____HiddenValue = require("config.libs.components.HiddenValue")
local HiddenValue = ____HiddenValue.HiddenValue
local ____Keymap = require("config.libs.components.Keymap")
local Keymap = ____Keymap.Keymap
local ____SeqProgressBar = require("config.libs.components.SeqProgressBar")
local SeqProgressBar = ____SeqProgressBar.SeqProgressBar
local ____Spectrogram = require("config.libs.components.Spectrogram")
local Spectrogram = ____Spectrogram.Spectrogram
local ____Value = require("config.libs.components.Value")
local Value = ____Value.Value
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
local KeyInfoPosition = ____constants.KeyInfoPosition
local ScreenWidth = ____constants.ScreenWidth
local W1_4 = ____constants.W1_4
local W3_4 = ____constants.W3_4
local ____Row = require("config.pixel.240x320.ui.seqView.Row")
local RowDrum23 = ____Row.RowDrum23
local RowDrumSample = ____Row.RowDrumSample
local ____TextGrid = require("config.pixel.240x320.ui.seqView.TextGrid")
local TextGridProgressBar = ____TextGrid.TextGridProgressBar
local TextGridProgressBarShifted = ____TextGrid.TextGridProgressBarShifted
local TextGridTrack = ____TextGrid.TextGridTrack
local TextGridTrackShifted = ____TextGrid.TextGridTrackShifted
local TextGridTrackStepEdit = ____TextGrid.TextGridTrackStepEdit
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
        React.createElement(Value, {
            value = "Tempo BPM",
            position = {W3_4, KeyInfoPosition[2], W1_4, 22},
            SHOW_LABEL_OVER_VALUE = 0,
            BAR_HEIGHT = 0,
            VALUE_FONT_SIZE = 16
        }),
        React.createElement(Spectrogram, {position = {0, 4, ScreenWidth, 70}, track = 0, data = "Spectrogram BUFFER", text = "Pixel"}),
        React.createElement(SeqProgressBar, {
            position = {0, 160, ScreenWidth, 5},
            seq_plugin = "Sequencer 1",
            track = 0,
            volume_plugin = "MasterVolume VOLUME",
            active_color = "#23a123",
            group = 0
        }),
        React.createElement(RowDrum23, {group = 1, track = 1, name_color = ColorTrack1, select_menu_context = "11"}),
        React.createElement(RowDrum23, {group = 2, track = 2, name_color = ColorTrack2, select_menu_context = "12"}),
        React.createElement(RowDrumSample, {group = 3, track = 3, name_color = ColorTrack3, select_menu_context = "13"}),
        React.createElement(RowDrumSample, {group = 4, track = 4, name_color = ColorTrack4, select_menu_context = "14"}),
        React.createElement(RowDrumSample, {group = 5, track = 5, name_color = ColorTrack5, select_menu_context = "15"}),
        React.createElement(RowDrumSample, {group = 6, track = 6, name_color = ColorTrack6, select_menu_context = "16"}),
        React.createElement(RowDrumSample, {group = 7, track = 7, name_color = ColorTrack7, select_menu_context = "17"}),
        React.createElement(RowDrumSample, {group = 8, track = 8, name_color = ColorTrack8, select_menu_context = "18"})
    )
end
return ____exports
