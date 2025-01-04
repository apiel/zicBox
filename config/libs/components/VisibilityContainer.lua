local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getPosition = ____ui.getPosition
function ____exports.VisibilityContainer(____bindingPattern0)
    local props
    local position
    position = ____bindingPattern0.position
    props = __TS__ObjectRest(____bindingPattern0, {position = true})
    return {
        {CONTAINER = "VisibilityContainer " .. getPosition(position)},
        props
    }
end
return ____exports
