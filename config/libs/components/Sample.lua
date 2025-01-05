local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getComponent = ____ui.getComponent
function ____exports.Sample(____bindingPattern0)
    local props
    local plugin
    local track
    local position
    position = ____bindingPattern0.position
    track = ____bindingPattern0.track
    plugin = ____bindingPattern0.plugin
    local data_id = ____bindingPattern0.data_id
    local encoders = ____bindingPattern0.encoders
    props = __TS__ObjectRest(____bindingPattern0, {
        position = true,
        track = true,
        plugin = true,
        data_id = true,
        encoders = true
    })
    return getComponent("Sample", position, {{track = track}, {plugin = plugin}, props})
end
return ____exports
