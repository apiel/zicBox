local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.pixel.ts.libs.ui")
local getComponent = ____ui.getComponent
function ____exports.Adsr(self, ____bindingPattern0)
    local props
    local values
    local plugin
    local position
    position = ____bindingPattern0.position
    plugin = ____bindingPattern0.plugin
    values = ____bindingPattern0.values
    props = __TS__ObjectRest(____bindingPattern0, {position = true, plugin = true, values = true})
    return getComponent(nil, "Adsr", position, {{PLUGIN = plugin}, {VALUES = values}, props})
end
return ____exports
