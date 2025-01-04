local ui = require "config/pixel/libs/ui"

-- /*md - `DATA: plugin_name data_id step_index` set plugin target */


-- /*md - `GROUP_RANGE: index1 index2` is the index of the first and last group for selection.*/
-- /*md - `COUNTER_DATA_ID: data_id` is the data id to show active step. */
-- /*md - `SEQUENCE_DATA_ID: data_id` is the data id to show if the sequence is playing. */
-- /*md - `ENCODERS: encoder_id1 encoder_id2 encoder_id3` is the id of the encoder to update step value. This component use 3 encoders. In standard view, it will change note, velocity and length. In shift view, it will change note, condition and motion. */
-- /*md - `SHIFT_MODE: 255` set the index of the context variable bank to use to switch between velocity/length and condition/motion. There is 255 context variable banks share within the whole app. */
-- /*md - `GLOBAL_SHIFT: 0` set the index of the context variable bank to detect gloabl shift value. */
-- /*md - `BACKGROUND_COLOR: color` is the background color of the component. */
-- /*md - `TEXT_COLOR: color` is the color of the text. */
-- /*md - `BAR_COLOR: color` is the color of the velocity bar. */
-- /*md - `BAR_BACKGROUND_COLOR: color` is the color of the velocity bar background. */
-- /*md - `TEXT_MOTION1_COLOR: color` is the first color of the motion text. */
-- /*md - `TEXT_MOTION2_COLOR: color` is the second color of the motion text. */

--- StepEdit is used to edit a sequencer step value.
--- @param params {DATA: { pluginName: string, dataId: string, stepIndex: number} | string } Params:
--- - DATA set plugin target
--- - COUNTER_DATA_ID is the data id to show active step
--- @param position {x: number, y: number, w: number, h: number} The position of the component.
--- @param options {GROUP: number, TRACK: number, GROUP_RANGE: string | table, SEQUENCE_DATA_ID: string, ENCODERS: string | table, SHIFT_MODE: number, GLOBAL_SHIFT: number, BACKGROUND_COLOR: string, TEXT_COLOR: string, BAR_COLOR: string, BAR_BACKGROUND_COLOR: string, TEXT_MOTION1_COLOR: string, TEXT_MOTION2_COLOR: string} Options:
--- - GROUP is the group of the component
--- - TRACK is the track of the component
--- - GROUP_RANGE is the index of the first and last group for selection
--- - SEQUENCE_DATA_ID is the data id to show if the sequence is playing
--- - ENCODERS (encoder_id1 encoder_id2 encoder_id3) is the id of the encoder to update step value. This component use 3 encoders. In standard view, it will change note, velocity and length. In shift view, it will change note, condition and motion.
--- - SHIFT_MODE set the index of the context variable bank to use to switch between velocity/length and condition/motion. There is 255 context variable banks share within the whole app.
--- - GLOBAL_SHIFT set the index of the context variable bank to detect gloabl shift value.
--- - BACKGROUND_COLOR set the background color
--- - TEXT_COLOR set the text color
--- - BAR_COLOR set the color of the velocity bar
--- - BAR_BACKGROUND_COLOR set the color of the velocity bar background
--- - TEXT_MOTION1_COLOR set the first color of the motion text
--- - TEXT_MOTION2_COLOR set the second color of the motion text
local function component(params, position, options)
    params.DATA = type(params.DATA) == "string" and params.DATA or params.DATA.pluginName .. " " .. params.DATA.dataId .. " " .. params.DATA.stepIndex
    ui.component("StepEdit", { "DATA", "COUNTER_DATA_ID" }, params, position, options)
end

return component
