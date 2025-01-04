local ____lualib = require("lualib_bundle")
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local React = require("config.libs.react")
local ____SeqProgressBar = require("config.libs.components.SeqProgressBar")
local SeqProgressBar = ____SeqProgressBar.SeqProgressBar
local ____constants = require("config.pixel.240x320.ui.constants")
local ScreenWidth = ____constants.ScreenWidth
function ____exports.ProgressBar(____bindingPattern0)
    local props
    local y
    y = ____bindingPattern0.y
    props = __TS__ObjectRest(____bindingPattern0, {y = true})
    return React.createElement(
        SeqProgressBar,
        __TS__ObjectAssign({position = {0, y, ScreenWidth, 5}, seq_plugin = "Sequencer 1", track = 0, active_color = "#23a123"}, props)
    )
end
return ____exports
