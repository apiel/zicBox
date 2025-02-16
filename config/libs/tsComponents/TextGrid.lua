local ____lualib = require("lualib_bundle")
local __TS__StringReplace = ____lualib.__TS__StringReplace
local __TS__Number = ____lualib.__TS__Number
local __TS__StringSplit = ____lualib.__TS__StringSplit
local __TS__StringAccess = ____lualib.__TS__StringAccess
local __TS__StringSubstr = ____lualib.__TS__StringSubstr
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local TextGridRender, visibilityContextIndex
local React = require("config.libs.react")
local ____HiddenValue = require("config.libs.nativeComponents.HiddenValue")
local HiddenValue = ____HiddenValue.HiddenValue
local ____Keymaps = require("config.libs.nativeComponents.Keymaps")
local Keymaps = ____Keymaps.Keymaps
local ____Text = require("config.libs.nativeComponents.Text")
local Text = ____Text.Text
local ____VisibilityContext = require("config.libs.nativeComponents.VisibilityContext")
local VisibilityContext = ____VisibilityContext.VisibilityContext
local ____ui = require("config.libs.ui")
local getBounds = ____ui.getBounds
local rgb = ____ui.rgb
function TextGridRender(____bindingPattern0)
    local props
    local contextValue
    local shiftedTextColor
    local activeBgColor
    local bgColor
    local rows
    local bounds
    bounds = ____bindingPattern0.bounds
    rows = ____bindingPattern0.rows
    bgColor = ____bindingPattern0.bgColor
    if bgColor == nil then
        bgColor = "background"
    end
    activeBgColor = ____bindingPattern0.activeBgColor
    if activeBgColor == nil then
        activeBgColor = "primary"
    end
    shiftedTextColor = ____bindingPattern0.shiftedTextColor
    if shiftedTextColor == nil then
        shiftedTextColor = rgb(80, 75, 75)
    end
    contextValue = ____bindingPattern0.contextValue
    props = __TS__ObjectRest(____bindingPattern0, {
        bounds = true,
        rows = true,
        bgColor = true,
        activeBgColor = true,
        shiftedTextColor = true,
        contextValue = true
    })
    local textColor = "text"
    local h = 11
    local x, y, w = table.unpack(getBounds(bounds))
    local textY = __TS__Number(y) + 2
    local children = {}
    local marginTop = 0
    for ____, row in ipairs(rows) do
        local items = __TS__StringSplit(row, " ")
        local width = __TS__Number(w) / #items
        local marginLeft = 0
        for ____, item in ipairs(items) do
            local bg = bgColor
            if __TS__StringAccess(item, 0) == "!" then
                item = __TS__StringSubstr(item, 1)
                bg = activeBgColor
            end
            local color = textColor
            if __TS__StringAccess(item, 0) == "^" then
                item = __TS__StringSubstr(item, 1)
                color = shiftedTextColor
            end
            children[#children + 1] = React.createElement(
                Text,
                __TS__ObjectAssign(
                    {
                        text = item,
                        bounds = {
                            __TS__Number(x) + marginLeft,
                            textY + marginTop,
                            width,
                            h
                        }
                    },
                    props,
                    {centered = true, bgColor = bg, color = color, visibilityContext = contextValue ~= nil and ({{index = visibilityContextIndex, condition = "SHOW_WHEN", value = contextValue}}) or nil}
                )
            )
            marginLeft = marginLeft + width
        end
        marginTop = marginTop + h
    end
    return children
end
visibilityContextIndex = 254
function ____exports.TextGrid(____bindingPattern0)
    local bgColor
    local selectedBackground
    local contextValue
    local keys
    local rows
    local selected
    local bounds
    bounds = ____bindingPattern0.bounds
    selected = ____bindingPattern0.selected
    rows = ____bindingPattern0.rows
    keys = ____bindingPattern0.keys
    contextValue = ____bindingPattern0.contextValue
    selectedBackground = ____bindingPattern0.selectedBackground
    bgColor = ____bindingPattern0.bgColor
    if bgColor == nil then
        bgColor = "background"
    end
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
        React.Fragment,
        nil,
        React.createElement(
            HiddenValue,
            nil,
            contextValue ~= nil and React.createElement(VisibilityContext, {index = visibilityContextIndex, condition = "SHOW_WHEN", value = contextValue}),
            keys and React.createElement(Keymaps, {keys = keys})
        ),
        React.createElement(TextGridRender, {
            bounds = bounds,
            rows = rows,
            activeBgColor = selectedBackground,
            contextValue = contextValue,
            bgColor = bgColor
        })
    )
end
return ____exports
