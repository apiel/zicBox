local core = require("config/pixel/libs/core")

local ui = {}

--- Create a view
--- @param name string The name of the view to create
function ui.view(name)
    zic("VIEW", name)
end

--- Set the window position
--- @param x number The x position
--- @param y number The y position
function ui.setWindowPosition(x, y)
    zic("WINDOW_POSITION", x .. " " .. y)
end

--- Set the screen size
--- @param width number The width of the screen
--- @param height number The height of the screen
function ui.setScreenSize(width, height)
    zic("SCREEN", width .. " " .. height)
end

--- Parse a position
--- @param position {x: number, y: number, w: number, h: number} | {x: number, y: number} The position of the component.
function ui.parsePosition(position)
    if position.w ~= nil and position.h ~= nil then
        return position.x .. " " .. position.y .. " " .. position.w .. " " .. position.h
    end
    return position.x .. " " .. position.y
end

--- Create a component
--- @param name string The name of the component
--- @param mandatoryParams table Mandatory params key in right order
--- @param params { [string]: string | boolean | number | table } Params to apply
--- @param position {x: number, y: number, w: number, h: number} | {x: number, y: number} The position of the component.
--- @param options { [string]: string | boolean | number | table } Options to apply
function ui.component(name, mandatoryParams, params, position, options)
    zic("COMPONENT", name .. " " .. ui.parsePosition(position))
    core.parseParams(params, mandatoryParams)
    core.parseOptions(options)
end

return ui
