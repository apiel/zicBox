local ui = require "config/pixel/libs/ui"

--- Create a text component
--- @param text string The text to display
--- @param position {x: number, y: number, w: number, h: number} The position of the component.
--- @param options {CENTERED: boolean, FONT_SIZE: number, FONT: string, BACKGROUND_COLOR: string, COLOR: string} Options:
--- - CENTERED Whether to center the text (default: false)
--- - FONT_SIZE The font size (default: 8)
--- - FONT The font name (optional)
--- - BACKGROUND_COLOR The background color (optional)
--- - COLOR The text color (optional)
local function component(text, position, options)
    ui.component("Text", { "TEXT" }, { TEXT = text }, position, options)
end

return component
