--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____TextGrid = require("config.pixel.240x320_kick.ui.kickView.TextGrid")
local TextGridTape = ____TextGrid.TextGridTape
function ____exports.KickView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(TextGridTape, nil)
    )
end
return ____exports
