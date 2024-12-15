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
--- @param position {x: number, y: number, w: number | nil, h: number | nil} The position of the component.
function ui.parsePosition(position)
    if position.w ~= nil and position.h ~= nil then
        return position.x .. " " .. position.y .. " " .. position.w .. " " .. position.h
    end
    return position.x .. " " .. position.y
end

local function parseKeyValue(key, value)
    -- camel to snake case
    -- key = key:gsub("(%u)", function(c)
    --     return "_" .. c
    -- end):upper()

    if type(value) == "boolean" then
        value = value and "true" or "false"
    end
    zic(key, value)
end

--- Parse options and apply them to zic configurations
--- @param options { [string]: string | boolean | number | table } Options to apply
function ui.parseOptions(options)
    if options ~= nil then
        for key, value in pairs(options) do
            parseKeyValue(key, value)
        end
    end
end

--- Parse params and apply them to zic configurations
--- @param params { [string]: string | boolean | number | table } Params to apply
--- @param mandatory table Mandatory params
function ui.parseParams(params, mandatory)
    if params == nil then
        error("Params cannot be nil", 2)
    end

    for _, key in ipairs(mandatory) do
        if params[key] == nil then
            error("Mandatory param " .. key .. " is missing", 2)
        end
    end

    for key, value in pairs(params) do
        parseKeyValue(key, value)
    end
end

--- Create a component
--- @param name string The name of the component
--- @param params { [string]: string | boolean | number | table } Params to apply
--- @param position {x: number, y: number, w: number | nil, h: number | nil} The position of the component.
--- @param options { [string]: string | boolean | number | table } Options to apply
function ui.component(name, mandatoryParams, params, position, options)
    zic("COMPONENT", name .. " " .. ui.parsePosition(position))
    ui.parseParams(params, mandatoryParams)
    ui.parseOptions(options)
end

return ui
