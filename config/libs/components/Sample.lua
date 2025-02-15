local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getComponent = ____ui.getComponent
local initializePlugin = ____ui.initializePlugin
function ____exports.Sample(____bindingPattern0)
    local props
    local plugin
    local track
    local bounds
    bounds = ____bindingPattern0.bounds
    track = ____bindingPattern0.track
    plugin = ____bindingPattern0.plugin
    local data_id = ____bindingPattern0.data_id
    local encoders = ____bindingPattern0.encoders
    props = __TS__ObjectRest(____bindingPattern0, {
        bounds = true,
        track = true,
        plugin = true,
        data_id = true,
        encoders = true
    })
    initializePlugin("Sample", "libzic_SampleComponent.so")
    return getComponent("Sample", bounds, {{track = track}, {plugin = plugin}, props})
end
return ____exports
