local ui = {}

--- Create a view
--- @param name string The name of the view to create
function ui.view(name)
    setConfig("VIEW", name)
end

--- Set the window position
--- @param x number The x position
--- @param y number The y position
function ui.setWindowPosition(x, y)
    setConfig("WINDOW_POSITION", x .. " " .. y)
end

--- Set the screen size
--- @param width number The width of the screen
--- @param height number The height of the screen
function ui.setScreenSize(width, height)
    setConfig("SCREEN", width .. " " .. height)
end

--- Get the component position
--- @param pos {x: number, y: number} The position of the component
--- @param size {w: number, h: number} | nil The size of the component
function ui.getComponentPosition(pos, size)
    if size ~= nil then
        return pos.x .. " " .. pos.y .. " " .. size.w .. " " .. size.h
    end
    return pos.x .. " " .. pos.y
end

return ui
