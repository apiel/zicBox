local core = require("config/pixel/libs/core")
local ui = require("config/pixel/libs/ui")

--- ScrollGroup is a container that scroll the grouped component together.
--- @param position {x: number, y: number, w: number, h: number} The position of the component.
--- @param options {BACKGROUND_COLOR: string, SCROLL_TO_CENTER: boolean, SCROLL_OFFSET: number} Options:
--- - BACKGROUND_COLOR is the background color of the component.
--- - SCROLL_TO_CENTER scroll to center (default: false).
--- - SCROLL_OFFSET set the where group component start to scroll. By default 70% from container height.
local function container(position, options)
    zic("CONTAINER", "ScrollGroupContainer " .. ui.parsePosition(position))
    core.zic(core.parseOptions(options))
end

return container
