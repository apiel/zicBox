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


local function parseKeyValue(key, value)
    -- Handle KEYMAPS
    if key == "KEYMAPS" then
        if type(value) == "table" then
            for _, v in ipairs(value) do
                if type(v) == "string" then
                    zic("KEYMAP", v)
                else
                    local controllerId = v.controller and v.controller or "Keyboard"
                    local k = type(v.key) == "number" and v.key or "'" .. v.key .. "'"
                    local action2 = v.action2 and " " .. v.action2 or ""
                    zic("KEYMAP", controllerId .. " " .. k .. " " .. v.action .. action2)
                end
            end
        end
        return
    end

    print(key .. ": " .. parseValue(value))
    zic(key, parseValue(value))
end

--- Parse options and apply them to zic configurations
--- @param options { [string]: string | boolean | number | table } Options to apply
function core.parseOptions(options)
    if options ~= nil then
        for key, value in pairs(options) do
            parseKeyValue(key, value)
        end
    end
end

--- Parse params and apply them to zic configurations
--- @param params { [string]: string | boolean | number | table } Params to apply
--- @param mandatory table Mandatory params key in right order
function core.parseParams(params, mandatory)
    if params == nil then
        error("Params cannot be nil", 2)
    end

    for _, key in ipairs(mandatory) do
        if params[key] == nil then
            error("Mandatory param " .. key .. " is missing", 2)
        end
        parseKeyValue(key, params[key])
    end
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

function core.buildPlateform()
    return IS_RPI and "arm" or "x86"
end

--- Load a dust configurations
--- @param scriptPath string The path of the script
function core.loadDustConfig(scriptPath)
    zic("LOAD_CONFIG", scriptPath .. " plugins/config/build/" .. core.buildPlateform() .. "/libzic_DustConfig.so")
end

return core
