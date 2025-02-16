local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getOldComponentToBeDeprecated = ____ui.getOldComponentToBeDeprecated
local initializePlugin = ____ui.initializePlugin
function ____exports.SeqProgressBar(____bindingPattern0)
    local props
    local seq_plugin
    local track
    local bounds
    bounds = ____bindingPattern0.bounds
    track = ____bindingPattern0.track
    seq_plugin = ____bindingPattern0.seq_plugin
    props = __TS__ObjectRest(____bindingPattern0, {bounds = true, track = true, seq_plugin = true})
    initializePlugin("SeqProgressBar", "libzic_SeqProgressBarComponent.so")
    return getOldComponentToBeDeprecated("SeqProgressBar", bounds, {{track = track}, {seq_plugin = seq_plugin}, props})
end
return ____exports
