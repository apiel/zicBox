local ____lualib = require("lualib_bundle")
local __TS__ArrayMap = ____lualib.__TS__ArrayMap
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getComponent = ____ui.getComponent
local initializePlugin = ____ui.initializePlugin
function ____exports.List(____bindingPattern0)
    local props
    local items
    local bounds
    bounds = ____bindingPattern0.bounds
    items = ____bindingPattern0.items
    props = __TS__ObjectRest(____bindingPattern0, {bounds = true, items = true})
    initializePlugin("List", "libzic_ListComponent.so")
    return getComponent(
        "List",
        bounds,
        {
            __TS__ArrayMap(
                items,
                function(____, item) return {ADD_ITEM = item} end
            ),
            props
        }
    )
end
return ____exports
