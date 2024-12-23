local ok = pcall(require, "pixel_env")

if not ok then
    print("There is no pixel_env.lua, use default config.")
    require "config/pixel/320x240/main"
end
