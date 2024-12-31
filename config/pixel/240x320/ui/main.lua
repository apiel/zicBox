local ui = require("config/pixel/libs/ui")
local seqView = require("config/pixel/240x320/ui/seqView/view")
local clipsView = require("config/pixel/240x320/ui/clipsView/view")

ui.setWindowPosition(400, 500)

ScreenWidth = 240
ScreenHeight = 320
ui.setScreenSize(ScreenWidth, ScreenHeight)

W1_4 = ScreenWidth / 4
W1_8 = ScreenWidth / 8
W2_4 = W1_4 * 2
W1_2 = W2_4
W3_4 = W1_4 * 3

KeyInfoPosition = { x = 0, y = 296, w = W3_4 }

PrimaryBar = "#377275"
SecondaryBar = "#b54f5e"
TertiaryBar = "#368a36"
QuaternaryBar = "#94854b"

ui.addZoneEncoder({ 0, 0, W1_4, ScreenHeight })
ui.addZoneEncoder({ W1_4, 0, W1_4, ScreenHeight })
ui.addZoneEncoder({ W1_2, 0, W1_4, ScreenHeight })
ui.addZoneEncoder({ W3_4, 0, W1_4, ScreenHeight })

seqView("seq")
clipsView("clips")