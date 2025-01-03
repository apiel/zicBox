--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("react")
local function GroupContainer(self, ____bindingPattern0)
    local a
    a = ____bindingPattern0.a
    return {component = "GroupContainer", props = {a = a}}
end
local function List(self, ____bindingPattern0)
    local b
    b = ____bindingPattern0.b
    return {component = "List", props = {b = b}}
end
local function MyCustomComponent(self)
    return React:createElement(
        React.Fragment,
        nil,
        React:createElement(List, {b = "bla"}),
        React:createElement(List, {b = "blo"})
    )
end
local elements = React:createElement(
    GroupContainer,
    {a = "test"},
    React:createElement(List, {b = "test123"}),
    React:createElement(List, {b = "test456"}),
    React:createElement(MyCustomComponent, nil)
)
print("end:", elements.component)
for element in pairs(elements) do
    print("- ", element)
end
return ____exports
