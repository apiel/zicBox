--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local ____Adsr = require("config.libs.components.Adsr")
local Adsr = ____Adsr.Adsr
local ____core = require("config.libs.core")
local applyZic = ____core.applyZic
local React = require("config.libs.react")
local function GroupContainer(____bindingPattern0)
    local a
    a = ____bindingPattern0.a
    return {{COMPONENT = "GroupContainer"}, {A = a}}
end
local function List(____bindingPattern0)
    local b
    b = ____bindingPattern0.b
    return {{COMPONENT = "List"}, {B = b}}
end
local function Item(____bindingPattern0)
    local k
    k = ____bindingPattern0.k
    return {{ITEM = k}}
end
local function MyCustomComponent()
    return React.createElement(
        React.Fragment,
        nil,
        React.createElement(List, {b = "bla"}),
        React.createElement(List, {b = "blo"})
    )
end
applyZic(React.createElement(
    GroupContainer,
    {a = "test"},
    React.createElement(List, {b = "test123"}),
    React.createElement(
        List,
        {b = "test456"},
        React.createElement(Item, {k = "yo"})
    ),
    React.createElement(MyCustomComponent, nil),
    React.createElement(Adsr, {position = "0 0", plugin = "plugin", values = "values"})
))
return ____exports