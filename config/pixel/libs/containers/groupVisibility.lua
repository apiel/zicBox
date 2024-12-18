local core = require("config/pixel/libs/core")
local ui = require("config/pixel/libs/ui")

--- GroupVisibility is a container that show/hide the components for a given group index.
--- @param name string The name of the container
--- @param group number The group index
local function container(name, group)
    zic("CONTAINER", "GroupVisibilityContainer " .. name .. " " .. ui.parsePosition({ 0, 0 }))
    zic("VISIBILITY_GROUP", group)
end

return container
