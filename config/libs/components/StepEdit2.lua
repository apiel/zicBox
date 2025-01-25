local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getComponent = ____ui.getComponent
local initializePlugin = ____ui.initializePlugin
function ____exports.StepEdit2(____bindingPattern0)
    local props
    local data
    local track
    local position
    position = ____bindingPattern0.position
    track = ____bindingPattern0.track
    data = ____bindingPattern0.data
    props = __TS__ObjectRest(____bindingPattern0, {position = true, track = true, data = true})
    initializePlugin("StepEdit2", "libzic_StepEdit2Component.so")
    return getComponent("StepEdit2", position, {{track = track}, {data = data}, props})
end
return ____exports
