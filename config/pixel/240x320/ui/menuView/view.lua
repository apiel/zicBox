local ui = require("config/pixel/libs/ui")
local textGrid = require("config/pixel/libs/component/textGrid")

local function view(viewName)
    ui.view(viewName)

    ui.component("List",
        {},
        {},
        { x = 0, y = 0, w = ScreenWidth, h = 280 }, {
            REDIRECT_VIEW = "menu",
            DONE_DATA = "SerializeTrack SAVE_PROJECT",
            KEYMAPS = {
                { key = "q", action = ".done" },
                { key = "w", action = ".up" },
                { key = "s", action = ".down" },
                { key = "e", action = ".type" },
            }
        })

    textGrid(
        {
            "  &icon::arrowUp::filled  ",
            "OK &icon::arrowDown::filled Back"
        }, KeyInfoPositionCenter,
        {
            KEYMAPS = {
                -- { key = "w", action = "incGroup:-1" },
                -- { key = "s", action = "incGroup:+1" },
                { key = "d", action = "setView:seq" },
            }
        }
    )
end

return view
