local ____lualib = require("lualib_bundle")
local __TS__Spread = ____lualib.__TS__Spread
local __TS__ArrayFlat = ____lualib.__TS__ArrayFlat
local __TS__SparseArrayNew = ____lualib.__TS__SparseArrayNew
local __TS__SparseArrayPush = ____lualib.__TS__SparseArrayPush
local __TS__SparseArraySpread = ____lualib.__TS__SparseArraySpread
local ____exports = {}
function ____exports.createElement(self, component, props, ...)
    local children = {...}
    local element = component(nil, props)
    local ____array_0 = __TS__SparseArrayNew(__TS__Spread(element))
    __TS__SparseArrayPush(
        ____array_0,
        unpack(__TS__ArrayFlat(children))
    )
    return {__TS__SparseArraySpread(____array_0)}
end
function ____exports.Fragment(self)
    return {{component = "Fragment"}}
end
return ____exports
