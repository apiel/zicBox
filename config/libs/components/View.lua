local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getView = ____ui.getView
function ____exports.View(____bindingPattern0)
    local props
    local name
    name = ____bindingPattern0.name
    props = __TS__ObjectRest(____bindingPattern0, {name = true})
    return getView(name, {props})
end
return ____exports
