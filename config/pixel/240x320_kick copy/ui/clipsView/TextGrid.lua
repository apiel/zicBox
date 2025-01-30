--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Keymaps = require("config.libs.components.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____TextGrid = require("config.libs.components.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____VisibilityContext = require("config.libs.components.VisibilityContext")
local VisibilityContext = ____VisibilityContext.VisibilityContext
local ____constants = require("config.pixel.240x320_kick copy.ui.constants")
local KeyInfoPosition = ____constants.KeyInfoPosition
function ____exports.TextGridClips()
    return React.createElement(
        TextGrid,
        {position = KeyInfoPosition, rows = {"&icon::toggle::rect &icon::arrowUp::filled ...", "&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled"}},
        React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 0}),
        React.createElement(Keymaps, {keys = {{key = "e", action = "contextToggle:254:1:0"}, {key = "a", action = "incGroup:-1"}, {key = "d", action = "incGroup:+1"}}})
    )
end
function ____exports.TextGridClipsShifted()
    return React.createElement(
        TextGrid,
        {position = KeyInfoPosition, rows = {"Menu Save ^...", "&icon::trash Back &icon::play::filled"}},
        React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 1}),
        React.createElement(Keymaps, {keys = {{key = "d", action = "playPause"}, {key = "e", action = "contextToggle:254:1:0"}, {key = "q", action = "setView:Menu"}}})
    )
end
return ____exports
