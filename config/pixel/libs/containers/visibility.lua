local core = require("config/pixel/libs/core")
local ui = require("config/pixel/libs/ui")

--- Visibility is a container that show/hide the components for a given group index.
--- @param position {x: number, y: number, w: number, h: number} The position of the component.
--- @param options { VISIBILITY_GROUP: number, VISIBILITY_CONTEXT: string | table } | nil Options:
--- - VISIBILITY_GROUP set the group index
local function container(position, options)
    zic("CONTAINER", "VisibilityContainer " .. ui.parsePosition(position))
    if options ~= nil then
        ui.parseVisibilityContext(options)
        core.zic(core.parseOptions(options))
    end
end

return container
