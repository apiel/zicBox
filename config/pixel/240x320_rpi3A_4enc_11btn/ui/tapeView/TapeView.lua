--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Keymap = require("config.libs.components.Keymap")
local Keymap = ____Keymap.Keymap
local ____Keymaps = require("config.libs.components.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____Tape = require("config.libs.components.Tape")
local Tape = ____Tape.Tape
local ____TextGrid = require("config.libs.components.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local btn2 = ____constants.btn2
local btn6 = ____constants.btn6
local KeyInfoPosition = ____constants.KeyInfoPosition
function ____exports.TapeView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(
            Tape,
            {position = {0, 0, 240, 280}, filename = "rec", bpm_value = "Tempo BPM", tape_plugin = "Tape PLAY_STOP"},
            React.createElement(Keymap, {key = "q", action = ".playStop"}),
            React.createElement(Keymap, {key = "a", action = ".left"}),
            React.createElement(Keymap, {key = "d", action = ".right"})
        ),
        React.createElement(
            TextGrid,
            {position = KeyInfoPosition, rows = {"&icon::play::filled Save &empty", "&icon::arrowLeft::filled Exit &icon::arrowRight::filled"}},
            React.createElement(Keymaps, {keys = {{key = btn2, action = "setView:SaveTape"}, {key = btn6, action = "setView:Menu"}}})
        )
    )
end
return ____exports
