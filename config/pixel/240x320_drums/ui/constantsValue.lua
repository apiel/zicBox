--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local ____constants = require("config.pixel.240x320.ui.constants")
local ScreenWidth = ____constants.ScreenWidth
local W2_4 = ____constants.W2_4
local top = 60
____exports.fullValues = {0, top, ScreenWidth - 1, 180}
____exports.topValues = {0, top, ScreenWidth - 1, 80}
____exports.bottomValues = {0, top + 100, ScreenWidth - 1, 80}
____exports.topLeftKnob = {0, top, W2_4 - 2, 80}
____exports.topRightKnob = {W2_4, top, W2_4 - 2, 80}
____exports.bottomLeftKnob = {0, top + 100, W2_4 - 2, 80}
____exports.bottomRightKnob = {W2_4, top + 100, W2_4 - 2, 80}
return ____exports
