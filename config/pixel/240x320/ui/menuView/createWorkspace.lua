local ui = require("config/pixel/libs/ui")
local textGrid = require("config/pixel/libs/component/textGrid")

local function view(viewName)
    ui.view(viewName)

    ui.component("Keyboard",
        {},
        {},
        { x = 0, y = 40, w = ScreenWidth, h = 200 }, {
            REDIRECT_VIEW = "Workspaces",
            DONE_DATA = "SerializeTrack CREATE_WORKSPACE",
            KEYMAPS = {
                { key = "q", action = ".type" },
                { key = "w", action = ".up" },
                { key = "s", action = ".down" },
                { key = "a", action = ".left" },
                { key = "d", action = ".right" },
            }
        })

    textGrid(
        {
            "Type &icon::arrowUp::filled &empty",
            "&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled"
        }, KeyInfoPositionCenter,
        {
            KEYMAPS = {}
        }
    )
end

return view
