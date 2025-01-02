local ui = require("config/pixel/libs/ui")
local textGrid = require("config/pixel/libs/component/textGrid")
local text = require("config/pixel/libs/component/text")

local marginTop = 10
local fontHeight = 16

local function menuItem(name, group)
    text(name, { x = 0, y = group * (fontHeight + 8) + marginTop, w = ScreenWidth, h = fontHeight },
        { CENTERED = true, GROUP = group, FONT = "Sinclair_S", FONT_HEIGHT = fontHeight })
end

local function view(viewName)
    ui.view(viewName)

    menuItem("Save project", 0)
    menuItem("Load project", 1)

    -- clips
    textGrid(
        {
            "  &icon::arrowUp::filled  ",
            "OK &icon::arrowDown::filled Back"
        }, KeyInfoPositionCenter,
        {
            KEYMAPS = {
                { key = "w", action = "incGroup:-1" },
                { key = "s", action = "incGroup:+1" },
                { key = "d", action = "setView:seq" },
            }
        }
    )
end

return view
