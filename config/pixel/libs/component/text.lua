local ui = require "config/pixel/libs/ui"

--- Create a text component
--- @param params {TEXT: string } Params:
--- - TEXT The text to display
--- @param position {x: number, y: number, w: number, h: number} The position of the component.
--- @param options {CENTERED: boolean, FONT_SIZE: number, FONT: string, BACKGROUND_COLOR: string, COLOR: string} Options:
--- - CENTERED Whether to center the text (default: false)
--- - FONT_SIZE The font size (default: 8)
--- - FONT The font name (optional)
--- - BACKGROUND_COLOR The background color (optional)
--- - COLOR The text color (optional)
local function text(params, position, options)
    zic("COMPONENT", "Text " .. ui.getComponentPosition(position))
    ui.parseParams(params, { "TEXT" })
    ui.parseOptions(options)
end

return text
