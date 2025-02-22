local ____lualib = require("lualib_bundle")
local __TS__ArrayFilter = ____lualib.__TS__ArrayFilter
local __TS__ArrayFlat = ____lualib.__TS__ArrayFlat
local ____exports = {}
function ____exports.View(self, ____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    local children = self.children
    return {{view = {
        name = name,
        components = __TS__ArrayFlat(
            __TS__ArrayFilter(
                children or ({}),
                function(____, child) return child end
            ),
            math.huge
        )
    }}}
end
return ____exports
