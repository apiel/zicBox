--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Keymaps = require("config.libs.nativeComponents.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____VisibilityContext = require("config.libs.nativeComponents.VisibilityContext")
local VisibilityContext = ____VisibilityContext.VisibilityContext
local ____TextGrid = require("config.libs.tsComponents.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____constants = require("config.pixel.240x320_drums.ui.constants")
local KeyInfoPosition = ____constants.KeyInfoPosition
function ____exports.TextGridSeq()
    return React.createElement(
        TextGrid,
        {bounds = KeyInfoPosition, rows = {"&icon::arrowUp::filled &icon::toggle::rect ...", "&icon::arrowDown::filled Synth &icon::musicNote::pixelated"}},
        React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = 0}),
        React.createElement(Keymaps, {keys = {
            {key = "q", action = "incGroup:-1"},
            {key = "e", action = "contextToggle:254:1:0"},
            {key = "a", action = "incGroup:+1"},
            {key = "s", action = "setView:Drum23"},
            {key = "d", action = "noteOn:Drum23:60"}
        }})
    )
end
return ____exports
