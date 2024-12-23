local ui = require "config/pixel/libs/ui"

--- KeyInfoBar provide some information about the current kaypad layout.
--- @param rows table ROWS is a table of strings, each strings is splitted in using space as delimiter.
--- @param position {x: number, y: number, w: number | nil} The position of the component.
--- @param options {GROUP: number, SHIFT_VISIBILITY: {index: number, value: number} | string, BACKGROUND_COLOR: string, TEXT_COLOR: string} Options:
--- - GROUP is the group of the component
--- - SHIFT_VISIBILITY is the index and value to make the component visible or not
--- - BACKGROUND_COLOR set the background color
--- - TEXT_COLOR set the text color
local function component(rows, position, options)
    if options and options.SHIFT_VISIBILITY then
        options.SHIFT_VISIBILITY = options.SHIFT_VISIBILITY.index .. " " .. options.SHIFT_VISIBILITY.value
    end
    ui.component("KeyInfoBar", {}, {}, position, options)
    for _, row in ipairs(rows) do
        zic("ROW", row)
    end
end

return component
