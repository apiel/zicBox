local ____lualib = require("lualib_bundle")
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getBounds = ____ui.getBounds
local initializePlugin = ____ui.initializePlugin
local ____core = require("config.libs.core")
local jsonStringify = ____core.jsonStringify
function ____exports.Text(____bindingPattern0)
    local props
    local position
    position = ____bindingPattern0.position
    local track = ____bindingPattern0.track
    props = __TS__ObjectRest(____bindingPattern0, {position = true, track = true})
    initializePlugin("Text", "libzic_TextComponent.so")
    local jsonConfig = jsonStringify(__TS__ObjectAssign(
        {
            componentName = "Text",
            bounds = getBounds(position)
        },
        props
    ))
    print("My config:", jsonConfig)
    return {{COMPONENT = jsonConfig}}
end
return ____exports
