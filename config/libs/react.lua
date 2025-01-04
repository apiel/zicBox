local ____lualib = require("lualib_bundle")
local __TS__ArrayFlat = ____lualib.__TS__ArrayFlat
local ____exports = {}
function ____exports.createElement(component, props, ...)
    local children = {...}
    local element = component(props)
    return __TS__ArrayFlat({element, children}, math.huge)
end
function ____exports.Fragment()
    return {{component = "Fragment"}}
end
return ____exports
