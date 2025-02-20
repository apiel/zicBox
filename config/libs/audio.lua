local ____lualib = require("lualib_bundle")
local __TS__StringEndsWith = ____lualib.__TS__StringEndsWith
local Error = ____lualib.Error
local RangeError = ____lualib.RangeError
local ReferenceError = ____lualib.ReferenceError
local SyntaxError = ____lualib.SyntaxError
local TypeError = ____lualib.TypeError
local URIError = ____lualib.URIError
local __TS__New = ____lualib.__TS__New
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____core = require("config.libs.core")
local applyZic = ____core.applyZic
local buildPlateform = ____core.buildPlateform
local jsonStringify = ____core.jsonStringify
--- Pre-load an audio plugin to the zic configuration and assign it an alias.
-- 
-- @param name string - The alias name of the plugin to load.
-- @param pluginPath string - The path of the plugin to load.
function ____exports.pluginAlias(name, pluginPath)
    applyZic({{AUDIO_PLUGIN_ALIAS = (((name .. " @/plugins/audio/build/") .. buildPlateform()) .. "/") .. pluginPath}})
end
--- Assign an audio plugin to the zic configuration using a specified alias and additional values.
-- 
-- @param pluginAlias string - The alias name of the plugin to load: "aliasName" or "name aliasName". If "name" is provided, it will be used as the name of the plugin in the UI, instead of "aliasName".
-- @param values table - An array of additional Zic values to apply to the plugin.
function ____exports.pluginOld(pluginAlias, values)
    if values == nil then
        values = {}
    end
end
function ____exports.audioPlugin(____bindingPattern0)
    local config
    local plugin
    local aliasName
    aliasName = ____bindingPattern0.aliasName
    plugin = ____bindingPattern0.plugin
    config = __TS__ObjectRest(____bindingPattern0, {aliasName = true, plugin = true})
    if not aliasName then
        if __TS__StringEndsWith(plugin, ".so") then
            error(
                __TS__New(Error, "Missing alias name. When providing a plugin path, an alias name must be provided."),
                0
            )
        end
        aliasName = plugin
    end
    applyZic({
        {AUDIO_PLUGIN = jsonStringify(__TS__ObjectAssign({aliasName = aliasName, plugin = plugin}, config))},
        {config}
    })
end
--- Set the auto save mode.
-- 
-- @param time number - The time between each auto save in ms.
-- This will be apply only to plugins that has been already loaded.
function ____exports.autoSave(time)
    applyZic({{AUTO_SAVE = time}})
end
return ____exports
