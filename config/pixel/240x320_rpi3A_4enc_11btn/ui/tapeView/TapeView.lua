--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____View = require("config.libs.nativeComponents.View")
local View = ____View.View
local ____TextGrid = require("config.libs.tsComponents.TextGrid")
local TextGrid = ____TextGrid.TextGrid
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
        React.createElement(TextGrid, {bounds = KeyInfoPosition, rows = {"&icon::play::filled Save &empty", "&icon::arrowLeft::filled Exit &icon::arrowRight::filled"}, keys = {{key = btn2, action = "setView:SaveTape"}, {key = btn6, action = "setView:Menu"}}})
    )
end
return ____exports
