
local ui = require("config/pixel/libs/ui")
local text = require("config/pixel/libs/component/text")
ui.setWindowPosition(400, 500)

global screenWidth = 320
global screenHeight = 240
ui.setScreenSize(screenWidth, screenHeight)

ui.view("Main")

text({
    pos = { x = 0, y = 0 },
    -- size = { w = screenWidth, h = 8},
    text = "Hello world",
    center = true,
    fontSize = 24
})
