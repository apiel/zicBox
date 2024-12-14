local ui = require "config/pixel/libs/ui"

--- Create a text component
--- @param params {text: string } Params:
--- - text: The text to display
--- @param position {x: number, y: number, w: number, h: number} The position of the component.
--- @param options {center: boolean, fontSize: number, font: string, backgroundColor: string, color: string} Options:
--- - center: Whether to center the text (default: false)
--- - fontSize: The font size (default: 8)
--- - font: The font name (optional)
--- - backgroundColor: The background color (optional)
--- - color: The text color (optional)
local function text(params, position, options)
    zic("COMPONENT", "Text " .. ui.getComponentPosition(position))
    zic("TEXT", params.text)
    if options ~= nil then
        if options.center ~= nil then
            zic("CENTERED", options.center and "true" or "false")
        end
        if options.fontSize ~= nil then
            zic("FONT_SIZE", options.fontSize)
        end
        if options.font ~= nil then
            zic("FONT", options.font)
        end
        if options.backgroundColor ~= nil then
            zic("BACKGROUND_COLOR", options.backgroundColor)
        end
        if options.color ~= nil then
            zic("COLOR", options.color)
        end
    end
end

return text
