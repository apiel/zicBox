--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local ____core = require("config.libs.core")
local applyZic = ____core.applyZic
local React = require("config.libs.react")
local ____ui = require("config.libs.ui")
local addZoneEncoder = ____ui.addZoneEncoder
local setScreenSize = ____ui.setScreenSize
local setWindowPosition = ____ui.setWindowPosition
local ____clipsView = require("config.pixel.240x320.ui.clipsView.index")
local ClipsView = ____clipsView.ClipsView
local ____constants = require("config.pixel.240x320.ui.constants")
local ScreenHeight = ____constants.ScreenHeight
local ScreenWidth = ____constants.ScreenWidth
local W1_2 = ____constants.W1_2
local W1_4 = ____constants.W1_4
local W3_4 = ____constants.W3_4
local ____seqView = require("config.pixel.240x320.ui.seqView.index")
local SeqView = ____seqView.SeqView
setWindowPosition(400, 500)
setScreenSize(ScreenWidth, ScreenHeight)
addZoneEncoder({0, 0, W1_4, ScreenHeight})
addZoneEncoder({W1_4, 0, W1_4, ScreenHeight})
addZoneEncoder({W1_2, 0, W1_4, ScreenHeight})
addZoneEncoder({W3_4, 0, W1_4, ScreenHeight})
applyZic(React.createElement(SeqView, {name = "Home"}))
applyZic(React.createElement(ClipsView, {name = "Clips"}))
return ____exports
