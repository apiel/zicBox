local ____lualib = require("lualib_bundle")
local __TS__ArrayMap = ____lualib.__TS__ArrayMap
local ____exports = {}
local ____Keymap = require("config.libs.nativeComponents.Keymap")
local Keymap = ____Keymap.Keymap
function ____exports.Keymaps(____bindingPattern0)
    local keys
    keys = ____bindingPattern0.keys
    return __TS__ArrayMap(
        keys,
        function(____, key) return Keymap(key) end
    )
end
return ____exports
