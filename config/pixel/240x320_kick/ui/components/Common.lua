--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____SeqProgressBar = require("config.libs.components.SeqProgressBar")
local SeqProgressBar = ____SeqProgressBar.SeqProgressBar
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
        not hideSequencer and React.createElement(
            SeqProgressBar,
            {
                position = {0, 0, ScreenWidth, 5},
                seq_plugin = "Sequencer",
                active_color = rgb(35, 161, 35),
                selection_color = rgb(35, 161, 35),
                foreground_color = rgb(34, 110, 34),
                volume_plugin = "Volume VOLUME",
                show_steps = true
            }
        ),
        React.createElement(TextGridCommon, {selected = selected})
    )
end
return ____exports
