--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Keymaps = require("config.libs.components.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____TextGrid = require("config.libs.components.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____VisibilityContext = require("config.libs.components.VisibilityContext")
local VisibilityContext = ____VisibilityContext.VisibilityContext
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local KeyInfoPosition = ____constants.KeyInfoPosition
function ____exports.TextGridSeq()
    return React.createElement(
        TextGrid,
        {position = KeyInfoPosition, rows = {"&icon::arrowUp::filled &icon::toggle::rect ...", "&icon::arrowDown::filled &icon::play::filled &icon::musicNote::pixelated"}},
        React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 0}),
        React.createElement(Keymaps, {keys = {
            {key = "q", action = "incGroup:-1"},
            {key = "e", action = "contextToggle:254:1:0"},
            {key = "a", action = "incGroup:+1"},
            {key = "s", action = "playPause"},
            {key = "d", action = "noteOn:Drum23:60"}
        }})
    )
end
return ____exports
