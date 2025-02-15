--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
function ____exports.VisibilityGroup(____bindingPattern0)
    local group
    local condition
    condition = ____bindingPattern0.condition
    group = ____bindingPattern0.group
    return {{VISIBILITY_GROUP = (condition .. " ") .. tostring(group)}}
end
return ____exports
