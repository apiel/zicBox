--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
function ____exports.VisibilityContext(____bindingPattern0)
    local value
    local index
    local condition
    condition = ____bindingPattern0.condition
    index = ____bindingPattern0.index
    value = ____bindingPattern0.value
    return {{VISIBILITY_CONTEXT = (((tostring(index) .. " ") .. condition) .. " ") .. tostring(value)}}
end
return ____exports
