local ____lualib = require("lualib_bundle")
local __TS__ArrayIsArray = ____lualib.__TS__ArrayIsArray
local ____exports = {}
local ____core = require("config.pixel.ts.libs.core")
local applyZic = ____core.applyZic
function ____exports.getPosition(self, pos)
    return __TS__ArrayIsArray(pos) and table.concat(pos, " ") or pos
end
--- Create a component
-- 
-- @param name string - The name of the component to create.
-- @param position string | string[] - The position of the component.
-- @param values table - An array of Zic values to apply to the component.
-- @returns table
function ____exports.getComponent(self, name, position, values)
    if values == nil then
        values = {}
    end
    return {
        {COMPONENT = (name .. " ") .. ____exports.getPosition(nil, position)},
        unpack(values)
    }
end
--- Create a component
-- 
-- @param name string - The name of the component to create.
-- @param position string | string[] - The position of the component.
-- @param values table - An array of Zic values to apply to the component.
function ____exports.component(self, name, position, values)
    if values == nil then
        values = {}
    end
    applyZic(____exports.getComponent(nil, name, position, values))
end
--- Add a zone encoder
-- 
-- @param position string | string[] - The position of the component.
function ____exports.addZoneEncoder(self, position)
    applyZic({{ADD_ZONE_ENCODER = ____exports.getPosition(nil, position)}})
end
--- Create a view
-- 
-- @param name string - The name of the view to create.
-- @param values table - An array of Zic values to apply to the view.
-- @returns table
function ____exports.getView(self, name, values)
    if values == nil then
        values = {}
    end
    return {
        {VIEW = name},
        unpack(values)
    }
end
--- Create a view and apply Zic values to it.
-- 
-- @param name string - The name of the view to create.
-- @param values table - An array of Zic values to apply to the view.
function ____exports.view(self, name, values)
    if values == nil then
        values = {}
    end
    applyZic(____exports.getView(nil, name, values))
end
--- Set the window position
-- 
-- @param x number The x position
-- @param y number The y position
function ____exports.setWindowPosition(self, x, y)
    applyZic({{WINDOW_POSITION = (tostring(x) .. " ") .. tostring(y)}})
end
--- Set the screen size
-- 
-- @param width number The width of the screen
-- @param height number The height of the screen
function ____exports.setScreenSize(self, width, height)
    applyZic({{SCREEN = (tostring(width) .. " ") .. tostring(height)}})
end
return ____exports
