local ui = require "config/pixel/libs/ui"

local record Props
    position: ui.Point
    size: ui.Size
    text: string
end

local function text(props: Props)
    setConfig("COMPONENT", "Text " .. props.position.x .. " " .. props.position.y .. " " .. props.size.w .. " " .. props.size.h)
    setConfig("TEXT", props.text)
end

return text