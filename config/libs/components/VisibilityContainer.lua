local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getPosition = ____ui.getPosition
function ____exports.VisibilityContainer(____bindingPattern0)
    local props
    local group
    local bounds
    bounds = ____bindingPattern0.bounds
    group = ____bindingPattern0.group
    props = __TS__ObjectRest(____bindingPattern0, {bounds = true, group = true})
    return {
        {CONTAINER = "VisibilityContainer " .. getPosition(bounds)},
        {VISIBILITY_GROUP = group},
        props
    }
end
return ____exports
