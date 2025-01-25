local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getComponent = ____ui.getComponent
local initializePlugin = ____ui.initializePlugin
function ____exports.MacroEnvelop(____bindingPattern0)
    local props
    local plugin
    local track
    local position
    position = ____bindingPattern0.position
    track = ____bindingPattern0.track
    plugin = ____bindingPattern0.plugin
    props = __TS__ObjectRest(____bindingPattern0, {position = true, track = true, plugin = true})
    initializePlugin("MacroEnvelop", "libzic_MacroEnvelopComponent.so")
    return getComponent("MacroEnvelop", position, {{track = track}, {plugin = plugin}, props})
end
return ____exports
