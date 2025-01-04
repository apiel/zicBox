local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getComponent = ____ui.getComponent
function ____exports.Encoder3(____bindingPattern0)
    local props
    local encoder_id
    local value
    local track
    local position
    position = ____bindingPattern0.position
    track = ____bindingPattern0.track
    value = ____bindingPattern0.value
    encoder_id = ____bindingPattern0.encoder_id
    props = __TS__ObjectRest(____bindingPattern0, {position = true, track = true, value = true, encoder_id = true})
    return getComponent("Encoder3", position, {{track = track}, {encoder_id = encoder_id}, {value = value}, props})
end
return ____exports
