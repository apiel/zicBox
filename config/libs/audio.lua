--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local ____core = require("config.libs.core")
local applyZic = ____core.applyZic
local buildPlateform = ____core.buildPlateform
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
function ____exports.plugin(pluginAlias, values)
    if values == nil then
        values = {}
    end
    applyZic({{AUDIO_PLUGIN = pluginAlias}, values})
end
--- Set the auto save mode.
-- 
-- @param time number - The time between each auto save in ms.
-- This will be apply only to plugins that has been already loaded.
function ____exports.autoSave(time)
    applyZic({{AUTO_SAVE = time}})
end
return ____exports
