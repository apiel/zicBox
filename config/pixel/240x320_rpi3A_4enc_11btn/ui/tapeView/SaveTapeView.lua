--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Keyboard = require("config.libs.components.Keyboard")
local Keyboard = ____Keyboard.Keyboard
local ____Keymaps = require("config.libs.components.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____TextGrid = require("config.libs.components.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local btn1 = ____constants.btn1
local btn2 = ____constants.btn2
local btn5 = ____constants.btn5
local btn6 = ____constants.btn6
local btn7 = ____constants.btn7
local btnShift = ____constants.btnShift
local KeyInfoPosition = ____constants.KeyInfoPosition
local ScreenWidth = ____constants.ScreenWidth
function ____exports.SaveTapeView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(
            Keyboard,
            {position = {0, 40, ScreenWidth, 200}, redirect_view = "Tape", done_data = "Tape SAVE"},
            React.createElement(Keymaps, {keys = {
                {key = btn1, action = ".type"},
                {key = btn2, action = ".up"},
                {key = btnShift, action = ".cancel"},
                {key = btn6, action = ".down"},
                {key = btn5, action = ".left"},
                {key = btn7, action = ".right"}
            }})
        ),
        React.createElement(TextGrid, {position = KeyInfoPosition, rows = {"Type &icon::arrowUp::filled Cancel", "&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled"}})
    )
end
return ____exports
