local ui = require "config/pixel/libs/ui"

--- Show a representation of the ADSR envdelop.
--- @param params {VALUES: table, PLUGIN: string } Params:
--- - VALUES (A_value D_value S_value R_value) are the values id for the encoders and data point.
--- - PLUGIN set plugin target
--- @param position {x: number, y: number, w: number, h: number} The position of the component.
--- @param options {ENCODERS: table, OUTLINE: boolean, FILLED: boolean, BACKGROUND_COLOR: string, FILL_COLOR: string, OUTLINE_COLOR: string, TEXT_COLOR1: string, TEXT_COLOR2: string, INACTIVE_COLOR_RATIO: number} Options:
--- - ENCODERS (A_encoder_id D_encoder_id S_encoder_id R_encoder_id) are the IDs of the encoders to update given value.
--- - OUTLINE is if the envelop should be outlined (default: true).
--- - FILLED is if the envelop should be filled (default: true).
--- - BACKGROUND_COLOR is the background color of the component.
--- - FILL_COLOR is the color of the envelop.
--- - OUTLINE_COLOR is the color of the envelop outline.
--- - TEXT_COLOR1 is the color of the value.
--- - TEXT_COLOR2 is the color of the unit.
--- - INACTIVE_COLOR_RATIO is the ratio of darkness for the inactive color (default: 0.5).
local function component(params, position, options)
    ui.component("Adsr", {"VALUES", "PLUGIN" }, params, position, options)
end

return component
