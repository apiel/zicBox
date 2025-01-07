--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____Text = require("config.libs.components.Text")
local Text = ____Text.Text
local ____View = require("config.libs.components.View")
local View = ____View.View
function ____exports.TapeView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(Text, {text = "view: " .. name, position = {0, 0, 240, 240}})
    )
end
return ____exports
