local ____lualib = require("lualib_bundle")
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local ____exports = {}
function ____exports.createElement(self, component, props, ...)
    local children = {...}
    local element = component(nil, props)
    if not element then
        return
    end
    if type(element.component) == "function" then
        return ____exports.createElement(nil, element.component, element.props, ...)
    end
    print(element.component, element.props)
    if element.props then
        for prop in pairs(element.props) do
            print("-" .. prop, element.props[prop])
        end
    end
    print(("Children (" .. tostring(#children)) .. "):")
    do
        local i = 0
        while i < #children do
            print(children[i + 1])
            i = i + 1
        end
    end
    return __TS__ObjectAssign({}, element, {children = children})
end
function ____exports.Fragment(self)
    return {component = "Fragment"}
end
return ____exports
