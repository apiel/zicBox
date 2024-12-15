local ui = require "config/pixel/libs/ui"

--- KeyInfoBar provide some information about the current kaypad layout.
--- @param params { ROW1: string, ROW2: string } Params:
--- - ROW1 is the text of the first row splitted in 5 parts using space as delimiter.
--- - ROW2 is the text of the second row splitted in 5 parts using space as delimiter.
--- @param position {x: number, y: number} The position of the component.
--- @param options {GROUP: number, SHIFT_VISIBILITY: {index: number, value: number} | string, BACKGROUND_COLOR: string, TEXT_COLOR: string} Options:
--- - GROUP is the group of the component
--- - SHIFT_VISIBILITY is the index and value to make the component visible or not
--- - BACKGROUND_COLOR set the background color
--- - TEXT_COLOR set the text color
local function component(params, position, options)
    if options and options.SHIFT_VISIBILITY then
        options.SHIFT_VISIBILITY = options.SHIFT_VISIBILITY.index .. " " .. options.SHIFT_VISIBILITY.value
    end
    ui.component("KeyInfoBar", { "ROW1", "ROW2" }, params, position, options)
end

return component
