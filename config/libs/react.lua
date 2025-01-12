local ____lualib = require("lualib_bundle")
local __TS__ArrayFilter = ____lualib.__TS__ArrayFilter
local __TS__ArrayFlat = ____lualib.__TS__ArrayFlat
local ____exports = {}
function ____exports.createElement(component, props, ...)
    local children = {...}
    local element = component(props)
    return __TS__ArrayFlat(
        {
            element,
            __TS__ArrayFilter(
                children,
                function(____, child) return child end
            )
        },
        math.huge
    )
end
function ____exports.Fragment()
    return {}
end
return ____exports
