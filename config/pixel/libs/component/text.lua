local ui = require "config/pixel/libs/ui"

--- Create a text component
--- @param props {text: string, pos: {x: number, y: number}, size: {w: number, h: number}, options: {center: boolean, fontSize: number, font: string, backgroundColor: string, color: string}}
--- - text: The text to display
--- - pos: The position of the component
--- - size: The size of the component (optional)
--- - options:
---    - center: Whether to center the text (optional)
---    - fontSize: The font size (optional)
---    - font: The font name (optional)
---    - backgroundColor: The background color (optional)
---    - color: The text color (optional)
local function text(props)
    setConfig("COMPONENT", "Text " .. ui.getComponentPosition(props.pos, props.size))
    setConfig("TEXT", props.text)
    if props.options ~= nil then
        if props.options.center ~= nil then
            setConfig("CENTERED", props.options.center and "true" or "false")
        end
        if props.options.fontSize ~= nil then
            setConfig("FONT_SIZE", props.options.fontSize)
        end
        if props.options.font ~= nil then
            setConfig("FONT", props.options.font)
        end
        if props.options.backgroundColor ~= nil then
            setConfig("BACKGROUND_COLOR", props.options.backgroundColor)
        end
        if props.options.color ~= nil then
            setConfig("COLOR", props.options.color)
        end
    end
end

return text
