local ui = require("config/pixel/libs/ui")
local text = require("config/pixel/libs/component/text")
ui.setWindowPosition(400, 500)

ScreenWidth = 320
ScreenHeight = 240
ui.setScreenSize(ScreenWidth, ScreenHeight)

ui.view("Main")

text(
    { text = "Hello world" },
    { x = 0, y = 0, w = ScreenWidth, h = 8 },
    { center = true, fontSize = 24 }
)
