local ____lualib = require("lualib_bundle")
local __TS__ArrayIsArray = ____lualib.__TS__ArrayIsArray
local __TS__ArrayJoin = ____lualib.__TS__ArrayJoin
local __TS__NumberToString = ____lualib.__TS__NumberToString
local __TS__StringPadStart = ____lualib.__TS__StringPadStart
local ____exports = {}
local ____core = require("config.libs.core")
local applyZic = ____core.applyZic
function ____exports.getPosition(pos)
    return __TS__ArrayIsArray(pos) and __TS__ArrayJoin(pos, " ") or pos
end
--- Create a component
-- 
-- @param name string - The name of the component to create.
-- @param position string | string[] - The position of the component.
-- @param values table - An array of Zic values to apply to the component.
-- @returns table
function ____exports.getComponent(name, position, values)
    if values == nil then
        values = {}
    end
    return {
        {COMPONENT = (name .. " ") .. ____exports.getPosition(position)},
        values
    }
end
--- Create a component
-- 
-- @param name string - The name of the component to create.
-- @param position string | string[] - The position of the component.
-- @param values table - An array of Zic values to apply to the component.
function ____exports.component(name, position, values)
    if values == nil then
        values = {}
    end
    applyZic(____exports.getComponent(name, position, values))
end
--- Add a zone encoder
-- 
-- @param position string | string[] - The position of the component.
function ____exports.addZoneEncoder(position)
    applyZic({{ADD_ZONE_ENCODER = ____exports.getPosition(position)}})
end
--- Create a view
-- 
-- @param name string - The name of the view to create.
-- @param values table - An array of Zic values to apply to the view.
-- @returns table
function ____exports.getView(name, values)
    if values == nil then
        values = {}
    end
    return {{VIEW = name}, values}
end
--- Create a view and apply Zic values to it.
-- 
-- @param name string - The name of the view to create.
-- @param values table - An array of Zic values to apply to the view.
function ____exports.view(name, values)
    if values == nil then
        values = {}
    end
    applyZic(____exports.getView(name, values))
end
--- Set the window position
-- 
-- @param x number The x position
-- @param y number The y position
function ____exports.setWindowPosition(x, y)
    applyZic({{WINDOW_POSITION = (tostring(x) .. " ") .. tostring(y)}})
end
--- Set the screen size
-- 
-- @param width number The width of the screen
-- @param height number The height of the screen
function ____exports.setScreenSize(width, height)
    applyZic({{SCREEN = (tostring(width) .. " ") .. tostring(height)}})
end
--- Returns a string representing a color in hexadecimal notation.
-- 
-- @param r number The red component of the color, between 0 and 255.
-- @param g number The green component of the color, between 0 and 255.
-- @param b number The blue component of the color, between 0 and 255.
-- @returns string A string in the format `#RRGGBB`, where `RR`, `GG`, and `BB` are the red, green, and blue components of the color, respectively, in hexadecimal notation.
function ____exports.rgb(r, g, b)
    return (("#" .. __TS__StringPadStart(
        __TS__NumberToString(r, 16),
        2,
        "0"
    )) .. __TS__StringPadStart(
        __TS__NumberToString(g, 16),
        2,
        "0"
    )) .. __TS__StringPadStart(
        __TS__NumberToString(b, 16),
        2,
        "0"
    )
end
return ____exports
