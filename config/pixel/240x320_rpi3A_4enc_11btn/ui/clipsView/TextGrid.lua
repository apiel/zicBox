--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____TextGrid = require("config.libs.tsComponents.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local btn1 = ____constants.btn1
local btn5 = ____constants.btn5
local btn7 = ____constants.btn7
local btnShift = ____constants.btnShift
local KeyInfoPosition = ____constants.KeyInfoPosition
function ____exports.TextGridClips()
    return React.createElement(TextGrid, {bounds = KeyInfoPosition, rows = {"&icon::toggle::rect &icon::arrowUp::filled ...", "&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled"}, keys = {{key = btnShift, action = "contextToggle:254:1:0"}, {key = btn5, action = "incGroup:-1"}, {key = btn7, action = "incGroup:+1"}}, contextValue = 0})
end
function ____exports.TextGridClipsShifted()
    return React.createElement(TextGrid, {bounds = KeyInfoPosition, rows = {"Menu Save ^...", "&icon::trash Back &icon::play::filled"}, keys = {{key = btn7, action = "playPause"}, {key = btnShift, action = "contextToggle:254:1:0"}, {key = btn1, action = "setView:Menu"}}, contextValue = 1})
end
return ____exports
