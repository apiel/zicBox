local ui = require("config/pixel/libs/ui")
local seqView = require("config/pixel/240x320/ui/seqView/view")
local clipsView = require("config/pixel/240x320/ui/clipsView/view")
local menuView = require("config/pixel/240x320/ui/menuView/view")

ui.setWindowPosition(400, 500)

ScreenWidth = 240
ScreenHeight = 320
ui.setScreenSize(ScreenWidth, ScreenHeight)

W1_4 = ScreenWidth / 4
W2_4 = W1_4 * 2
W1_2 = W2_4
W3_4 = W1_4 * 3

W1_8 = ScreenWidth / 8
W2_8 = W1_8 * 2
W3_8 = W1_8 * 3
W4_8 = W1_8 * 4
W5_8 = W1_8 * 5
W6_8 = W1_8 * 6
W7_8 = W1_8 * 7
W8_8 = W1_8 * 8

KeyInfoPosition = { x = 0, y = 296, w = W3_4 }
KeyInfoPositionCenter = { x = W1_8, y = KeyInfoPosition.y, w = KeyInfoPosition.w }

PrimaryBar = "#377275"
SecondaryBar = "#b54f5e"
TertiaryBar = "#368a36"
QuaternaryBar = "#94854b"

ColorTrack1 = "#fd6f0e"
ColorTrack2 = "#ffcd04"
ColorTrack3 = "#a3c912"
ColorTrack4 = "#23a123"
ColorTrack5 = "#15a5ca"
ColorTrack6 = "#4643d8"
ColorTrack7 = "#902ed1"
ColorTrack8 = "#cb11ca"

ui.addZoneEncoder({ 0, 0, W1_4, ScreenHeight })
ui.addZoneEncoder({ W1_4, 0, W1_4, ScreenHeight })
ui.addZoneEncoder({ W1_2, 0, W1_4, ScreenHeight })
ui.addZoneEncoder({ W3_4, 0, W1_4, ScreenHeight })

seqView("Home")
clipsView("Clips")
menuView("Menu")
