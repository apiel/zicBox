--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Text = require("config.libs.nativeComponents.Text")
local Text = ____Text.Text
local ____constants = require("config.pixel.240x320_drums.ui.constants")
local ScreenWidth = ____constants.ScreenWidth
function ____exports.Title(____bindingPattern0)
    local title
    title = ____bindingPattern0.title
    return React.createElement(Text, {fontSize = 16, text = title, bounds = {0, 20, ScreenWidth, 16}, centered = true})
end
return ____exports
