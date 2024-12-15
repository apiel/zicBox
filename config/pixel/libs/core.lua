local core = {}

local function parseValue(value)
    if type(value) == "boolean" then
        return value and "true" or "false"
    end
    if type(value) == "table" then
        for i, v in ipairs(value) do
            value[i] = parseValue(v)
        end
        return table.concat(value, " ")
    end
    return value
end

local function parseKeyMap(value)
    local keyValues = {}
    if type(value) == "table" then
        for _, v in ipairs(value) do
            if type(v) == "string" then
                table.insert(keyValues, { key = "KEYMAP", value = v })
            else
                local controllerId = v.controller and v.controller or "Keyboard"
                local k = type(v.key) == "number" and v.key or "'" .. v.key .. "'"
                local action2 = v.action2 and " " .. v.action2 or ""
                table.insert(keyValues,
                    { key = "KEYMAP", value = controllerId .. " " .. k .. " " .. v.action .. action2 })
            end
        end
    end
    return keyValues
end

local function parseKeyValue(key, value)
    if key == "KEYMAPS" then
        return parseKeyMap(value)
    end
    return { { key = key, value = parseValue(value) } }
end

local function tableMerge(target, source)
    for _, item in ipairs(source) do
        table.insert(target, item)
    end
    return target
end

--- Parse options and apply them to zic configurations
--- @param options { [string]: string | boolean | number | table } Options to apply
function core.parseOptions(options)
    local keyValues = {}
    if options ~= nil then
        for key, value in pairs(options) do
            tableMerge(keyValues, parseKeyValue(key, value))
        end
    end
    return keyValues
end

--- Parse params and apply them to zic configurations
--- @param params { [string]: string | boolean | number | table } Params to apply
--- @param mandatory table Mandatory params key in right order
function core.parseParams(params, mandatory)
    if params == nil then
        error("Params cannot be nil", 2)
    end

    local keyValues = {}
    for _, key in ipairs(mandatory) do
        if params[key] == nil then
            error("Mandatory param " .. key .. " is missing", 2)
        end
        tableMerge(keyValues, parseKeyValue(key, params[key]))
    end
    return keyValues
end

--- Add a controller to the zic configuration
--- @param name string The name of the controller
--- @param pluginPath string The path of the controller plugin
--- @param mandatoryParams table Mandatory params key in right order
--- @param params { [string]: string | boolean | number | table } Params to apply
--- @param options { [string]: string | boolean | number | table } Options to apply
function core.controller(name, pluginPath, mandatoryParams, params, options)
    zic("PLUGIN_CONTROLLER", name .. " " .. pluginPath)
    core.parseParams(params, mandatoryParams)
    core.parseOptions(options)
end

--- Build the plateform
--- @return string The plateform
function core.buildPlateform()
    return IS_RPI and "arm" or "x86"
end

--- Load a dust configurations
--- @param scriptPath string The path of the script
function core.loadDustConfig(scriptPath)
    zic("LOAD_CONFIG", scriptPath .. " plugins/config/build/" .. core.buildPlateform() .. "/libzic_DustConfig.so")
end

--- Apply keyValues to zic
--- @param keyValues table values to apply
function core.zicKeyValues(keyValues)
    for _, keyValue in ipairs(keyValues) do
        zic(keyValue.key, keyValue.value)
    end
end

return core
