local ____lualib = require("lualib_bundle")
local __TS__ArrayIsArray = ____lualib.__TS__ArrayIsArray
local __TS__ArrayMap = ____lualib.__TS__ArrayMap
local __TS__ObjectKeys = ____lualib.__TS__ObjectKeys
local __TS__TypeOf = ____lualib.__TS__TypeOf
local Error = ____lualib.Error
local RangeError = ____lualib.RangeError
local ReferenceError = ____lualib.ReferenceError
local SyntaxError = ____lualib.SyntaxError
local TypeError = ____lualib.TypeError
local URIError = ____lualib.URIError
local __TS__New = ____lualib.__TS__New
local __TS__ArrayFlat = ____lualib.__TS__ArrayFlat
local ____exports = {}
function ____exports._zic(key, value)
    zic(key, value)
end
--- Convert a value to a JSON string. Since Lua doesn't have builtin JSON support, 
-- this function is a custom function to serialize values to JSON strings.
-- 
-- @param value The value to convert
-- @returns A JSON string representing the value
-- @throws {Error} If the value is of an unsupported type
function ____exports.jsonStringify(value)
    if type(value) == "string" then
        return ("\"" .. value) .. "\""
    elseif type(value) == "number" or type(value) == "boolean" then
        return tostring(value)
    elseif value == nil then
        return "null"
    elseif __TS__ArrayIsArray(value) then
        local elements = table.concat(
            __TS__ArrayMap(
                value,
                function(____, item) return ____exports.jsonStringify(item) end
            ),
            ","
        )
        return ("[" .. elements) .. "]"
    elseif type(value) == "table" then
        local properties = table.concat(
            __TS__ArrayMap(
                __TS__ObjectKeys(value),
                function(____, key) return (("\"" .. key) .. "\":") .. ____exports.jsonStringify(value[key]) end
            ),
            ","
        )
        return ("{" .. properties) .. "}"
    else
        error(
            __TS__New(
                Error,
                "Unsupported type: " .. __TS__TypeOf(value)
            ),
            0
        )
    end
end
--- Get the build the plateform
-- 
-- @returns string The plateform
function ____exports.buildPlateform()
    return IS_RPI and "arm" or "x86"
end
--- Apply zic values by iterating over them
-- 
-- @param values table Zic values
function ____exports.applyZic(values)
    for ____, obj in ipairs(__TS__ArrayFlat(values, math.huge)) do
        for key in pairs(obj) do
        end
    end
end
function ____exports.dump(obj, log)
    if log == nil then
        log = true
    end
    local buffer = ""
    local function color(str, c)
        if c == nil then
            c = 35
        end
        return ((("[" .. tostring(c)) .. "m") .. str) .. "[0m"
    end
    if type(obj) == "string" or type(obj) == "number" or type(obj) == "boolean" then
        buffer = buffer .. tostring(obj)
    elseif __TS__ArrayIsArray(obj) then
        buffer = buffer .. color("[")
        for ____, item in ipairs(obj) do
            buffer = buffer .. ____exports.dump(item, false)
            buffer = buffer .. color(",")
        end
        buffer = buffer .. color("]")
    elseif type(obj) == "table" then
        buffer = buffer .. color("{")
        for key in pairs(obj) do
            buffer = buffer .. color(key .. ": ", 34)
            buffer = buffer .. ____exports.dump(obj[key], false)
            buffer = buffer .. color(",")
        end
        buffer = buffer .. color("}")
    end
    if log then
        print(buffer)
    end
    return buffer
end
return ____exports
