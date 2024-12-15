local ui = require "config/pixel/libs/ui"

--- Encoder3 is used to display current audio plugin value for a given parameter.
--- @param params {ENCODER_ID: number, VALUE: { pluginName: string, keyName: string} } Params:
--- - ENCODER_ID is used to set the encoder id that will interract with this component
--- - VALUE is used to set the value to control
--- @param position {x: number, y: number, w: number, h: number} The position of the component.
--- @param options {GROUP: number, TRACK: number, TYPE: string, LABEL: string, COLOR: string, BACKGROUND_COLOR: string, TEXT_COLOR: string, FLOAT_PRECISION: number, SHOW_GROUP: boolean, SHOW_VALUE: boolean, SHOW_UNIT: boolean, FONT_UNIT_SIZE: number, FONT_VALUE_SIZE: number, FONT_TITLE_SIZE: number, STRING_VALUE_REPLACE_TITLE: boolean} Options:
--- - GROUP is the group of the component
--- - TRACK is the track of the component
--- - TYPE is used to set the encoder types. TWO_SIDED when there is a centered value like PAN and you want to show both side value: 20%|80%
--- - LABEL overwrite the value label
--- - COLOR set the ring color
--- - BACKGROUND_COLOR set the background color
--- - TEXT_COLOR set the text color
--- - FLOAT_PRECISION set how many digits after the decimal point (by default none)
--- - SHOW_GROUP show group if the component is part of the current active group (default FALSE)
--- - SHOW_VALUE show value (default TRUE)
--- - SHOW_UNIT show unit (default TRUE)
--- - FONT_UNIT_SIZE set the unit font size
--- - FONT_VALUE_SIZE set the value font size
--- - FONT_TITLE_SIZE set the title font size
--- - STRING_VALUE_REPLACE_TITLE instead to show string value in knob, show under the knob. Can be useful for long string value.
local function component(params, position, options)
    ui.component("Encoder3", {"ENCODER_ID", "VALUE"}, params, position, options)
end

return component