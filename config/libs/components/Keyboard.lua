local ____lualib = require("lualib_bundle")
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getComponent = ____ui.getComponent
local initializePlugin = ____ui.initializePlugin
function ____exports.Keyboard(____bindingPattern0)
    local props
    local position
    position = ____bindingPattern0.position
    local items = ____bindingPattern0.items
    props = __TS__ObjectRest(____bindingPattern0, {position = true, items = true})
    initializePlugin("Keyboard", "libzic_KeyboardComponent.so")
    return getComponent("Keyboard", position, {props})
end
return ____exports
