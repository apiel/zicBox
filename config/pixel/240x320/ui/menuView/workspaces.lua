local ui = require("config/pixel/libs/ui")
local textGrid = require("config/pixel/libs/component/textGrid")

local function view(viewName)
    ui.view(viewName)

    ui.component("List",
        { "PLUGIN" },
        {
            PLUGIN = "SerializeTrack",
        },
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
    -- zic("ADD_ITEM", "Workspaces")
    -- zic("ADD_ITEM", "Daw")
    -- zic("ADD_ITEM", "More")

    textGrid(
        {
            "&empty &icon::arrowUp::filled ...",
            "Use &icon::arrowDown::filled Back"
        }, KeyInfoPositionCenter,
        {
            KEYMAPS = {
                -- { key = "w", action = "incGroup:-1" },
                -- { key = "s", action = "incGroup:+1" },
                { key = "d", action = "setView:Menu" },
            }
        }
    )
end

return view
