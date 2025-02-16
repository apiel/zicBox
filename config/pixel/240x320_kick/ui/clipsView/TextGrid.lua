--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____TextGrid = require("config.libs.tsComponents.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local KeyInfoPosition = ____constants.KeyInfoPosition
function ____exports.TextGridClips()
    return React.createElement(TextGrid, {bounds = KeyInfoPosition, rows = {"&icon::toggle::rect &icon::arrowUp::filled ...", "&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled"}, keys = {{key = "e", action = "contextToggle:254:1:0"}, {key = "a", action = "incGroup:-1"}, {key = "d", action = "incGroup:+1"}}, contextValue = 0})
end
function ____exports.TextGridClipsShifted()
    return React.createElement(TextGrid, {bounds = KeyInfoPosition, rows = {"Menu Save ^...", "&icon::trash Back &icon::play::filled"}, keys = {{key = "d", action = "playPause"}, {key = "e", action = "contextToggle:254:1:0"}, {key = "q", action = "setView:Menu"}}, contextValue = 1})
end
return ____exports
