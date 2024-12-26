local core = require("config/pixel/libs/core")
local ui = require("config/pixel/libs/ui")

--- Visibility is a container that show/hide the components for a given group index.
--- @param position {x: number, y: number, w: number, h: number} The position of the component.
--- @param options { VISIBILITY_GROUP: number, VISIBILITY_CONTEXT: string | table } | nil Options:
--- - VISIBILITY_GROUP set the group index
local function container(position, options)
    zic("CONTAINER", "VisibilityContainer " .. ui.parsePosition(position))
    if options ~= nil then
        if options.VISIBILITY_CONTEXT ~= nil then
            if type(options.VISIBILITY_CONTEXT) == "table" then
                for _, context in ipairs(options.VISIBILITY_CONTEXT) do
                    zic("VISIBILITY_CONTEXT", context)
                end
            else
                zic("VISIBILITY_CONTEXT", options.VISIBILITY_CONTEXT)
            end
            options.VISIBILITY_CONTEXT = nil
        end
        core.zic(core.parseOptions(options))
    end
end

return container
