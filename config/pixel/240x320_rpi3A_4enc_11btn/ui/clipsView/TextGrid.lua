--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Keymaps = require("config.libs.components.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____TextGrid = require("config.libs.components.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____VisibilityContext = require("config.libs.components.VisibilityContext")
local VisibilityContext = ____VisibilityContext.VisibilityContext
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local btn1 = ____constants.btn1
local btn5 = ____constants.btn5
local btn7 = ____constants.btn7
local btnShift = ____constants.btnShift
local KeyInfoPosition = ____constants.KeyInfoPosition
function ____exports.TextGridClips()
    return React.createElement(
        TextGrid,
        {position = KeyInfoPosition, rows = {"&icon::toggle::rect &icon::arrowUp::filled ...", "&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled"}},
        React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 0}),
        React.createElement(Keymaps, {keys = {{key = btnShift, action = "contextToggle:254:1:0"}, {key = btn5, action = "incGroup:-1"}, {key = btn7, action = "incGroup:+1"}}})
    )
end
function ____exports.TextGridClipsShifted()
    return React.createElement(
        TextGrid,
        {position = KeyInfoPosition, rows = {"Menu Save ^...", "&icon::trash Back &icon::play::filled"}},
        React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 1}),
        React.createElement(Keymaps, {keys = {{key = btn7, action = "playPause"}, {key = btnShift, action = "contextToggle:254:1:0"}, {key = btn1, action = "setView:Menu"}}})
    )
end
return ____exports
