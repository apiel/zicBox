local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getComponent = ____ui.getComponent
local initializePlugin = ____ui.initializePlugin
function ____exports.StepEditMono(____bindingPattern0)
    local props
    local data
    local track
    local bounds
    bounds = ____bindingPattern0.bounds
    track = ____bindingPattern0.track
    data = ____bindingPattern0.data
    props = __TS__ObjectRest(____bindingPattern0, {bounds = true, track = true, data = true})
    initializePlugin("StepEditMono", "libzic_StepEditMonoComponent.so")
    return getComponent("StepEditMono", bounds, {{track = track}, {data = data}, props})
end
return ____exports
