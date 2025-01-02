local ui = require("config/pixel/libs/ui")
local textGrid = require("config/pixel/libs/component/textGrid")

local function view(viewName)
    ui.view(viewName)

    textGrid(
        {
            "OK &icon::arrowUp::filled Cancel",
            "&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled"
        }, KeyInfoPositionCenter,
        {
            KEYMAPS = {
                { key = "w", action = "incGroup:-1" },
                { key = "s", action = "incGroup:+1" },
                { key = "e", action = "setView:menu" },
            }
        }
    )
end

return view
