--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
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
    for ____, obj in ipairs(values) do
        for key in pairs(obj) do
            local value = type(obj[key]) == "boolean" and (obj[key] and "true" or "false") or obj[key]
            print((key .. ": ") .. tostring(value))
        end
    end
end
return ____exports
