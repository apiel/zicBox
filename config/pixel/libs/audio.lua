local core = require("config/pixel/libs/core")

local audio = {}

function audio.pluginAlias(name, pluginPath)
    zic("AUDIO_PLUGIN_ALIAS", name .. " @/plugins/audio/build/" .. core.buildPlateform() .. "/" .. pluginPath)
end

return audio