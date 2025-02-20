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
local ____core = require("config.libs.core")
local applyZic = ____core.applyZic
local jsonStringify = ____core.jsonStringify
function ____exports.audioPlugin(config)
    if not config.aliasName then
        if __TS__StringEndsWith(config.plugin, ".so") then
            error(
                __TS__New(Error, "Missing alias name. When providing a plugin path, an alias name must be provided."),
                0
            )
        end
        config.aliasName = config.plugin
    end
    applyZic({{AUDIO_PLUGIN = jsonStringify(config)}})
end
--- Set the auto save mode.
-- 
-- @param time number - The time between each auto save in ms.
-- This will be apply only to plugins that has been already loaded.
function ____exports.autoSave(time)
    applyZic({{AUTO_SAVE = time}})
end
return ____exports
