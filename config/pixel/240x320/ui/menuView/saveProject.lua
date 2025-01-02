local ui = require("config/pixel/libs/ui")
local textGrid = require("config/pixel/libs/component/textGrid")

local function view(viewName)
    ui.view(viewName)

    ui.component("Keyboard",
        {},
        {},
        { x = 0, y = 0, w = ScreenWidth, h = 200 }, {
            KEYMAPS = {
                -- { key = "q", action = ".toggle", context = "254:0" },
                -- { key = "w", action = ".up", context = "254:0" },
                -- { key = "s", action = ".down", context = "254:0" },

                -- { key = "q", action = ".next", context = "254:1" },
                -- { key = "s", action = ".delete", context = "254:1" },
                -- { key = "d", action = ".save", context = "254:1" },
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
