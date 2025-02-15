local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getComponent = ____ui.getComponent
local initializePlugin = ____ui.initializePlugin
function ____exports.StepEditDrum(____bindingPattern0)
    local props
    local data
    local track
    local bounds
    bounds = ____bindingPattern0.bounds
    track = ____bindingPattern0.track
    data = ____bindingPattern0.data
    props = __TS__ObjectRest(____bindingPattern0, {bounds = true, track = true, data = true})
    initializePlugin("StepEditDrum", "libzic_StepEditDrumComponent.so")
    return getComponent("StepEditDrum", bounds, {{track = track}, {data = data}, props})
end
return ____exports
