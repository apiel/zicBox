--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____SeqProgressBar = require("config.libs.components.SeqProgressBar")
local SeqProgressBar = ____SeqProgressBar.SeqProgressBar
local ____SeqSynthBar = require("config.libs.components.SeqSynthBar")
local SeqSynthBar = ____SeqSynthBar.SeqSynthBar
local ____ui = require("config.libs.ui")
local rgb = ____ui.rgb
local ____constants = require("config.pixel.240x320_kick.ui.constants")
local ScreenWidth = ____constants.ScreenWidth
local ____TextGridCommon = require("config.pixel.240x320_kick.ui.components.TextGridCommon")
local TextGridCommon = ____TextGridCommon.TextGridCommon
function ____exports.Common(____bindingPattern0)
    local hideSequencer
    local selected
    selected = ____bindingPattern0.selected
    hideSequencer = ____bindingPattern0.hideSequencer
    return React.createElement(
        React.Fragment,
        nil,
        not hideSequencer and React.createElement(SeqSynthBar, {position = {0, 270, ScreenWidth, 10}, seq_plugin = "Sequencer"}),
        React.createElement(
            SeqProgressBar,
            {
                position = {0, 285, ScreenWidth, 5},
                seq_plugin = "Sequencer 0",
                active_color = rgb(35, 161, 35),
                selection_color = rgb(35, 161, 35),
                volume_plugin = "Volume VOLUME"
            }
        ),
        React.createElement(TextGridCommon, {selected = selected})
    )
end
return ____exports
