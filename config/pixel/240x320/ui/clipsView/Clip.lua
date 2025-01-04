--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____ProgressBar = require("config.pixel.240x320.ui.components.ProgressBar")
local ProgressBar = ____ProgressBar.ProgressBar
local ____ValueBpm = require("config.pixel.240x320.ui.components.ValueBpm")
local ValueBpm = ____ValueBpm.ValueBpm
function ____exports.ClipsView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(ProgressBar, {y = 5}),
        React.createElement(ValueBpm, nil)
    )
end
return ____exports
