local ui = require("config/pixel/libs/ui")
local textGrid = require("config/pixel/libs/component/textGrid")

local function view(viewName)
    ui.view(viewName)

    ui.component("Keyboard",
        {},
        {},
        { x = 0, y = 40, w = ScreenWidth, h = 200 }, {
            REDIRECT_VIEW = "menu",
            DONE_DATA = "SerializeTrack SAVE_WORKSPACE",
            KEYMAPS = {
                { key = "q", action = ".done" },
                { key = "w", action = ".up" },
                { key = "s", action = ".down" },
                { key = "a", action = ".left" },
                { key = "d", action = ".right" },
                { key = "e", action = ".type" },
            }
        })

    textGrid(
        {
            "Save &icon::arrowUp::filled Type",
            "&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled"
        }, KeyInfoPositionCenter,
        {
            KEYMAPS = {}
        }
    )
end

return view
