local ____lualib = require("lualib_bundle")
local __TS__StringEndsWith = ____lualib.__TS__StringEndsWith
local Error = ____lualib.Error
local RangeError = ____lualib.RangeError
local ReferenceError = ____lualib.ReferenceError
local SyntaxError = ____lualib.SyntaxError
local TypeError = ____lualib.TypeError
local URIError = ____lualib.URIError
local __TS__New = ____lualib.__TS__New
local ____exports = {}
function ____exports.audioPlugin(plugin, config)
    if config == nil then
        config = {}
    end
    config.plugin = plugin
    if not config.alias then
        if __TS__StringEndsWith(plugin, ".so") then
            error(
                __TS__New(Error, "Missing alias name. When providing a plugin path, an alias name must be provided."),
                0
            )
        end
        config.alias = plugin
    end
    return config
end
return ____exports
