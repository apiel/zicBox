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

--- Get the component position
--- @param position {x: number, y: number, w: number, h: number} The position of the component.
function ui.getComponentPosition(position)
    if position.w ~= nil and position.h ~= nil then
        return position.x .. " " .. position.y .. " " .. position.w .. " " .. position.h
    end
    return position.x .. " " .. position.y
end

local function camelToSnake(camel_str)
    return camel_str:gsub("(%u)", function(c)
        return "_" .. c
    end):upper()
end

function ui.parseOptions(options)
    if options ~= nil then
        for key, value in pairs(options) do
            if type(value) == "boolean" then
                value = value and "true" or "false"
            end
            print("key:" .. camelToSnake(key) .. " val:" .. value)
            zic(camelToSnake(key), value)
        end
    end
end

return ui
