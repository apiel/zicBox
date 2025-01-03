local ui = require("config/pixel/libs/ui")
local textGrid = require("config/pixel/libs/component/textGrid")
local workspaces = require("config/pixel/240x320/ui/menuView/workspaces")

local function view(viewName)
    ui.view(viewName)

    ui.component("List", {}, {},
        { x = 0, y = 0, w = ScreenWidth, h = 280 }, {
            -- REDIRECT_VIEW = "menu",
            KEYMAPS = {
                -- { key = "a", action = ".data:LOAD_WORKSPACE" },
                { key = "a", action = ".setView" },
                { key = "w", action = ".up" },
                { key = "s", action = ".down" },
                -- { key = "e", action = ".type" },
            }
        })
    zic("ADD_ITEM", "Workspaces")
    zic("ADD_ITEM", "Daw")
    zic("ADD_ITEM", "More")
    zic("ADD_ITEM", "Shutdown") -- TODO allow different color

    textGrid(
        {
            "  &icon::arrowUp::filled  ",
            "Select &icon::arrowDown::filled Exit"
        }, KeyInfoPositionCenter,
        {
            KEYMAPS = {
                -- { key = "w", action = "incGroup:-1" },
                -- { key = "s", action = "incGroup:+1" },
                { key = "d", action = "setView:Home" },
            }
        }
    )

    workspaces("Workspaces")
end

return view
