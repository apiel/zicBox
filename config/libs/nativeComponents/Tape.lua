local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getOldComponentToBeDeprecated = ____ui.getOldComponentToBeDeprecated
local initializePlugin = ____ui.initializePlugin
function ____exports.Tape(____bindingPattern0)
    local props
    local track
    local bounds
    bounds = ____bindingPattern0.bounds
    track = ____bindingPattern0.track
    props = __TS__ObjectRest(____bindingPattern0, {bounds = true, track = true})
    initializePlugin("Tape", "libzic_TapeComponent.so")
    return getOldComponentToBeDeprecated("Tape", bounds, {{track = track}, props})
end
return ____exports
