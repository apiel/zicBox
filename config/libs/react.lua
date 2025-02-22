local ____lualib = require("lualib_bundle")
local __TS__ArrayFilter = ____lualib.__TS__ArrayFilter
local __TS__ArrayFlat = ____lualib.__TS__ArrayFlat
local ____exports = {}
local ____View = require("config.libs.nativeComponents.View")
local View = ____View.View
function ____exports.createElement(component, props, ...)
    local children = {...}
    local element = component(
        {children = children},
        table.unpack({props})
    )
    return __TS__ArrayFlat(
        {
            element,
            component == View and ({}) or __TS__ArrayFilter(
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
