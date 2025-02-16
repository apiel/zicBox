--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
local ____exports = {}
do
    local ____export = require("config.pixel.240x320_kick.constants")
    for ____exportKey, ____exportValue in pairs(____export) do
        if ____exportKey ~= "default" then
            ____exports[____exportKey] = ____exportValue
        end
    end
end
____exports.ScreenWidth = 240
____exports.ScreenHeight = 320
____exports.W1_4 = ____exports.ScreenWidth / 4
____exports.W2_4 = ____exports.W1_4 * 2
____exports.W1_2 = ____exports.W2_4
____exports.W3_4 = ____exports.W1_4 * 3
____exports.W1_8 = ____exports.ScreenWidth / 8
____exports.W2_8 = ____exports.W1_8 * 2
____exports.W3_8 = ____exports.W1_8 * 3
____exports.W4_8 = ____exports.W1_8 * 4
____exports.W5_8 = ____exports.W1_8 * 5
____exports.W6_8 = ____exports.W1_8 * 6
____exports.W7_8 = ____exports.W1_8 * 7
____exports.W8_8 = ____exports.W1_8 * 8
____exports.KeyInfoPosition = {0, 296, ____exports.ScreenWidth}
____exports.PrimaryBar = "#377275"
____exports.SecondaryBar = "#b54f5e"
____exports.TertiaryBar = "#368a36"
____exports.QuaternaryBar = "#94854b"
____exports.ColorTrack1 = "#a3c912"
____exports.ColorTrack2 = "#fd6f0e"
____exports.ColorTrack3 = "#15a5ca"
____exports.ColorTrack4 = "#ffcd04"
return ____exports
