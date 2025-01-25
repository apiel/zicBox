local ____lualib = require("lualib_bundle")
local __TS__ArrayMap = ____lualib.__TS__ArrayMap
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local ____ui = require("config.libs.ui")
local getComponent = ____ui.getComponent
local pluginComponent = ____ui.pluginComponent
local initialized = false
function ____exports.TextGrid(____bindingPattern0)
    local props
    local rows
    local track
    local position
    position = ____bindingPattern0.position
    track = ____bindingPattern0.track
    rows = ____bindingPattern0.rows
    props = __TS__ObjectRest(____bindingPattern0, {position = true, track = true, rows = true})
    if not initialized then
        initialized = true
        pluginComponent("TextGrid", "libzic_TextGridComponent.so")
    end
    return getComponent(
        "TextGrid",
        position,
        {
            {track = track},
            __TS__ArrayMap(
                rows,
                function(____, row) return {ROW = row} end
            ),
            props
        }
    )
end
return ____exports
