local ui = require "config/pixel/libs/ui"

--- HiddenEncoder is used to change a value without showing it.
--- @param params {ENCODER_ID: number, VALUE: { pluginName: string, keyName: string} | string } Params:
--- - ENCODER_ID is used to set the encoder id that will interract with this component
--- - VALUE is used to set the value to control
--- @param position {x: number, y: number, w: number, h: number} The position of the component.
--- @param options {GROUP: number, TRACK: number} Options:
--- - GROUP is the group of the component
--- - TRACK is the track of the component
local function component(params, position, options)
    params.VALUE = type(params.VALUE) == "string" and params.VALUE or params.VALUE.pluginName .. " " .. params.VALUE.keyName
    ui.component("HiddenEncoder", {"ENCODER_ID", "VALUE"}, params, position, options)
end

return component