local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getComponent = ____ui.getComponent
local initializePlugin = ____ui.initializePlugin
function ____exports.SeqSynthBar(____bindingPattern0)
    local props
    local seq_plugin
    local track
    local position
    position = ____bindingPattern0.position
    track = ____bindingPattern0.track
    seq_plugin = ____bindingPattern0.seq_plugin
    props = __TS__ObjectRest(____bindingPattern0, {position = true, track = true, seq_plugin = true})
    initializePlugin("SeqSynthBar", "libzic_SeqSynthBarComponent.so")
    return getComponent("SeqSynthBar", position, {{track = track}, {seq_plugin = seq_plugin}, props})
end
return ____exports
