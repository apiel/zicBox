local core = require("config/pixel/libs/core")

local ui = {}

--- Create a view
--- @param name string The name of the view to create
--- @param options { COMPONENTS_TRACK: number, GROUP_LOOP: boolean } | nil Options:
--- - COMPONENTS_TRACK set default track value for all components contained in the view
--- - GROUP_LOOP set if group incrementation should loop back to start once reached the end. (default: true)
function ui.view(name, options)
    zic("VIEW", name)
    if options ~= nil then
        core.zic(core.parseOptions(options))
    end
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
--- @param position {x: number, y: number, w: number, h: number} | {x: number, y: number, w: number} | {x: number, y: number} | table The position of the component.
function ui.parsePosition(position)
    if #position == 2 or #position == 3 or #position == 4 then
        return table.concat(position, " ")
    end
    if position.w ~= nil and position.h ~= nil then
        return position.x .. " " .. position.y .. " " .. position.w .. " " .. position.h
    end
    if position.w ~= nil then
        return position.x .. " " .. position.y .. " " .. position.w
    end
    return position.x .. " " .. position.y
end

--- Create a component
--- @param name string The name of the component
--- @param mandatoryParams table Mandatory params key in right order
--- @param params { [string]: string | boolean | number | table } Params to apply
--- @param position {x: number, y: number, w: number, h: number} | {x: number, y: number, w: number} | {x: number, y: number} The position of the component
--- @param options { [string]: string | boolean | number | table } | nil Options to apply
function ui.component(name, mandatoryParams, params, position, options)
    zic("COMPONENT", name .. " " .. ui.parsePosition(position))
    if options ~= nil and options.TRACK ~= nil then
        zic("TRACK", options.TRACK)
        options.TRACK = nil
    end
    core.zic(core.parseParams(params, mandatoryParams))
    if options ~= nil then
        core.zic(core.parseOptions(options))
    end
end

--- Add a zone encoder, used to simulate encoder in SDL mode
--- @param position {x: number, y: number, w: number, h: number} The position of the zone
function ui.addZoneEncoder(position)
    zic("ADD_ZONE_ENCODER", ui.parsePosition(position))
end

function ui.parseVisibilityContext(options, key)
    if (key == nil) then
        key = "VISIBILITY_CONTEXT"
    end
    if options[key] ~= nil then
        if type(options[key]) == "table" then
            for _, context in ipairs(options[key]) do
                zic(key, context)
            end
        else
            zic(key, options[key])
        end
        options[key] = nil
    end
end

function ui.parseVisibilityGroup(options, key)
    if (key == nil) then
        key = "VISIBILITY_GROUP"
    end
    if options[key] ~= nil then
        if type(options[key]) == "table" then
            for _, context in ipairs(options[key]) do
                zic(key, context)
            end
        else
            zic(key, options[key])
        end
        options[key] = nil
    end
end

return ui
