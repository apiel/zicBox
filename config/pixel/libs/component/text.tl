local ui = require "config/pixel/libs/ui"

-- /*md - `FONT_SIZE: size` is the font size of the component. */
-- /*md - `FONT: font` is the font of the component. */
-- /*md - `BACKGROUND_COLOR: color` is the background color of the component. */
-- /*md - `COLOR: color` is the color of the component. */

local record TextProps
    pos: ui.Point
    size: ui.Size | nil
    text: string
    center: boolean | nil
    fontSize: number | nil
    font: string | nil
    backgroundColor: string | nil
    color: string | nil
end

---@param yo
local function text(props: TextProps)
    setConfig("COMPONENT", "Text " .. ui.getComponentPosition(props.pos, props.size))
    setConfig("TEXT", props.text)
    if props.center ~= nil then
        setConfig("CENTERED", props.center and "true" or "false")
    end
    if props.fontSize ~= nil then
        setConfig("FONT_SIZE", props.fontSize)
    end
    if props.font ~= nil then
        setConfig("FONT", props.font)
    end
    if props.backgroundColor ~= nil then
        setConfig("BACKGROUND_COLOR", props.backgroundColor)
    end
    if props.color ~= nil then
        setConfig("COLOR", props.color)
    end
end

return text