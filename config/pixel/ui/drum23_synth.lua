local scrollGroup = require("config/pixel/libs/containers/scrollGroup")
local text = require("config/pixel/libs/component/text")
local encoder3 = require("config/pixel/libs/component/encoder3")
local graph = require("config/pixel/libs/component/graph")
local drumEnvelop = require("config/pixel/libs/component/drumEnvelop")

local function drum23_synth(track)
  scrollGroup("Scroll", { x = 0, y = 0, w = ScreenWidth, h = 214 }, {
    SCROLL_TO_CENTER = true,
  })

  local sepHeight = 12
  local rowHeight = 55

  -- # set to - rowHeight so we easily schuffle thing around
  local y = -(rowHeight)
  local group = -1


  -- #   Drum
  group = group + 1
  y = y + rowHeight
  text(
    { TEXT = "Drum (" .. track .. ")" },
    { x = 0, y = y, w = ScreenWidth, h = 8 },
    { CENTERED = true, GROUP = group }
  )

  y = y + sepHeight
  encoder3(
    { ENCODER_ID = 0, VALUE = "Volume VOLUME" },
    { x = 0, y = y, w = W1_4, h = 50 },
    { GROUP = group, COLOR = "secondary", TRACK = track }
  )
  -- encoder3(
  --   { ENCODER_ID = 1, VALUE = "MMFilter CUTOFF" },
  --   { x = W1_4, y = y, w = W1_4, h = 50 },
  --   { GROUP = group, COLOR = "primary", TRACK = track }
  -- )
  -- encoder3(
  --   { ENCODER_ID = 2, VALUE = "MMFilter RESONANCE" },
  --   { x = W2_4, y = y, w = W1_4, h = 50 },
  --   { GROUP = group, COLOR = "primary", TRACK = track }
  -- )
  encoder3(
    { ENCODER_ID = 3, VALUE = "Distortion LEVEL" },
    { x = W3_4, y = y, w = W1_4, h = 50 },
    { GROUP = group, COLOR = "tertiary", TRACK = track }
  )

  -- #   Distortion
  group = group + 1
  y = y + rowHeight
  text(
    { TEXT = "Distortion" },
    { x = 0, y = y, w = ScreenWidth, h = 8 },
    { CENTERED = true, GROUP = group }
  )

  y = y + sepHeight
  encoder3(
    { ENCODER_ID = 0, VALUE = "Distortion WAVESHAPE" },
    { x = 0, y = y, w = W1_4, h = 50 },
    { GROUP = group, COLOR = "tertiary", TRACK = track }
  )
  encoder3(
    { ENCODER_ID = 1, VALUE = "Distortion DRIVE" },
    { x = W1_4, y = y, w = W1_4, h = 50 },
    { GROUP = group, COLOR = "tertiary", TRACK = track }
  )
  encoder3(
    { ENCODER_ID = 2, VALUE = "Distortion COMPRESS" },
    { x = W2_4, y = y, w = W1_4, h = 50 },
    { GROUP = group, COLOR = "tertiary", TRACK = track }
  )
  encoder3(
    { ENCODER_ID = 3, VALUE = "Distortion BASS" },
    { x = W3_4, y = y, w = W1_4, h = 50 },
    { GROUP = group, COLOR = "tertiary", TRACK = track }
  )

  -- #   Waveform
  group = group + 1
  y = y + rowHeight
  text(
    { TEXT = "Waveform" },
    { x = 0, y = y, w = ScreenWidth, h = 8 },
    { CENTERED = true, GROUP = group }
  )

  y = y + sepHeight
  graph(
    { PLUGIN = "Drum23", DATA_ID = "WAVEFORM" },
    { x = 0, y = y, w = W3_4 - 2, h = 50 },
    { GROUP = group, TRACK = track, ENCODERS = { "0 WAVEFORM_TYPE", "1 SHAPE", "2 MACRO" } }
  )
  encoder3(
    { ENCODER_ID = 3, VALUE = "Drum23 GAIN_CLIPPING" },
    { x = W3_4, y = y, w = W1_4, h = 50 },
    { GROUP = group, COLOR = "quaternary", TRACK = track }
  )

  -- #   Amp envelope
  group = group + 1
  y = y + rowHeight
  text(
    { TEXT = "Amplitude" },
    { x = 0, y = y, w = ScreenWidth, h = 8 },
    { CENTERED = true, GROUP = group }
  )

  y = y + sepHeight
  drumEnvelop(
    { PLUGIN = "Drum23", ENVELOP_DATA_ID = "0" },
    { x = 0, y = y, w = W3_4 - 2, h = 50 },
    { GROUP = group, TRACK = track, ENCODERS = { "0 PHASE", "1 TIME", "2 MODULATION" } }
  )
  encoder3(
    { ENCODER_ID = 3, VALUE = "Drum23 DURATION" },
    { x = W3_4, y = y, w = W1_4, h = 50 },
    { GROUP = group, COLOR = "quaternary", TRACK = track }
  )

  -- #   Frequency envelope
  group = group + 1
  y = y + rowHeight
  text(
    { TEXT = "Frequency" },
    { x = 0, y = y, w = ScreenWidth, h = 8 },
    { CENTERED = true, GROUP = group }
  )

  y = y + sepHeight
  drumEnvelop(
    { PLUGIN = "Drum23", ENVELOP_DATA_ID = "4" },
    { x = 0, y = y, w = W3_4 - 2, h = 50 },
    { GROUP = group, TRACK = track, ENCODERS = { "0 PHASE", "1 TIME", "2 MODULATION" } }
  )
  encoder3(
    { ENCODER_ID = 3, VALUE = "Drum23 PITCH" },
    { x = W3_4, y = y, w = W1_4, h = 50 },
    { GROUP = group, COLOR = "secondary", TRACK = track }
  )

  -- #   Click
  group = group + 1
  y = y + rowHeight
  text(
    { TEXT = "Click transient" },
    { x = 0, y = y, w = ScreenWidth, h = 8 },
    { CENTERED = true, GROUP = group }
  )

  y = y + sepHeight
  encoder3(
    { ENCODER_ID = 0, VALUE = "Drum23 CLICK" },
    { x = 0, y = y, w = W1_4, h = 50 },
    { GROUP = group, COLOR = "quaternary", TRACK = track }
  )
  encoder3(
    { ENCODER_ID = 1, VALUE = "Drum23 CLICK_DURATION" },
    { x = W1_4, y = y, w = W1_4, h = 50 },
    { GROUP = group, COLOR = "quaternary", TRACK = track }
  )
  encoder3(
    { ENCODER_ID = 2, VALUE = "Drum23 CLICK_CUTOFF" },
    { x = W2_4, y = y, w = W1_4, h = 50 },
    { GROUP = group, COLOR = "quaternary", TRACK = track }
  )
  encoder3(
    { ENCODER_ID = 3, VALUE = "Drum23 CLICK_RESONANCE" },
    { x = W3_4, y = y, w = W1_4, h = 50 },
    { GROUP = group, COLOR = "quaternary", TRACK = track }
  )
end

return drum23_synth
