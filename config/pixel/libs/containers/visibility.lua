local core = require("config/pixel/libs/core")
local ui = require("config/pixel/libs/ui")

--- Visibility is a container that show/hide the components for a given group index.
--- @param name string The name of the container
--- @param options { VISIBILITY_GROUP: number } | nil Options:
--- - VISIBILITY_GROUP set the group index
local function container(name, options)
    zic("CONTAINER", "VisibilityContainer " .. name .. " " .. ui.parsePosition({ 0, 0 }))
    if options ~= nil then
        core.zic(core.parseOptions(options))
    end
end

return container
