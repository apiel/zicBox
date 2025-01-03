local ____lualib = require("lualib_bundle")
local __TS__Iterator = ____lualib.__TS__Iterator
local ____exports = {}
local React = require("libs.react")
local function GroupContainer(self, ____bindingPattern0)
    local a
    a = ____bindingPattern0.a
    return {"COMPONENT: GroupContainer", "A: " .. a}
end
local function List(self, ____bindingPattern0)
    local b
    b = ____bindingPattern0.b
    return {"COMPONENT: List", "B: " .. b}
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
print("end:")
for ____, element in __TS__Iterator(elements) do
    print("- ", element)
end
return ____exports
