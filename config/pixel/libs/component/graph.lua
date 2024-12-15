local ui = require "config/pixel/libs/ui"

--- Show a representation of data points and provide a way to control them.
--- @param params {PLUGIN: string, DATA_ID: string} Params:
--- - PLUGIN set plugin target
--- - DATA_ID is the data id to get the shape/graph to draw
--- @param position {x: number, y: number, w: number, h: number} The position of the component.
--- @param options {GROUP: number, TRACK: number, ENCODERS: table, OUTLINE: boolean, FILLED: boolean, BACKGROUND_COLOR: string, FILL_COLOR: string, OUTLINE_COLOR: string, TEXT_COLOR1: string, TEXT_COLOR2: string, INACTIVE_COLOR_RATIO: number} Options:
--- - GROUP is the group of the component
--- - TRACK is the track of the component
--- - OUTLINE is if the envelop should be outlined (default: true).
--- - FILLED is if the envelop should be filled (default: true).
--- - BACKGROUND_COLOR is the background color of the component.
--- - FILL_COLOR is the color of the envelop.
--- - OUTLINE_COLOR is the color of the envelop outline.
--- - TEXT_COLOR1 is the color of the value.
--- - TEXT_COLOR2 is the color of the unit.
--- - INACTIVE_COLOR_RATIO is the ratio of darkness for the inactive color (default: 0.5).
--- - ENCODERS should be an array of string with `encoder_id value [string]`where is the id of the encoder to update for a given value, e.g. `ENCODER: 0 LEVEL`. Set `string` to force using string rendering instead of numbers with unit...
local function component(params, position, options)
    local encoders = options.ENCODERS
    options.ENCODERS = nil
    ui.component("Graph", { "PLUGIN", "DATA_ID" }, params, position, options)
    if type(encoders) == "table" then
        for _, encoder in ipairs(encoders) do
            zic("ENCODER", encoder)
        end
    end
end

return component
