--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____View = require("config.libs.components.View")
local View = ____View.View
local ____Common = require("config.pixel.240x320_kick.ui.Common")
local Common = ____Common.Common
local ____TextGridSeq = require("config.pixel.240x320_kick.ui.Seq.TextGridSeq")
local TextGridSeq = ____TextGridSeq.TextGridSeq
function ____exports.SeqView(____bindingPattern0)
    local name
    name = ____bindingPattern0.name
    return React.createElement(
        View,
        {name = name},
        React.createElement(TextGridSeq, nil),
        React.createElement(Common, {selected = 3})
    )
end
return ____exports
