local ui = require("config/pixel/libs/ui")

require("config/pixel/audio/main")

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
--     { VALUES = "ATTACK_0 DECAY_0 SUSTAIN_0 RELEASE_0", PLUGIN = "FM" },
--     { x = 0, y = 20, w = ScreenWidth, h = 50 },
--     { TRACK = 4 }
-- )

local encoder3 = require("config/pixel/libs/component/encoder3")
encoder3(
    { ENCODER_ID = 1, VALUE = "MMFilter CUTOFF" },
    { x = 0, y = 80, w = 50, h = 50 },
    { TRACK = 2 }
)