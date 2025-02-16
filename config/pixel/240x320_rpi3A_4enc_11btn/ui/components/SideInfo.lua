--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
local React = require("config.libs.react")
local ____TextGrid = require("config.libs.tsComponents.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local SideColor = ____constants.SideColor
local SideKeyInfoPosition = ____constants.SideKeyInfoPosition
function ____exports.SideInfo(____bindingPattern0)
    local ctxValue
    local down
    local up
    up = ____bindingPattern0.up
    down = ____bindingPattern0.down
    ctxValue = ____bindingPattern0.ctxValue
    return React.createElement(TextGrid, {bounds = SideKeyInfoPosition, rows = {up, down}, bgColor = SideColor, contextValue = ctxValue})
end
return ____exports
