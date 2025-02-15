--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Keymaps = require("config.libs.components.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____TextGrid = require("config.libs.components.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____VisibilityContext = require("config.libs.components.VisibilityContext")
local VisibilityContext = ____VisibilityContext.VisibilityContext
local ____constants = require("config.pixel.240x320.ui.constants")
local KeyInfoPosition = ____constants.KeyInfoPosition
do
end
function ____exports.TextGridClips()
    return React.createElement(
        TextGrid,
        {bounds = KeyInfoPosition, rows = {"&icon::toggle::rect &icon::arrowUp::filled ...", "&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled"}},
        React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 0}),
        React.createElement(Keymaps, {keys = {{key = "e", action = "contextToggle:254:1:0"}, {key = "a", action = "incGroup:-1"}, {key = "d", action = "incGroup:+1"}}})
    )
end
function ____exports.TextGridClipsShifted()
    return React.createElement(
        TextGrid,
        {bounds = KeyInfoPosition, rows = {"Next &icon::play::filled ^...", "Home &icon::trash Save"}},
        React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 1}),
        React.createElement(Keymaps, {keys = {{key = "w", action = "playPause"}, {key = "e", action = "contextToggle:254:1:0"}, {key = "a", action = "setView:Home"}}})
    )
end
return ____exports
