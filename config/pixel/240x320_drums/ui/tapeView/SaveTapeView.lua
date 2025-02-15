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
local ____constants = require("config.pixel.240x320_drums.ui.constants")
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
            {bounds = {0, 40, ScreenWidth, 200}, redirect_view = "Tape", done_data = "Tape SAVE"},
            React.createElement(Keymaps, {keys = {
                {key = "q", action = ".type"},
                {key = "w", action = ".up"},
                {key = "e", action = ".cancel"},
                {key = "s", action = ".down"},
                {key = "a", action = ".left"},
                {key = "d", action = ".right"}
            }})
        ),
        React.createElement(TextGrid, {bounds = KeyInfoPosition, rows = {"Type &icon::arrowUp::filled Cancel", "&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled"}})
    )
end
return ____exports
