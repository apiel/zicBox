local ____lualib = require("lualib_bundle")
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local ____exports = {}
local ____ui = require("config.libs.ui")
local getJsonComponent = ____ui.getJsonComponent
____exports.HiddenValue = function(props)
    if props == nil then
        props = {}
    end
    return getJsonComponent("HiddenValue", "libzic_HiddenValueComponent.so")(__TS__ObjectAssign({}, props, {bounds = {0, 0, 0, 0}}))
end
return ____exports
