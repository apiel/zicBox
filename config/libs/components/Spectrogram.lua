local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getComponent = ____ui.getComponent
function ____exports.Spectrogram(____bindingPattern0)
    local props
    local text
    local data
    local track
    local position
    position = ____bindingPattern0.position
    track = ____bindingPattern0.track
    data = ____bindingPattern0.data
    text = ____bindingPattern0.text
    props = __TS__ObjectRest(____bindingPattern0, {position = true, track = true, data = true, text = true})
    return getComponent("Spectrogram", position, {{track = track}, {data = data, text = text}, props})
end
return ____exports
