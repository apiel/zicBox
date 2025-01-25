local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getComponent = ____ui.getComponent
local initializePlugin = ____ui.initializePlugin
function ____exports.Clips(____bindingPattern0)
    local props
    local track
    local position
    position = ____bindingPattern0.position
    track = ____bindingPattern0.track
    props = __TS__ObjectRest(____bindingPattern0, {position = true, track = true})
    initializePlugin("Clips", "libzic_ClipsComponent.so")
    return getComponent("Clips", position, {{track = track}, props})
end
return ____exports
