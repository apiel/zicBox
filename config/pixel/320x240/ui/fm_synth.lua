local scrollGroup = require("config/pixel/libs/containers/scrollGroup")
local text = require("config/pixel/libs/component/text")
local encoder3 = require("config/pixel/libs/component/encoder3")
local fmAlgo = require("config/pixel/libs/component/fmAlgo")
local fmSynthOperator = require("config/pixel/320x240/ui/fm_synth_operator")

local function fm_synth(track)
  scrollGroup({ x = 0, y = 0, w = ScreenWidth, h = 214 }, { SCROLL_TO_CENTER = true })

  local sepHeight = 12
  local rowHeight = 55

  -- # set to - rowHeight so we easily schuffle thing around
  local y = -(rowHeight)
  local group = -1


  -- #   FM
  group = group + 1
  y = y + rowHeight

  text("FM (" .. track .. ")", { x = 0, y = y, w = ScreenWidth, h = 8 }, { CENTERED = true, GROUP = group })

  y = y + sepHeight
  encoder3(
    { ENCODER_ID = 0, VALUE = "Volume VOLUME" },
    { x = 0, y = y, w = W1_4, h = 50 },
    { GROUP = group, COLOR = "secondary", TRACK = track }
  )
  encoder3(
    { ENCODER_ID = 1, VALUE = "MMFilter CUTOFF" },
    { x = W1_4, y = y, w = W1_4, h = 50 },
    { GROUP = group, COLOR = "primary", TRACK = track }
  )
  encoder3(
    { ENCODER_ID = 2, VALUE = "MMFilter RESONANCE" },
    { x = W2_4, y = y, w = W1_4, h = 50 },
    { GROUP = group, COLOR = "primary", TRACK = track }
  )
  fmAlgo(
    { ENCODER_ID = 3, VALUE = "FM ALGO" },
    { x = W3_4, y = y, w = W1_4, h = 50 },
    { GROUP = group, TEXT_COLOR = "secondary", TRACK = track }
  )

  group = group + 1
  y = y + rowHeight
  y = y + sepHeight
  encoder3(
    { ENCODER_ID = 0, VALUE = "Bitcrusher SAMPLE_STEP" },
    { x = 0, y = y, w = W1_4, h = 50 },
    { GROUP = group, COLOR = "tertiary", TRACK = track }
  )
  encoder3(
    { ENCODER_ID = 1, VALUE = "Delay MASTER_AMPLITUDE" },
    { x = W1_4, y = y, w = W1_4, h = 50 },
    { GROUP = group, COLOR = "quaternary", TRACK = track, LABEL = "Delay" }
  )
  encoder3(
    { ENCODER_ID = 2, VALUE = "Delay TIME_RATIO" },
    { x = W2_4, y = y, w = W1_4, h = 50 },
    { GROUP = group, COLOR = "quaternary", TRACK = track, LABEL = "Delay time" }
  )
  encoder3(
    { ENCODER_ID = 3, VALUE = "FM FREQUENCY" },
    { x = W3_4, y = y, w = W1_4, h = 50 },
    { GROUP = group, COLOR = "primary", TRACK = track }
  )

  group = group + 1

  -- #   Op 1
  y = fmSynthOperator(track, 0, group, y, sepHeight, rowHeight)
  group = group + 2

  -- #   Op 2
  y = fmSynthOperator(track, 1, group, y, sepHeight, rowHeight)
  group = group + 2

  -- #   Op 3
  y = fmSynthOperator(track, 2, group, y, sepHeight, rowHeight)
  group = group + 2

  -- #   Op 4
  y = fmSynthOperator(track, 3, group, y, sepHeight, rowHeight)
  group = group + 2
end

return fm_synth
