local ____lualib = require("lualib_bundle")
local __TS__ArrayFlat = ____lualib.__TS__ArrayFlat
local __TS__ArrayIsArray = ____lualib.__TS__ArrayIsArray
local ____exports = {}
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
            zic(
                key,
                tostring(obj[key])
            )
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
