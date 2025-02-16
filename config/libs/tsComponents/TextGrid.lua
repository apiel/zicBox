local ____lualib = require("lualib_bundle")
local __TS__Number = ____lualib.__TS__Number
local __TS__StringSplit = ____lualib.__TS__StringSplit
local __TS__StringAccess = ____lualib.__TS__StringAccess
local __TS__StringSubstr = ____lualib.__TS__StringSubstr
local __TS__ObjectAssign = ____lualib.__TS__ObjectAssign
local __TS__ObjectRest = ____lualib.__TS__ObjectRest
local ____exports = {}
local React = require("config.libs.react")
local ____ui = require("config.libs.ui")
local getBounds = ____ui.getBounds
local rgb = ____ui.rgb
local ____Text = require("config.libs.nativeComponents.Text")
local Text = ____Text.Text
local ____VisibilityContext = require("config.libs.nativeComponents.VisibilityContext")
local VisibilityContext = ____VisibilityContext.VisibilityContext
function ____exports.TextGrid(____bindingPattern0)
    local props
    local visibilityValue
    local visibilityContext
    local visibilityCondition
    local shiftedTextColor
    local activeBgColor
    local textColor
    local bgColor
    local rows
    local bounds
    bounds = ____bindingPattern0.bounds
    rows = ____bindingPattern0.rows
    bgColor = ____bindingPattern0.bgColor
    if bgColor == nil then
        bgColor = "background"
    end
    textColor = ____bindingPattern0.textColor
    if textColor == nil then
        textColor = "text"
    end
    activeBgColor = ____bindingPattern0.activeBgColor
    if activeBgColor == nil then
        activeBgColor = "primary"
    end
    shiftedTextColor = ____bindingPattern0.shiftedTextColor
    if shiftedTextColor == nil then
        shiftedTextColor = rgb(80, 75, 75)
    end
    visibilityCondition = ____bindingPattern0.visibilityCondition
    visibilityContext = ____bindingPattern0.visibilityContext
    visibilityValue = ____bindingPattern0.visibilityValue
    props = __TS__ObjectRest(____bindingPattern0, {
        bounds = true,
        rows = true,
        bgColor = true,
        textColor = true,
        activeBgColor = true,
        shiftedTextColor = true,
        visibilityCondition = true,
        visibilityContext = true,
        visibilityValue = true
    })
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
                    {centered = true, bgColor = bg, color = color}
                ),
                visibilityContext and visibilityCondition and visibilityValue and React.createElement(VisibilityContext, {index = visibilityContext, condition = visibilityCondition, value = visibilityValue})
            )
            marginLeft = marginLeft + width
        end
        marginTop = marginTop + h
    end
    return children
end
return ____exports
