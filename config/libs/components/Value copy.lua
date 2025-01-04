local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getComponent = ____ui.getComponent
function ____exports.Value(____bindingPattern0)
    local props
    local value
    local track
    local position
    position = ____bindingPattern0.position
    track = ____bindingPattern0.track
    value = ____bindingPattern0.value
    props = __TS__ObjectRest(____bindingPattern0, {position = true, track = true, value = true})
    return getComponent("Value", position, {{track = track}, {value = value}, props})
end
return ____exports
