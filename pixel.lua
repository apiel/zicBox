local function file_exists(path)
    local file = io.open(path, "r")
    if file then
        file:close()
        return true
    else
        return false
    end
end

if file_exists("pixel_env.lua") then
    require "pixel_env"
else
    require "config/pixel/demo/main"
end
