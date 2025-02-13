local ____lualib = require("lualib_bundle")
local __TS__ArrayMap = ____lualib.__TS__ArrayMap
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getComponent = ____ui.getComponent
local initializePlugin = ____ui.initializePlugin
function ____exports.GraphEncoder(____bindingPattern0)
    local props
    local encoders
    local data_id
    local plugin
    local track
    local position
    position = ____bindingPattern0.position
    track = ____bindingPattern0.track
    plugin = ____bindingPattern0.plugin
    data_id = ____bindingPattern0.data_id
    encoders = ____bindingPattern0.encoders
    props = __TS__ObjectRest(____bindingPattern0, {
        position = true,
        track = true,
        plugin = true,
        data_id = true,
        encoders = true
    })
    initializePlugin("GraphEncoder", "libzic_GraphEncoderComponent.so")
    return getComponent(
        "GraphEncoder",
        position,
        {
            {track = track},
            {plugin = plugin},
            {data_id = data_id},
            __TS__ArrayMap(
                encoders,
                function(____, encoder) return {encoder = encoder} end
            ),
            props
        }
    )
end
return ____exports
