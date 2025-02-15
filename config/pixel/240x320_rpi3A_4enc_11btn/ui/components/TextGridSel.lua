local ____lualib = require("lualib_bundle")
local __TS__StringReplace = ____lualib.__TS__StringReplace
local ____exports = {}
local React = require("config.libs.react")
local ____Keymaps = require("config.libs.nativeComponents.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____VisibilityContext = require("config.libs.nativeComponents.VisibilityContext")
local VisibilityContext = ____VisibilityContext.VisibilityContext
local ____TextGrid = require("config.libs.tsComponents.TextGrid")
local TextGrid = ____TextGrid.TextGrid
local ____constants = require("config.pixel.240x320_rpi3A_4enc_11btn.ui.constants")
local KeyInfoPosition = ____constants.KeyInfoPosition
function ____exports.TextGridSel(____bindingPattern0)
    local selectedBackground
    local contextValue
    local keys
    local rows
    local selected
    selected = ____bindingPattern0.selected
    rows = ____bindingPattern0.rows
    keys = ____bindingPattern0.keys
    contextValue = ____bindingPattern0.contextValue
    selectedBackground = ____bindingPattern0.selectedBackground
    if selected then
        do
            local i = 0
            while i < #rows do
                rows[i + 1] = __TS__StringReplace(rows[i + 1], selected, "!" .. selected)
                i = i + 1
            end
        end
    end
    return React.createElement(
        TextGrid,
        {bounds = KeyInfoPosition, rows = rows, activeBgColor = selectedBackground},
        React.createElement(VisibilityContext, {index = 254, condition = "SHOW_WHEN", value = contextValue}),
        React.createElement(Keymaps, {keys = keys})
    )
end
return ____exports
