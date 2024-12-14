require "config/pixel/libs/zic"

local record ui

    record Size
       w: number
       h: number
    end

    record Point
        x: number
        y: number
    end
end

function ui.view(name: string)
    setConfig("VIEW", name)
end

function ui.setWindowPosition(x: number, y: number)
    setConfig("WINDOW_POSITION", x .. " " .. y)
end

function ui.setScreenSize(width: number, height: number)
    setConfig("SCREEN", width .. " " .. height)
end

return ui
