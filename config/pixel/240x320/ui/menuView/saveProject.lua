local ui = require("config/pixel/libs/ui")
local textGrid = require("config/pixel/libs/component/textGrid")

local function view(viewName)
    ui.view(viewName)

    ui.component("Keyboard",
        {},
        {},
        { x = 0, y = 80, w = ScreenWidth, h = 200 }, {
            KEYMAPS = {
                { key = "w", action = ".up" },
                { key = "s", action = ".down" },
                { key = "a", action = ".left" },
                { key = "d", action = ".right" },
            }
        })

    textGrid(
        {
            "OK &icon::arrowUp::filled Cancel",
            "&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled"
        }, KeyInfoPositionCenter,
        {
            KEYMAPS = {
                -- { key = "w", action = "incGroup:-1" },
                -- { key = "s", action = "incGroup:+1" },
                { key = "e", action = "setView:menu" },
            }
        }
    )
end

return view
