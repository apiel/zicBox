local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getComponent = ____ui.getComponent
local initializePlugin = ____ui.initializePlugin
function ____exports.Value(____bindingPattern0)
    local props
    local value
    local track
    local bounds
    bounds = ____bindingPattern0.bounds
    track = ____bindingPattern0.track
    value = ____bindingPattern0.value
    props = __TS__ObjectRest(____bindingPattern0, {bounds = true, track = true, value = true})
    initializePlugin("Value", "libzic_ValueComponent.so")
    return getComponent("Value", bounds, {{track = track}, {value = value}, props})
end
return ____exports
