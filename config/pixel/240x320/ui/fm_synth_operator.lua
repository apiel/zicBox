local text = require("config/pixel/libs/component/text")
local adsr = require("config/pixel/libs/component/adsr")
local encoder3 = require("config/pixel/libs/component/encoder3")

local function fmSynthOperator(track, op, group, y, sepHeight, rowHeight)
  -- #   Operator
  -- $group=$group+1
  y = y + rowHeight

  text(
    { TEXT = "Operator (" .. (op + 1) .. ")" },
    { x = 0, y = y, w = ScreenWidth, h = 8 },
    { CENTERED = true, GROUP = group }
  )

  y = y + sepHeight
  adsr(
    { PLUGIN = "FM", VALUES = "ATTACK_" .. op .. " DECAY_" .. op .. " SUSTAIN_" .. op .. " RELEASE_" .. op },
    { x = 0, y = y, w = ScreenWidth, h = 50 },
    { TRACK = track, GROUP = group }
  )

  y = y + rowHeight
  encoder3(
    { ENCODER_ID = 0, VALUE = "FM RATIO_" .. op },
    { x = 0, y = y, w = W1_4, h = 50 },
    { GROUP = group + 1, COLOR = "tertiary", TRACK = track, FLOAT_PRECISION = 2 }
  )
  encoder3(
    { ENCODER_ID = 1, VALUE = "FM FEEDBACK_" .. op },
    { x = W1_4, y = y, w = W1_4, h = 50 },
    { GROUP = group + 1, COLOR = "tertiary", TRACK = track, FLOAT_PRECISION = 2 }
  )

  return y
end

return fmSynthOperator
