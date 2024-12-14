
local ui = require("config/pixel/libs/ui")
local text = require("config/pixel/libs/component/text")
ui.setWindowPosition(400, 500)

ScreenWidth = 320
ScreenHeight = 240
ui.setScreenSize(ScreenWidth, ScreenHeight)

ui.view("Main")

text({
    pos = { x = 0, y = 0 },
    size = { w = ScreenWidth, h = 8 },
    text = "Hello world" ,
    options = { center = true, fontSize = 24 }
})
