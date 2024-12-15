local keyInfoBar = require("config/pixel/libs/component/keyInfoBar")
local stepEdit = require("config/pixel/libs/component/stepEdit")

local function sequencer(track)
  keyInfoBar(
    {
      ROW1 = "1 2 3 4 Shift",
      ROW2 = "5 6 7 8 &icon::play"
    },
    { x = 0, y = 216 },
    {
      SHIFT_VISIBILITY = { index = 254, value = 0 },
      KEYMAPS = {
        { key = "t", action = "shiftToggle:255:0:1", action2 = "shift:254:1:0" },
        { key = "g", action = "playPause" }
      }
    }
  )

  keyInfoBar(
    {
      ROW1 = "1 2 3 4 Track",
      ROW2 = "5 6 7 8 Master"
    },
    { x = 0, y = 216 },
    {
      SHIFT_VISIBILITY = { index = 254, value = 1 },
      KEYMAPS = {
        { key = "q", action = track == 1 and "setView:Track1" or "setView:Sequencer1" },
        { key = "w", action = track == 2 and "setView:Track2" or "setView:Sequencer2" },
        { key = "e", action = track == 3 and "setView:Track3" or "setView:Sequencer3" },
        { key = "r", action = track == 4 and "setView:Track4" or "setView:Sequencer4" },
        { key = "t", action = "shiftToggle:255:0:1",                                  action2 = "shift:254:1:0" },
        { key = "a", action = track == 5 and "setView:Track5" or "setView:Sequencer5" },
        { key = "s", action = track == 6 and "setView:Track6" or "setView:Sequencer6" },
        { key = "d", action = track == 7 and "setView:Track7" or "setView:Sequencer7" },
        { key = "f", action = track == 8 and "setView:Track8" or "setView:Sequencer8" },
        { key = "g", action = "setView:Master" },
      }
    }
  )


  --   COMPONENT: Text 0 0 $SCREEN_WIDTH 8
  --     TEXT: Track \($track)
  --     CENTERED: true

  -- $rowHeight=25
  -- $y=-($rowHeight) + 12
  -- $step=0
  -- $groupRange=0

  -- while: $step < 32
  --   $y=$y+$rowHeight

  --     COMPONENT: StepEdit 0 $y $1/4W 25
  --       GROUP: $step
  --       GROUP_RANGE: ($groupRange) ($groupRange+7)
  --       DATA: Sequencer GET_STEP $step
  --       COUNTER_DATA_ID: STEP_COUNTER
  --       SEQUENCE_DATA_ID: IS_PLAYING
  --       ENCODERS: 0 1 2
  --       KEYMAP: Keyboard 'q' stepToggle
  --       KEYMAP: Keyboard 't' restoreShiftMode
  --       GLOBAL_SHIFT: 254

  --     $step=$step+1
  --     COMPONENT: StepEdit $1/4W $y $1/4W 25
  --       GROUP: $step
  --       GROUP_RANGE: ($groupRange) ($groupRange+7)
  --       DATA: Sequencer GET_STEP $step
  --       COUNTER_DATA_ID: STEP_COUNTER
  --       SEQUENCE_DATA_ID: IS_PLAYING
  --       ENCODERS: 0 1 2
  --       KEYMAP: Keyboard 'w' stepToggle
  --       KEYMAP: Keyboard 't' restoreShiftMode
  --       GLOBAL_SHIFT: 254

  --     $step=$step+1
  --     COMPONENT: StepEdit $2/4W $y $1/4W 25
  --       GROUP: $step
  --       GROUP_RANGE: ($groupRange) ($groupRange+7)
  --       DATA: Sequencer GET_STEP $step
  --       COUNTER_DATA_ID: STEP_COUNTER
  --       SEQUENCE_DATA_ID: IS_PLAYING
  --       ENCODERS: 0 1 2
  --       KEYMAP: Keyboard 'e' stepToggle
  --       KEYMAP: Keyboard 't' restoreShiftMode
  --       GLOBAL_SHIFT: 254

  --     $step=$step+1
  --     COMPONENT: StepEdit $3/4W $y $1/4W 25
  --       GROUP: $step
  --       GROUP_RANGE: ($groupRange) ($groupRange+7)
  --       DATA: Sequencer GET_STEP $step
  --       COUNTER_DATA_ID: STEP_COUNTER
  --       SEQUENCE_DATA_ID: IS_PLAYING
  --       ENCODERS: 0 1 2
  --       KEYMAP: Keyboard 'r' stepToggle
  --       KEYMAP: Keyboard 't' restoreShiftMode
  --       GLOBAL_SHIFT: 254

  --   $y=$y+$rowHeight
  --     $step=$step+1
  --     COMPONENT: StepEdit 0 $y $1/4W 25
  --       GROUP: $step
  --       GROUP_RANGE: ($groupRange) ($groupRange+7)
  --       DATA: Sequencer GET_STEP $step
  --       COUNTER_DATA_ID: STEP_COUNTER
  --       SEQUENCE_DATA_ID: IS_PLAYING
  --       ENCODERS: 0 1 2
  --       KEYMAP: Keyboard 'a' stepToggle
  --       KEYMAP: Keyboard 't' restoreShiftMode
  --       GLOBAL_SHIFT: 254

  --     $step=$step+1
  --     COMPONENT: StepEdit $1/4W $y $1/4W 25
  --       GROUP: $step
  --       GROUP_RANGE: ($groupRange) ($groupRange+7)
  --       DATA: Sequencer GET_STEP $step
  --       COUNTER_DATA_ID: STEP_COUNTER
  --       SEQUENCE_DATA_ID: IS_PLAYING
  --       ENCODERS: 0 1 2
  --       KEYMAP: Keyboard 's' stepToggle
  --       KEYMAP: Keyboard 't' restoreShiftMode
  --       GLOBAL_SHIFT: 254

  --     $step=$step+1
  --     COMPONENT: StepEdit $2/4W $y $1/4W 25
  --       GROUP: $step
  --       GROUP_RANGE: ($groupRange) ($groupRange+7)
  --       DATA: Sequencer GET_STEP $step
  --       COUNTER_DATA_ID: STEP_COUNTER
  --       SEQUENCE_DATA_ID: IS_PLAYING
  --       ENCODERS: 0 1 2
  --       KEYMAP: Keyboard 'd' stepToggle
  --       KEYMAP: Keyboard 't' restoreShiftMode
  --       GLOBAL_SHIFT: 254

  --     $step=$step+1
  --     COMPONENT: StepEdit $3/4W $y $1/4W 25
  --       GROUP: $step
  --       GROUP_RANGE: ($groupRange) ($groupRange+7)
  --       DATA: Sequencer GET_STEP $step
  --       COUNTER_DATA_ID: STEP_COUNTER
  --       SEQUENCE_DATA_ID: IS_PLAYING
  --       ENCODERS: 0 1 2
  --       KEYMAP: Keyboard 'f' stepToggle
  --       KEYMAP: Keyboard 't' restoreShiftMode
  --       GLOBAL_SHIFT: 254

  --     $step=$step+1

  --   $groupRange=$groupRange+8

  -- # need to keep this else loop doesnt work properly...
  -- print: end step=$step

  --   COMPONENT: HiddenEncoder 0 0 0 0
  --     ENCODER_ID: 3
end

return sequencer
