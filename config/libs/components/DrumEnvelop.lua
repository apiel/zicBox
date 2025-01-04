local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getComponent = ____ui.getComponent
function ____exports.DrumEnvelop(____bindingPattern0)
    local props
    local envelop_data_id
    local plugin
    local track
    local position
    position = ____bindingPattern0.position
    track = ____bindingPattern0.track
    plugin = ____bindingPattern0.plugin
    envelop_data_id = ____bindingPattern0.envelop_data_id
    props = __TS__ObjectRest(____bindingPattern0, {position = true, track = true, plugin = true, envelop_data_id = true})
    return getComponent("DrumEnvelop", position, {{track = track}, {plugin = plugin, envelop_data_id = envelop_data_id}, props})
end
return ____exports
