local ui = require("config/pixel/libs/ui")
local textGrid = require("config/pixel/libs/component/textGrid")
local createWorkspace = require("config/pixel/240x320/ui/menuView/createWorkspace")

local function view(viewName)
    ui.view(viewName)

    ui.component("Workspaces",
        { "PLUGIN" },
        {
            PLUGIN = "SerializeTrack",
        },
        { x = 0, y = 0, w = ScreenWidth, h = 280 }, {
            KEYMAPS = {
                { key = "a", action = ".data:LOAD_WORKSPACE", context = "254:0" },
                { key = "w", action = ".up", context = "254:0" },
                { key = "s", action = ".down", context = "254:0" },
                { key = "d", action = ".delete", context = "254:1" },
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
        -- could introduce copy
        -- could introduce rename
        {
            VISIBILITY_CONTEXT = { "254 SHOW_WHEN 1" },
            KEYMAPS = {
                { key = "q", action = "setView:CreateWorkspace", action2 = "contextToggle:254:1:0" },
                { key = "e", action = "contextToggle:254:1:0" },
            }
        }
    )

    createWorkspace("CreateWorkspace")
end

return view
