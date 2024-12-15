local scrollGroup = require("config/pixel/libs/containers/scrollGroup")
local text = require("config/pixel/libs/component/text")

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

  --    COMPONENT: Encoder3 $3/4W $y $1/4W 50
  --       GROUP: $group
  --       ENCODER_ID: 3
  --       COLOR: tertiary
  --       TRACK: $track
  --       VALUE: Distortion LEVEL

  -- #   Distortion
  -- $group=$group+1
  -- $y=$y+$rowHeight

  --     COMPONENT: Text 0 $y $SCREEN_WIDTH 8
  --       TEXT: Distortion
  --       CENTERED: true
  --       GROUP: $group

  -- $y=$y+$sepHeight
  --     COMPONENT: Encoder3 0 $y $1/4W 50
  --       GROUP: $group
  --       ENCODER_ID: 0
  --       COLOR: tertiary
  --       TRACK: $track
  --       VALUE: Distortion WAVESHAPE

  --     COMPONENT: Encoder3 $1/4W $y $1/4W 50
  --       GROUP: $group
  --       ENCODER_ID: 1
  --       COLOR: tertiary
  --       TRACK: $track
  --       VALUE: Distortion DRIVE

  --     COMPONENT: Encoder3 $2/4W $y $1/4W 50
  --       GROUP: $group
  --       ENCODER_ID: 2
  --       COLOR: tertiary
  --       TRACK: $track
  --       VALUE: Distortion COMPRESS

  --     COMPONENT: Encoder3 $3/4W $y $1/4W 50
  --       GROUP: $group
  --       ENCODER_ID: 3
  --       COLOR: tertiary
  --       TRACK: $track
  --       VALUE: Distortion BASS

  -- #   Waveform
  -- $group=$group+1
  -- $y=$y+$rowHeight

  --     COMPONENT: Text 0 $y $SCREEN_WIDTH 8
  --       TEXT: Waveform
  --       CENTERED: true
  --       GROUP: $group

  -- $y=$y+$sepHeight
  --     COMPONENT: Graph 0 $y ($3/4W-2) 50
  --       GROUP: $group
  --       TRACK: $track
  --       PLUGIN: SynthDrum23
  --       DATA_ID: WAVEFORM
  --       ENCODER: 0 WAVEFORM_TYPE
  --       ENCODER: 1 SHAPE
  --       ENCODER: 2 MACRO

  --     COMPONENT: Encoder3 $3/4W $y $1/4W 50
  --       GROUP: $group
  --       ENCODER_ID: 3
  --       COLOR: quaternary
  --       TRACK: $track
  --       VALUE: SynthDrum23 GAIN_CLIPPING

  -- #   Amp envelope
  -- $group=$group+1
  -- $y=$y+$rowHeight

  --     COMPONENT: Text 0 $y $SCREEN_WIDTH 8
  --       TEXT: Amplitude
  --       CENTERED: true
  --       GROUP: $group

  -- $y=$y+$sepHeight

  --     COMPONENT: DrumEnvelop 0 $y ($3/4W-2) 50
  --       GROUP: $group
  --       TRACK: $track
  --       PLUGIN: SynthDrum23
  --       ENVELOP_DATA_ID: 0
  --       ENCODER_PHASE: 0
  --       ENCODER_TIME: 1
  --       ENCODER_MODULATION: 2

  --     COMPONENT: Encoder3 $3/4W $y $1/4W 50
  --       GROUP: $group
  --       ENCODER_ID: 3
  --       COLOR: quaternary
  --       TRACK: $track
  --       VALUE: SynthDrum23 DURATION

  -- #   Frequency envelope
  -- $group=$group+1
  -- $y=$y+$rowHeight

  --     COMPONENT: Text 0 $y $SCREEN_WIDTH 8
  --       TEXT: Frequency
  --       CENTERED: true
  --       GROUP: $group

  -- $y=$y+$sepHeight

  --     COMPONENT: DrumEnvelop 0 $y ($3/4W-2) 50
  --       GROUP: $group
  --       TRACK: $track
  --       PLUGIN: SynthDrum23
  --       ENVELOP_DATA_ID: 4
  --       ENCODER_PHASE: 0
  --       ENCODER_TIME: 1
  --       ENCODER_MODULATION: 2

  --     COMPONENT: Encoder3 $3/4W $y $1/4W 50
  --       GROUP: $group
  --       ENCODER_ID: 3
  --       COLOR: secondary
  --       TRACK: $track
  --       VALUE: SynthDrum23 PITCH

  -- #   Click
  -- $group=$group+1
  -- $y=$y+$rowHeight

  --     COMPONENT: Text 0 $y $SCREEN_WIDTH 8
  --       TEXT: Click transient
  --       CENTERED: true
  --       GROUP: $group

  -- $y=$y+$sepHeight
  --     COMPONENT: Encoder3 0 $y $1/4W 50
  --       GROUP: $group
  --       ENCODER_ID: 0
  --       COLOR: quaternary
  --       TRACK: $track
  --       VALUE: SynthDrum23 CLICK

  --     COMPONENT: Encoder3 $1/4W $y $1/4W 50
  --       GROUP: $group
  --       ENCODER_ID: 1
  --       COLOR: quaternary
  --       TRACK: $track
  --       VALUE: SynthDrum23 CLICK_DURATION


  --     COMPONENT: Encoder3 $2/4W $y $1/4W 50
  --       GROUP: $group
  --       ENCODER_ID: 2
  --       COLOR: quaternary
  --       TYPE: STRING
  --       TRACK: $track
  --       VALUE: SynthDrum23 CLICK_CUTOFF

  --     COMPONENT: Encoder3 $3/4W $y $1/4W 50
  --       GROUP: $group
  --       ENCODER_ID: 3
  --       COLOR: quaternary
  --       TRACK: $track
  --       VALUE: SynthDrum23 CLICK_RESONANCE
end

return drum23_synth
