--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
function ____exports.Keymap(____bindingPattern0)
    local action2
    local action
    local key
    local context
    local controller
    controller = ____bindingPattern0.controller
    context = ____bindingPattern0.context
    key = ____bindingPattern0.key
    action = ____bindingPattern0.action
    action2 = ____bindingPattern0.action2
    controller = controller or "Keyboard"
    if context then
        controller = controller .. ":" .. context
    end
    if type(key) == "string" and #key == 1 then
        key = ("'" .. key) .. "'"
    end
    return {{KEYMAP = ((((controller .. " ") .. tostring(key)) .. " ") .. action) .. (action2 and " " .. action2 or "")}}
end
return ____exports
