local ui = require("config/pixel/libs/ui")
local textGrid = require("config/pixel/libs/component/textGrid")
local text = require("config/pixel/libs/component/text")

local marginTop = 10
local fontHeight = 16

local function menuItem(name, group, action)
    text(name, { x = 0, y = group * (fontHeight + 8) + marginTop, w = ScreenWidth, h = fontHeight },
        {
            CENTERED = true,
            GROUP = group,
            FONT = "Sinclair_S",
            FONT_HEIGHT = fontHeight,
            KEYMAPS = {
                { key = "a", action = action },
            }
        })
end

local function view(viewName)
    ui.view(viewName)

    menuItem("Save project", 0, "setView:MenuSaveProject")
    menuItem("Load project", 1, "setView:MenuLoadProject")

    textGrid(
        {
            "  &icon::arrowUp::filled  ",
            "OK &icon::arrowDown::filled Back"
        }, KeyInfoPositionCenter,
        {
            KEYMAPS = {
                { key = "w", action = "incGroup:-1" },
                { key = "s", action = "incGroup:+1" },
                { key = "d", action = "setView:Home" },
            }
        }
    )
end

return view
