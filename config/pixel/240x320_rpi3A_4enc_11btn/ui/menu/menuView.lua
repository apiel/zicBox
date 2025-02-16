--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____List = require("config.libs.nativeComponents.List")
local List = ____List.List
local ____View = require("config.libs.nativeComponents.View")
local View = ____View.View
local ____TextGrid = require("config.libs.tsComponents.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local btn1 = ____constants.btn1
local btn2 = ____constants.btn2
local btn5 = ____constants.btn5
local btn6 = ____constants.btn6
local btn7 = ____constants.btn7
local KeyInfoPosition = ____constants.KeyInfoPosition
local ScreenWidth = ____constants.ScreenWidth
function ____exports.MenuView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(List, {bounds = {0, 0, ScreenWidth, 280}, items = {"Tape", "Workspaces", "Shutdown"}, keys = {{key = btn1, action = ".setView"}, {key = btn2, action = ".up"}, {key = btn5, action = ".setView"}, {key = btn6, action = ".down"}}}),
        React.createElement(TextGrid, {bounds = KeyInfoPosition, rows = {"  &icon::arrowUp::filled  ", "Select &icon::arrowDown::filled Exit"}, keys = {{key = btn7, action = "setView:Clips"}}})
    )
end
return ____exports
