local ui = require("config/pixel/libs/ui")

ui.setWindowPosition(400, 500)

ScreenWidth = 320
ScreenHeight = 240
ui.setScreenSize(ScreenWidth, ScreenHeight)

ui.view("Main")

local text = require("config/pixel/libs/component/text")
text(
    { TEXT = "Hello world" },
    { x = 0, y = 0, w = ScreenWidth, h = 8 },
    { CENTERED = true, FONT_SIZE = 24 }
)

-- local adsr = require("config/pixel/libs/component/adsr")
-- adsr(
--     { VALUES = { "ATTACK_0", "DECAY_0", "SUSTAIN_0", "RELEASE_0" }, PLUGIN = "FM" },
--     { x = 0, y = 20, w = ScreenWidth, h = 50 },
--     { TRACK = 5 }
-- )