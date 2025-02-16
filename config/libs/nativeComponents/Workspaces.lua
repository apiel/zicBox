local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getOldComponentToBeDeprecated = ____ui.getOldComponentToBeDeprecated
local initializePlugin = ____ui.initializePlugin
function ____exports.Workspaces(____bindingPattern0)
    local props
    local plugin
    local track
    local bounds
    bounds = ____bindingPattern0.bounds
    track = ____bindingPattern0.track
    plugin = ____bindingPattern0.plugin
    props = __TS__ObjectRest(____bindingPattern0, {bounds = true, track = true, plugin = true})
    initializePlugin("Workspaces", "libzic_WorkspacesComponent.so")
    return getOldComponentToBeDeprecated("Workspaces", bounds, {{track = track}, {plugin = plugin}, props})
end
return ____exports
