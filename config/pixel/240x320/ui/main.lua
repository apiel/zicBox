local ui = require("config/pixel/libs/ui")
-- local mainKeyboard = require("config/pixel/240x320/ui/keyboard_main")
-- local sequencer = require("config/pixel/240x320/ui/sequencer")
-- local drum23Synth = require("config/pixel/240x320/ui/drum23_synth")
-- local fmSynth = require("config/pixel/240x320/ui/fm_synth")
local mainView = require("config/pixel/240x320/ui/mainView")

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
SecondaryBar = "#734047"
TertiaryBar = "#368a36"
QuaternaryBar = "#94854b"

ui.addZoneEncoder({ 0, 0, W1_4, ScreenHeight })
ui.addZoneEncoder({ W1_4, 0, W1_4, ScreenHeight })
ui.addZoneEncoder({ W1_2, 0, W1_4, ScreenHeight })
ui.addZoneEncoder({ W3_4, 0, W1_4, ScreenHeight })

ui.view("Main")
mainView()
