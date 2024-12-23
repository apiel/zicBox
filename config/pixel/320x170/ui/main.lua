local ui = require("config/pixel/libs/ui")
local mainKeyboard = require("config/pixel/320x170/ui/keyboard_main")
local sequencer = require("config/pixel/320x170/ui/sequencer")
local drum23Synth = require("config/pixel/320x170/ui/drum23_synth")
local fmSynth = require("config/pixel/320x170/ui/fm_synth")
local drumSample = require("config/pixel/320x170/ui/drumSample")

ui.setWindowPosition(400, 500)

ScreenWidth = 320
ScreenHeight = 170
ui.setScreenSize(ScreenWidth, ScreenHeight)

W1_4 = ScreenWidth / 4
W2_4 = W1_4 * 2
W1_2 = W2_4
W3_4 = W1_4 * 3

KeyInfoPosition = { x = 0, y = 146, w = W2_4 }

ui.addZoneEncoder({ 0, 0, W1_4, ScreenHeight })
ui.addZoneEncoder({ W1_4, 0, W1_4, ScreenHeight })
ui.addZoneEncoder({ W1_2, 0, W1_4, ScreenHeight })
ui.addZoneEncoder({ W3_4, 0, W1_4, ScreenHeight })

local track = 0

-- - Track Sample (1)
track = 1
ui.view("Track" .. track) -- , { COMPONENTS_TRACK = track }
drumSample(track)

-- -- - Track Drum23 (2)
-- track = 2
-- ui.view("Track" .. track, { COMPONENTS_TRACK = track })
-- mainKeyboard(track, "Drum23")
-- drum23Synth(track)

-- ui.view("Sequencer" .. track, { COMPONENTS_TRACK = track })
-- sequencer(track)

-- -- - Track Drum23 (3)
-- track = 3
-- ui.view("Track" .. track, { COMPONENTS_TRACK = track })
-- mainKeyboard(track, "Drum23")
-- drum23Synth(track)

-- ui.view("Sequencer" .. track, { COMPONENTS_TRACK = track })
-- sequencer(track)

-- -- - Track Drum23 (4)
-- track = 4
-- ui.view("Track" .. track, { COMPONENTS_TRACK = track })
-- mainKeyboard(track, "Drum23")
-- drum23Synth(track)

-- ui.view("Sequencer" .. track, { COMPONENTS_TRACK = track })
-- sequencer(track)

-- -- # - Track FM (5)
-- track = 5
-- ui.view("Track" .. track, { COMPONENTS_TRACK = track })
-- mainKeyboard(track, "FM")
-- fmSynth(track)

-- ui.view("Sequencer" .. track, { COMPONENTS_TRACK = track })
-- sequencer(track)

-- -- # - Track FM (6)
-- track = 6
-- ui.view("Track" .. track, { COMPONENTS_TRACK = track })
-- mainKeyboard(track, "FM")
-- fmSynth(track)

-- ui.view("Sequencer" .. track, { COMPONENTS_TRACK = track })
-- sequencer(track)
