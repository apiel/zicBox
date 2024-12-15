local ui = require "config/pixel/libs/ui"

--- HiddenEncoder is used to change a value without showing it.
--- @param params {ENCODER_ID: number } Params:
--- - ENCODER_ID is used to set the encoder id that will interract with this component
--- @param options {GROUP: number, TRACK: number, VALUE: { pluginName: string, keyName: string} | string} | nil Options:
--- - GROUP is the group of the component
--- - TRACK is the track of the component
--- - VALUE is used to set the value to control
local function component(params, options)
    if options ~= nil then
        options.VALUE = type(options.VALUE) == "string" and options.VALUE or
            options.VALUE.pluginName .. " " .. options.VALUE.keyName
    end
    ui.component("HiddenEncoder", { "ENCODER_ID" }, params, { 0, 0 }, options)
end

return component
