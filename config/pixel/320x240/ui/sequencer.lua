local textGrid = require("config/pixel/libs/component/textGrid")
local stepEdit = require("config/pixel/libs/component/stepEdit")
local text = require("config/pixel/libs/component/text")
local hiddenEncoder = require("config/pixel/libs/component/hiddenEncoder")

local function sequencer(track)
  textGrid(
    {
      "1 2 3 4 Shift",
      "5 6 7 8 &icon::play::filled"
    },
    { x = 0, y = 216 },
    {
      CONTEXT_VISIBILITY = {"254 SHOW_WHEN 0"},
      KEYMAPS = {
        { key = "t", action = "shiftToggle:255:0:1", action2 = "shift:254:1:0" },
        { key = "g", action = "playPause" }
      }
    }
  )

  textGrid(
    {
      "1 2 3 4 Track",
      "5 6 7 8 Master"
    },
    { x = 0, y = 216 },
    {
      CONTEXT_VISIBILITY = {"254 SHOW_WHEN 1"},
      KEYMAPS = {
        { key = "q", action = track == 1 and "setView:Track1" or "setView:Sequencer1" },
        { key = "w", action = track == 2 and "setView:Track2" or "setView:Sequencer2" },
        { key = "e", action = track == 3 and "setView:Track3" or "setView:Sequencer3" },
        { key = "r", action = track == 4 and "setView:Track4" or "setView:Sequencer4" },
        { key = "t", action = "shiftToggle:255:0:1", action2 = "shift:254:1:0" },
        { key = "a", action = track == 5 and "setView:Track5" or "setView:Sequencer5" },
        { key = "s", action = track == 6 and "setView:Track6" or "setView:Sequencer6" },
        { key = "d", action = track == 7 and "setView:Track7" or "setView:Sequencer7" },
        { key = "f", action = track == 8 and "setView:Track8" or "setView:Sequencer8" },
        { key = "g", action = "setView:Master" },
      }
    }
  )

  text({ TEXT = "Track " .. track }, { x = 0, y = 0, w = ScreenWidth, h = 8 }, { CENTERED = true })

  local rowHeight = 25
  local y = -rowHeight + 12
  local step = 0
  local groupRange = 0

  while step < 32 do
    y = y + rowHeight
    stepEdit(
      {
        DATA = { pluginName = "Sequencer", dataId = "GET_STEP", stepIndex = step },
        COUNTER_DATA_ID = "STEP_COUNTER"
      },
      { x = 0, y = y, w = W1_4, h = 25 },
      {
        GROUP = step,
        TRACK = track,
        GROUP_RANGE = { groupRange, groupRange + 7 },
        SEQUENCE_DATA_ID = "IS_PLAYING",
        GLOBAL_SHIFT = 254,
        ENCODERS = { 0, 1, 2 },
        KEYMAPS = {
          { key = "q", action = ".stepToggle" },
          { key = "t", action = ".restoreShiftMode" }
        }
      }
    )

    step = step + 1
    stepEdit(
      {
        DATA = { pluginName = "Sequencer", dataId = "GET_STEP", stepIndex = step },
        COUNTER_DATA_ID = "STEP_COUNTER"
      },
      { x = W1_4, y = y, w = W1_4, h = 25 },
      {
        GROUP = step,
        TRACK = track,
        GROUP_RANGE = { groupRange, groupRange + 7 },
        SEQUENCE_DATA_ID = "IS_PLAYING",
        GLOBAL_SHIFT = 254,
        ENCODERS = { 0, 1, 2 },
        KEYMAPS = {
          { key = "w", action = ".stepToggle" },
          { key = "t", action = ".restoreShiftMode" }
        }
      }
    )

    step = step + 1
    stepEdit(
      {
        DATA = { pluginName = "Sequencer", dataId = "GET_STEP", stepIndex = step },
        COUNTER_DATA_ID = "STEP_COUNTER"
      },
      { x = W2_4, y = y, w = W1_4, h = 25 },
      {
        GROUP = step,
        TRACK = track,
        GROUP_RANGE = { groupRange, groupRange + 7 },
        SEQUENCE_DATA_ID = "IS_PLAYING",
        GLOBAL_SHIFT = 254,
        ENCODERS = { 0, 1, 2 },
        KEYMAPS = {
          { key = "e", action = ".stepToggle" },
          { key = "t", action = ".restoreShiftMode" }
        }
      }
    )

    step = step + 1
    stepEdit(
      {
        DATA = { pluginName = "Sequencer", dataId = "GET_STEP", stepIndex = step },
        COUNTER_DATA_ID = "STEP_COUNTER"
      },
      { x = W3_4, y = y, w = W1_4, h = 25 },
      {
        GROUP = step,
        TRACK = track,
        GROUP_RANGE = { groupRange, groupRange + 7 },
        SEQUENCE_DATA_ID = "IS_PLAYING",
        GLOBAL_SHIFT = 254,
        ENCODERS = { 0, 1, 2 },
        KEYMAPS = {
          { key = "r", action = ".stepToggle" },
          { key = "t", action = ".restoreShiftMode" }
        }
      }
    )

    y = y + rowHeight

    step = step + 1
    stepEdit(
      {
        DATA = { pluginName = "Sequencer", dataId = "GET_STEP", stepIndex = step },
        COUNTER_DATA_ID = "STEP_COUNTER"
      },
      { x = 0, y = y, w = W1_4, h = 25 },
      {
        GROUP = step,
        TRACK = track,
        GROUP_RANGE = { groupRange, groupRange + 7 },
        SEQUENCE_DATA_ID = "IS_PLAYING",
        GLOBAL_SHIFT = 254,
        ENCODERS = { 0, 1, 2 },
        KEYMAPS = {
          { key = "a", action = ".stepToggle" },
          { key = "t", action = ".restoreShiftMode" }
        }
      }
    )

    step = step + 1
    stepEdit(
      {
        DATA = { pluginName = "Sequencer", dataId = "GET_STEP", stepIndex = step },
        COUNTER_DATA_ID = "STEP_COUNTER"
      },
      { x = W1_4, y = y, w = W1_4, h = 25 },
      {
        GROUP = step,
        TRACK = track,
        GROUP_RANGE = { groupRange, groupRange + 7 },
        SEQUENCE_DATA_ID = "IS_PLAYING",
        GLOBAL_SHIFT = 254,
        ENCODERS = { 0, 1, 2 },
        KEYMAPS = {
          { key = "s", action = ".stepToggle" },
          { key = "t", action = ".restoreShiftMode" }
        }
      }
    )

    step = step + 1
    stepEdit(
      {
        DATA = { pluginName = "Sequencer", dataId = "GET_STEP", stepIndex = step },
        COUNTER_DATA_ID = "STEP_COUNTER"
      },
      { x = W2_4, y = y, w = W1_4, h = 25 },
      {
        GROUP = step,
        TRACK = track,
        GROUP_RANGE = { groupRange, groupRange + 7 },
        SEQUENCE_DATA_ID = "IS_PLAYING",
        GLOBAL_SHIFT = 254,
        ENCODERS = { 0, 1, 2 },
        KEYMAPS = {
          { key = "d", action = ".stepToggle" },
          { key = "t", action = ".restoreShiftMode" }
        }
      }
    )

    step = step + 1
    stepEdit(
      {
        DATA = { pluginName = "Sequencer", dataId = "GET_STEP", stepIndex = step },
        COUNTER_DATA_ID = "STEP_COUNTER"
      },
      { x = W3_4, y = y, w = W1_4, h = 25 },
      {
        GROUP = step,
        TRACK = track,
        GROUP_RANGE = { groupRange, groupRange + 7 },
        SEQUENCE_DATA_ID = "IS_PLAYING",
        GLOBAL_SHIFT = 254,
        ENCODERS = { 0, 1, 2 },
        KEYMAPS = {
          { key = "f", action = ".stepToggle" },
          { key = "t", action = ".restoreShiftMode" }
        }
      }
    )

    step = step + 1

    groupRange = groupRange + 8
  end

  hiddenEncoder({ ENCODER_ID = 3 })
end

return sequencer
