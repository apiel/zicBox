--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____SeqProgressBar = require("config.libs.components.SeqProgressBar")
local SeqProgressBar = ____SeqProgressBar.SeqProgressBar
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local ProgressPosition = ____constants.ProgressPosition
function ____exports.ProgressBar()
    return React.createElement(SeqProgressBar, {
        position = ProgressPosition,
        seq_plugin = "Sequencer 0",
        active_color = "#23a123",
        selection_color = "#23a123",
        volume_plugin = "Volume VOLUME"
    })
end
return ____exports