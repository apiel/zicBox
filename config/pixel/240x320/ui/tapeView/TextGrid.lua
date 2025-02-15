--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Keymaps = require("config.libs.components.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____TextGrid = require("config.libs.components.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____constants = require("config.pixel.240x320.ui.constants")
local KeyInfoPosition = ____constants.KeyInfoPosition
do
end
function ____exports.TextGridTape()
    return React.createElement(
        TextGrid,
        {bounds = KeyInfoPosition, rows = {"&icon::play::filled Save &empty", "&icon::arrowLeft::filled Home &icon::arrowRight::filled"}},
        React.createElement(Keymaps, {keys = {{key = "w", action = "setView:SaveTape"}, {key = "s", action = "setView:Home"}}})
    )
end
return ____exports
