--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Text = require("config.libs.components.Text")
local Text = ____Text.Text
local ____constants = require("config.pixel.240x320_kick copy.ui.constants")
local ScreenWidth = ____constants.ScreenWidth
function ____exports.Title(____bindingPattern0)
    local title
    title = ____bindingPattern0.title
    return React.createElement(Text, {font_size = 16, text = title, position = {0, 20, ScreenWidth, 16}, centered = true})
end
return ____exports
