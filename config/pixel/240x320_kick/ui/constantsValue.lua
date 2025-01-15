local ____lualib = require("lualib_bundle")
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local ____exports = {}
local ____constants = require("config.pixel.240x320.ui.constants")
local PrimaryBar = ____constants.PrimaryBar
local QuaternaryBar = ____constants.QuaternaryBar
local ScreenWidth = ____constants.ScreenWidth
local SecondaryBar = ____constants.SecondaryBar
local TertiaryBar = ____constants.TertiaryBar
local W2_4 = ____constants.W2_4
local valueH = 29
local rowSpacing = 8
____exports.height = 65
____exports.posY = 84
____exports.encoderH = 50
____exports.topLeft = {0, 0, W2_4 - 2, valueH}
____exports.bottomLeft = {0, valueH + rowSpacing, W2_4 - 2, valueH}
____exports.topRight = {W2_4, 0, W2_4 - 2, valueH}
____exports.bottomRight = {W2_4, valueH + rowSpacing, W2_4 - 2, valueH}
____exports.posContainerValues = {0, ____exports.posY, ScreenWidth, valueH * 2 + 2}
____exports.posTopContainer = {0, 4, ScreenWidth, 70}
local base = {SHOW_LABEL_OVER_VALUE = 6, LABEL_FONT_SIZE = 8}
____exports.primary = __TS__ObjectAssign({LABEL_COLOR = "primary", BAR_COLOR = PrimaryBar}, base)
____exports.secondary = __TS__ObjectAssign({LABEL_COLOR = "secondary", BAR_COLOR = SecondaryBar}, base)
____exports.tertiary = __TS__ObjectAssign({LABEL_COLOR = "tertiary", BAR_COLOR = TertiaryBar}, base)
____exports.quaternary = __TS__ObjectAssign({LABEL_COLOR = "quaternary", BAR_COLOR = QuaternaryBar}, base)
return ____exports
