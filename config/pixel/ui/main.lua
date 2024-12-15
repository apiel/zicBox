local ui = require("config/pixel/libs/ui")
local mainKeyboard = require("config/pixel/ui/keyboard_main")

ui.setWindowPosition(400, 500)

ScreenWidth = 320
ScreenHeight = 240
ui.setScreenSize(ScreenWidth, ScreenHeight)

W1_4 = ScreenWidth / 4
W2_4 = W1_4 * 2
W1_2 = W2_4
W3_4 = W1_4 * 3

ui.addZoneEncoder({ 0, 0, W1_4, ScreenHeight })
ui.addZoneEncoder({ W1_4, 0, W1_4, ScreenHeight })
ui.addZoneEncoder({ W1_2, 0, W1_4, ScreenHeight })
ui.addZoneEncoder({ W3_4, 0, W1_4, ScreenHeight })

local track = 0
local drum23_synth = require("config/pixel/ui/drum23_synth")

-- - Track Drum23 (1)
track = track + 1
ui.view("Track" .. track, { COMPONENTS_TRACK = track })
mainKeyboard(track, "SynthDrum23")
-- drum23_synth(track)

-- - Track Drum23 (2)
track = track + 1
ui.view("Track" .. track, { COMPONENTS_TRACK = track })
mainKeyboard(track, "SynthDrum23")
-- drum23_synth(track)

