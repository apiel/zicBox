--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____List = require("config.libs.nativeComponents.List")
local List = ____List.List
local ____View = require("config.libs.nativeComponents.View")
local View = ____View.View
function ____exports.Yo(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(List, {bounds = {0, 0, 123, 280}, items = {"Tape", "Workspaces", "Shutdown"}, keys = {{key = 0, action = ".setView"}}})
    )
end
return ____exports
