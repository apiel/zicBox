--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Keymap = require("config.libs.components.Keymap")
local Keymap = ____Keymap.Keymap
local ____Keymaps = require("config.libs.components.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____TextGrid = require("config.libs.components.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____VisibilityContext = require("config.libs.components.VisibilityContext")
local VisibilityContext = ____VisibilityContext.VisibilityContext
local ____VisibilityGroup = require("config.libs.components.VisibilityGroup")
local VisibilityGroup = ____VisibilityGroup.VisibilityGroup
local ____constants = require("config.pixel.240x320.ui.constants")
local KeyInfoPosition = ____constants.KeyInfoPosition
do
end
function ____exports.TextGridProgressBar()
    return React.createElement(
        TextGrid,
        {position = KeyInfoPosition, rows = {"&icon::play::filled &icon::arrowUp::filled ...", "&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled"}},
        React.createElement(VisibilityGroup, {condition = "SHOW_WHEN", group = 0}),
        React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 0}),
        React.createElement(Keymap, {key = "q", action = "playPause"}),
        React.createElement(Keymap, {key = "e", action = "contextToggle:254:1:0"})
    )
end
function ____exports.TextGridProgressBarShifted()
    return React.createElement(
        TextGrid,
        {position = KeyInfoPosition, rows = {"Menu &icon::play::filled ^...", "Clips ? Save"}},
        React.createElement(VisibilityGroup, {condition = "SHOW_WHEN", group = 0}),
        React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 1}),
        React.createElement(Keymaps, {keys = {{key = "q", action = "setView:Menu", action2 = "contextToggle:254:1:0"}, {key = "w", action = "playPause"}, {key = "a", action = "setView:Clips"}, {key = "e", action = "contextToggle:254:1:0"}}})
    )
end
return ____exports
