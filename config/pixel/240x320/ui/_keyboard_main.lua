local keyInfoBar = require("config/pixel/libs/component/keyInfoBar")

local position = { x = 0, y = 216 }

local function keyboardMain(track, synth)
  keyInfoBar(
    {
      ROW1 =
      "&icon::musicNote::pixelated &icon::arrowUp::filled &icon::musicNote::pixelated &icon::musicNote::pixelated Track",
      ROW2 = "Seq &icon::arrowDown::filled &icon::musicNote::pixelated &icon::musicNote::pixelated &icon::play::filled"
    }, position,
    {
      CONTEXT_VISIBILITY = {"254 SHOW_WHEN 0"},
      KEYMAPS = {
        { key = "q", action = "noteOn:" .. synth .. ":58" },
        { key = "w", action = "incGroup:-1" },
        { key = "e", action = "noteOn:" .. synth .. ":59" },
        { key = "r", action = "noteOn:" .. synth .. ":60" },
        { key = "t", action = "shift:254:1:0" },
        { key = "a", action = "setView:Sequencer" .. track },
        { key = "s", action = "incGroup:+1" },
        { key = "d", action = "noteOn:" .. synth .. ":61" },
        { key = "f", action = "noteOn:" .. synth .. ":62" },
        { key = "g", action = "playPause" }
      }
    }
  )

  keyInfoBar(
    {
      ROW1 = "1 2 3 4 Track",
      ROW2 = "5 6 7 8 Master"
    }, position,
    {
      CONTEXT_VISIBILITY = {"254 SHOW_WHEN 0"},
      KEYMAPS = {
        { key = "q", action = "setView:Track1" },
        { key = "w", action = track == 2 and "setView:Sequencer2" or "setView:Track2" },
        { key = "e", action = track == 3 and "setView:Sequencer3" or "setView:Track3" },
        { key = "r", action = track == 4 and "setView:Sequencer4" or "setView:Track4" },
        { key = "t", action = "shift:254:1:0" },
        { key = "a", action = track == 5 and "setView:Sequencer5" or "setView:Track5" },
        { key = "s", action = track == 6 and "setView:Sequencer6" or "setView:Track6" },
        { key = "d", action = track == 7 and "setView:Sequencer7" or "setView:Track7" },
        { key = "f", action = track == 8 and "setView:Sequencer8" or "setView:Track8" },
        { key = "g", action = "setView:Master" },
      }
    }
  )
end

return keyboardMain
