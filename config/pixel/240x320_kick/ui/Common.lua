--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____ProgressBar = require("config.pixel.240x320_kick.ui.ProgressBar")
local ProgressBar = ____ProgressBar.ProgressBar
local ____TextGridCommon = require("config.pixel.240x320_kick.ui.TextGridCommon")
local TextGridCommon = ____TextGridCommon.TextGridCommon
function ____exports.Common(____bindingPattern0)
    local selected
    selected = ____bindingPattern0.selected
    return React.createElement(
        React.Fragment,
        nil,
        React.createElement(ProgressBar, nil),
        React.createElement(TextGridCommon, {selected = selected})
    )
end
return ____exports
