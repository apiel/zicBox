--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____List = require("config.libs.nativeComponents.List")
local List = ____List.List
local ____Text = require("config.libs.nativeComponents.Text")
local Text = ____Text.Text
function ____exports.Yo2()
    return React.createElement(
        React.Fragment,
        nil,
        React.createElement(Text, {fontSize = 16, text = "yo2", bounds = {0, 0, 100, 16}, centered = true}),
        React.createElement(List, {bounds = {0, 0, 123, 280}, items = {"yoyoyoy"}, keys = {{key = 0, action = ".setView"}}})
    )
end
return ____exports
