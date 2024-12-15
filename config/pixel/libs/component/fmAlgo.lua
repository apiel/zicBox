local ui = require "config/pixel/libs/ui"

--- FmAlgo show current FM algorithm and change them. The filled square are the carrier where audio is outputted and the not filled square are the operators modulating the frequency.
--- @param params {ENCODER_ID: number, VALUE: { pluginName: string, keyName: string} | string } Params:
--- - ENCODER_ID is used to set the encoder id that will interract with this component
--- - VALUE is used to set the value to control
--- @param position {x: number, y: number, w: number, h: number} The position of the component.
--- @param options {GROUP: number, TRACK: number, BACKGROUND_COLOR: string, TEXT_COLOR: string, BORDER_COLOR: string, DATA_ID: string} Options:
--- - GROUP is the group of the component
--- - TRACK is the track of the component
--- - BACKGROUND_COLOR set the background color
--- - TEXT_COLOR set the text color
--- - BORDER_COLOR set the border color
--- - DATA_ID set the data id that will return the current algorithm layout
local function component(params, position, options)
    params.VALUE = type(params.VALUE) == "string" and params.VALUE or params.VALUE.pluginName .. " " .. params.VALUE.keyName
    ui.component("FmAlgo", {"ENCODER_ID", "VALUE"}, params, position, options)
end

return component