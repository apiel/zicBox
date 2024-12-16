local core = require("config/pixel/libs/core")

local audio = {}

--- Pre-load an audio plugin to the zic configuration and aissign it an alias
function audio.pluginAlias(name, pluginPath)
    zic("AUDIO_PLUGIN_ALIAS", name .. " @/plugins/audio/build/" .. core.buildPlateform() .. "/" .. pluginPath)
end

--- Assign a plugin to the zic configuration
--- @param pluginAlias string The alias name of the plugin to load
--- @param options { NAME: string, TRACK: number, SERIALIZABLE: boolean } | nil Options:
--- - NAME give a name to the plugin to be re-used in the UI (else it will use the name of the alias)
--- - TRACK give a track number to the plugin (defaut: 0)
--- - SERIALIZABLE set if the plugin is serializable (default: true)
function audio.plugin(pluginAlias, options)
    if options ~= nil and options.NAME ~= nil then
        zic("AUDIO_PLUGIN", options.NAME .. " " .. pluginAlias)
        options.NAME = nil
    else
        zic("AUDIO_PLUGIN", pluginAlias)
    end
    if options ~= nil then
        core.zic(core.parseOptions(options))
    end
end

--- Set the auto save mode
--- @param time number The time between each auto save in ms.
--- This will be apply only to plugins that has been already loaded
function audio.autoSave(time)
    zic("AUTO_SAVE", time)
end

return audio
