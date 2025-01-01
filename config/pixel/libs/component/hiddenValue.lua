local ui = require "config/pixel/libs/ui"

--- HiddenValue is used to change a value without showing it.
--- @param options {GROUP: number, TRACK: number, VALUE: { pluginName: string, keyName: string} | string} | nil Options:
--- - GROUP is the group of the component
--- - TRACK is the track of the component
--- - VALUE is used to set the value to control
--- - ENCODER_ID is used to set the encoder id that will interract with this component
local function component(options)
    if options ~= nil then
        options.VALUE = type(options.VALUE) == "string" and options.VALUE or
            options.VALUE.pluginName .. " " .. options.VALUE.keyName
    end
    ui.component("HiddenValue", {}, {}, { 0, 0 }, options)
end

return component
