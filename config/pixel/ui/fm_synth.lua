local scrollGroup = require("config/pixel/libs/containers/scrollGroup")
local text = require("config/pixel/libs/component/text")
local encoder3 = require("config/pixel/libs/component/encoder3")
local fmAlgo = require("config/pixel/libs/component/fmAlgo")
local fmSynthOperator = require("config/pixel/ui/fm_synth_operator")

local function fm_synth(track)
  --   CONTAINER: ScrollGroupContainer Scroll 0 0 $SCREEN_WIDTH 214
  --     SCROLL_TO_CENTER: true

  local sepHeight = 12
  local rowHeight = 55

  -- # set to - rowHeight so we easily schuffle thing around
  local y = -(rowHeight)
  local group = -1


  -- #   FM
  -- $group=$group+1
  -- $y=$y+$rowHeight

  --     COMPONENT: Text 0 $y $SCREEN_WIDTH 8
  --       TEXT: FM \($track)
  --       CENTERED: true
  --       GROUP: $group

  -- $y=$y+$sepHeight
  --     COMPONENT: Encoder3 0 $y $1/4W 50
  --       GROUP: $group
  --       ENCODER_ID: 0
  --       COLOR: secondary
  --       TRACK: $track
  --       VALUE: Volume VOLUME

  --     COMPONENT: Encoder3 $1/4W $y $1/4W 50
  --       GROUP: $group
  --       ENCODER_ID: 1
  --       COLOR: primary
  --       TRACK: $track
  --       VALUE: MMFilter CUTOFF

  --     COMPONENT: Encoder3 $2/4W $y $1/4W 50
  --       GROUP: $group
  --       ENCODER_ID: 2
  --       COLOR: primary
  --       TRACK: $track
  --       VALUE: MMFilter RESONANCE

  --     COMPONENT: FmAlgo $3/4W $y $1/4W 50
  --       GROUP: $group
  --       ENCODER_ID: 3
  --       TEXT_COLOR: secondary
  --       TRACK: $track
  --       VALUE: FM ALGO

  -- $group=$group+1
  -- $y=$y+$rowHeight
  -- $y=$y+$sepHeight
  --     COMPONENT: Encoder3 0 $y $1/4W 50
  --       GROUP: $group
  --       ENCODER_ID: 0
  --       COLOR: tertiary
  --       TRACK: $track
  --       VALUE: Bitcrusher SAMPLE_STEP

  --     COMPONENT: Encoder3 $1/4W $y $1/4W 50
  --       GROUP: $group
  --       ENCODER_ID: 1
  --       COLOR: quaternary
  --       TRACK: $track
  --       VALUE: Delay MASTER_AMPLITUDE
  --       LABEL: Delay

  --     COMPONENT: Encoder3 $2/4W $y $1/4W 50
  --       GROUP: $group
  --       ENCODER_ID: 2
  --       COLOR: quaternary
  --       TRACK: $track
  --       VALUE: Delay TIME_RATIO
  --       LABEL: Delay time

  --     COMPONENT: Encoder3 $3/4W $y $1/4W 50
  --       GROUP: $group
  --       ENCODER_ID: 3
  --       COLOR: primary
  --       TRACK: $track
  --       VALUE: FM FREQUENCY

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
