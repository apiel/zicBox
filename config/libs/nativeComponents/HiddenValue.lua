local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getComponent = ____ui.getComponent
local initializePlugin = ____ui.initializePlugin
function ____exports.HiddenValue(____bindingPattern0)
    if ____bindingPattern0 == nil then
        ____bindingPattern0 = {}
    end
    local props
    local track
    track = ____bindingPattern0.track
    props = __TS__ObjectRest(____bindingPattern0, {track = true})
    initializePlugin("HiddenValue", "libzic_HiddenValueComponent.so")
    return getComponent("HiddenValue", {0, 0}, {{track = track}, props})
end
return ____exports
