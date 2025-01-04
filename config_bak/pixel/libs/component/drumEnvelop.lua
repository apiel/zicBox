local ui = require "config/pixel/libs/ui"

--- DrumEnvelop show a representation of a drum envelop (envelop with relative time and modulation, without sustain).
--- @param params {PLUGIN: string, ENVELOP_DATA_ID: string } Params:
--- - PLUGIN set plugin target
--- - ENVELOP_DATA_ID is the data id to get the shape/graph to draw
--- @param position {x: number, y: number, w: number, h: number} The position of the component.
--- @param options {GROUP: number, TRACK: number, STEP_DATA_ID: string, TIME_DATA_ID: string, MOD_DATA_ID: string, OUTLINE: boolean, FILLED: boolean, ENCODER_PHASE: number, ENCODER_TIME: number, ENCODER_MODULATION: number, BACKGROUND_COLOR: string, FILL_COLOR: string, OUTLINE_COLOR: string, TEXT_COLOR: string, CURSOR_COLOR: string, INACTIVE_COLOR_RATIO: number} Options:
--- - GROUP is the group of the component
--- - TRACK is the track of the component
--- - STEP_DATA_ID is the data id to get/set the current step/phase to edit
--- - TIME_DATA_ID is the data id to get/set the step to time.
--- - MOD_DATA_ID is the data id to get/set the step to mod.
--- - OUTLINE is if the envelop should be outlined (default: true).
--- - FILLED is if the envelop should be filled (default: true).
--- - ENCODER_PHASE is the id of the encoder to control the phase.
--- - ENCODER_TIME is the id of the encoder to control the time.
--- - ENCODER_MODULATION is the id of the encoder to control the modulation.
--- - BACKGROUND_COLOR set the background color
--- - FILL_COLOR set the fill color
--- - OUTLINE_COLOR set the outline color
--- - TEXT_COLOR set the text color
--- - CURSOR_COLOR set the cursor color
--- - INACTIVE_COLOR_RATIO is the ratio of darkness for the inactive color (default: 0.5).
local function component(params, position, options)
    ui.component("DrumEnvelop", { "PLUGIN", "ENVELOP_DATA_ID" }, params, position, options)
end

return component
