local ui = require("config/pixel/libs/ui")
local textGrid = require("config/pixel/libs/component/textGrid")

local function view(viewName)
    ui.view(viewName)

    ui.component("Workspaces",
        { "PLUGIN" },
        {
            PLUGIN = "SerializeTrack",
        },
        { x = 0, y = 0, w = ScreenWidth, h = 280 }, {
            KEYMAPS = {
                { key = "a", action = ".data:LOAD_WORKSPACE" },
                { key = "w", action = ".up" },
                { key = "s", action = ".down" },
            }
        })

    textGrid(
        {
            "&empty &icon::arrowUp::filled ...",
            "Use &icon::arrowDown::filled Exit"
        }, KeyInfoPositionCenter,
        {
            VISIBILITY_CONTEXT = { "254 SHOW_WHEN 0" },
            KEYMAPS = {
                { key = "d", action = "setView:Home" },
                { key = "e", action = "contextToggle:254:1:0" },
            }
        }
    )

    textGrid(
        {
            "New &empty ^...",
            "&empty &empty &icon::trash"
        }, KeyInfoPositionCenter,
        {
            VISIBILITY_CONTEXT = { "254 SHOW_WHEN 1" },
            KEYMAPS = {
                { key = "e", action = "contextToggle:254:1:0" },
            }
        }
    )
end

return view
